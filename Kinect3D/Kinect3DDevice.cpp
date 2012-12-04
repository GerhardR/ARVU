#include "Kinect3DDevice.h"

#include <iostream>
#include <algorithm>
#include <vector>

using namespace std;

Kinect3DDevice::Kinect3DDevice(bool skeleton) : use_skeleton(skeleton), m_hThNuiProcess(INVALID_HANDLE_VALUE), m_hEvNuiProcessStop(INVALID_HANDLE_VALUE) {
    HRESULT hr;

    m_hNextDepthFrameEvent = CreateEvent( NULL, TRUE, FALSE, NULL );
    m_hNextVideoFrameEvent = CreateEvent( NULL, TRUE, FALSE, NULL );
    m_hNextSkeletonEvent = CreateEvent( NULL, TRUE, FALSE, NULL );

    hr =  use_skeleton ? NuiInitialize( NUI_INITIALIZE_FLAG_USES_COLOR | NUI_INITIALIZE_FLAG_USES_DEPTH_AND_PLAYER_INDEX | NUI_INITIALIZE_FLAG_USES_SKELETON ) 
                       : NuiInitialize( NUI_INITIALIZE_FLAG_USES_COLOR | NUI_INITIALIZE_FLAG_USES_DEPTH );
    if( FAILED( hr ) ){
        cout << "Kinect3DDevice: Could not open Kinect Device" << endl;
        return;
    }

     hr = NuiSkeletonTrackingEnable( m_hNextSkeletonEvent, 0 );
     hr = NuiImageStreamOpen(
        NUI_IMAGE_TYPE_COLOR,
        NUI_IMAGE_RESOLUTION_640x480,
        0,
        2,
        m_hNextVideoFrameEvent,
        &m_pVideoStreamHandle );

      hr = NuiImageStreamOpen(
        use_skeleton ? NUI_IMAGE_TYPE_DEPTH_AND_PLAYER_INDEX : NUI_IMAGE_TYPE_DEPTH,
        use_skeleton ? NUI_IMAGE_RESOLUTION_320x240 : NUI_IMAGE_RESOLUTION_640x480,
        0,
        2,
        m_hNextDepthFrameEvent,
        &m_pDepthStreamHandle );

    // Start the Nui processing thread
    m_hEvNuiProcessStop=CreateEvent(NULL,FALSE,FALSE,NULL);
    m_hThNuiProcess=CreateThread(NULL,0,run,this,0,NULL);
}

Kinect3DDevice::~Kinect3DDevice(){
   // Stop the Nui processing thread
    if(m_hEvNuiProcessStop!=INVALID_HANDLE_VALUE)
    {
        // Signal the thread
        SetEvent(m_hEvNuiProcessStop);

        // Wait for thread to stop
        if(m_hThNuiProcess!=INVALID_HANDLE_VALUE)
        {
            WaitForSingleObject(m_hThNuiProcess,INFINITE);
            CloseHandle(m_hThNuiProcess);
        }
        CloseHandle(m_hEvNuiProcessStop);
    }

    NuiShutdown( );
    if( m_hNextSkeletonEvent && ( m_hNextSkeletonEvent != INVALID_HANDLE_VALUE ) )
    {
        CloseHandle( m_hNextSkeletonEvent );
        m_hNextSkeletonEvent = NULL;
    }
    if( m_hNextDepthFrameEvent && ( m_hNextDepthFrameEvent != INVALID_HANDLE_VALUE ) )
    {
        CloseHandle( m_hNextDepthFrameEvent );
        m_hNextDepthFrameEvent = NULL;
    }
    if( m_hNextVideoFrameEvent && ( m_hNextVideoFrameEvent != INVALID_HANDLE_VALUE ) )
    {
        CloseHandle( m_hNextVideoFrameEvent );
        m_hNextVideoFrameEvent = NULL;
    }
}

DWORD WINAPI Kinect3DDevice::run(LPVOID pParam)
{
    Kinect3DDevice *pthis=(Kinect3DDevice *) pParam;
    HANDLE hEvents[4];
    int	nEventIdx;

    // Configure events to be listened on
    hEvents[0]=pthis->m_hEvNuiProcessStop;
    hEvents[1]=pthis->m_hNextDepthFrameEvent;
    hEvents[2]=pthis->m_hNextVideoFrameEvent;
    hEvents[3]=pthis->m_hNextSkeletonEvent;

    // Main thread loop
    while(1)
    {
        // Wait for an event to be signalled
        nEventIdx=WaitForMultipleObjects(sizeof(hEvents)/sizeof(hEvents[0]),hEvents,FALSE,100);

        // If the stop event, stop looping and exit
        if(nEventIdx==0)
            break;            

        // Process signal events
        switch(nEventIdx)
        {
            case 1:
                pthis->callDepthCallback();
                break;

            case 2:
                pthis->callVideoCallback();
                break;

            case 3:
                pthis->callSkeletonCallback( );
                break;
        }
    }

    return (0);
}

void Kinect3DDevice::callVideoCallback(){
    const NUI_IMAGE_FRAME * pImageFrame = NULL;

    HRESULT hr = NuiImageStreamGetNextFrame( m_pVideoStreamHandle, 0, &pImageFrame );
    if( FAILED( hr ) )
        return;

    INuiFrameTexture * pTexture = pImageFrame->pFrameTexture;
    NUI_LOCKED_RECT LockedRect;
    pTexture->LockRect( 0, &LockedRect, NULL, 0 );
    if( LockedRect.Pitch != 0 ) {
        BYTE * pBuffer = (BYTE*) LockedRect.pBits;
        this->VideoCallback(pBuffer);
    } else {
        cout << "Buffer length of received texture is bogus\r\n" << endl;
    }
    // cout << "Rgbframe \t" << pImageFrame->dwFrameNumber << endl;
    NuiImageStreamReleaseFrame( m_pVideoStreamHandle, pImageFrame );
}

void Kinect3DDevice::callDepthCallback( )
{
    const NUI_IMAGE_FRAME * pImageFrame = NULL;

    HRESULT hr = NuiImageStreamGetNextFrame(m_pDepthStreamHandle, 0, &pImageFrame );

    if( FAILED( hr ) )
        return;

    INuiFrameTexture * pTexture = pImageFrame->pFrameTexture;
    NUI_LOCKED_RECT LockedRect;
    pTexture->LockRect( 0, &LockedRect, NULL, 0 );
    if( LockedRect.Pitch != 0 ) {
        BYTE * pBuffer = (BYTE*) LockedRect.pBits;
        this->DepthCallback(pBuffer);
    } else {
        cout << "Buffer length of received texture is bogus\r\n" << endl;
    }
    // cout << "Depthframe \t" << pImageFrame->dwFrameNumber << endl;
    NuiImageStreamReleaseFrame( m_pDepthStreamHandle, pImageFrame );
}

void Kinect3DDevice::callSkeletonCallback(){

    HRESULT hr =  NuiSkeletonGetNextFrame(0, &m_SkeletonFrame);
    if(FAILED(hr))
        return;

    //smooth skeleton data
    hr = NuiTransformSmooth(&m_SkeletonFrame, NULL);
    this->SkeletonCallback(m_SkeletonFrame.SkeletonData);
    // cout << "Skelframe \t" << m_SkeletonFrame.dwFrameNumber << endl;
}

MyKinect::MyKinect(bool use_skel) : Kinect3DDevice(use_skel) {
    int w, h;

    getVideoSize(w,h);
    rgb.resize(w*h);
        
    getDepthSize(w,h);
    depth.resize(w*h);
    depth_texture.resize(w*h);
}

void MyKinect::VideoCallback(void *video){
    // cout << "rgb" << endl;
    const uint32_t * data = static_cast<uint32_t *>(video);
    copy(data, data + rgb.size(), rgb.data());
    rgb_valid = true;
}

void MyKinect::DepthCallback(void *depth){
    // cout << "depth" << endl;
    const uint16_t * data = static_cast<uint16_t *>(depth);
    // copy raw depth data for saving later
    copy(data, data + this->depth.size(), this->depth.data());
    // and transform to 8 bit for rendering, this depends on skeleton data being used
    transform(data, data + this->depth_texture.size(), this->depth_texture.data(), shift_right<uint16_t>(isUsingSkeleton()?7:4));
    // this creates a color map representing the texture for rendering
    // transformDepth2Rgb(data, this->getDepthTexture());
    depth_valid = true;
}

void MyKinect::SkeletonCallback(NUI_SKELETON_DATA  * data){
}

void MyKinect::make3DPoints( vector<Point> & points ) const {
    points.clear();

    int W, H;
    getDepthSize(W,H);

    for(int y = 0; y < H; ++y)
        for( int x = 0; x < W; ++x){
            const uint16_t d = depth[W*y + x]; 
            if( d == 0)
                continue;
            LONG colorX, colorY;
            HRESULT res = isUsingSkeleton() 
                    ? NuiImageGetColorPixelCoordinatesFromDepthPixel( NUI_IMAGE_RESOLUTION_640x480, NULL, x, y, d, &colorX, &colorY)
                    : NuiImageGetColorPixelCoordinatesFromDepthPixel( NUI_IMAGE_RESOLUTION_640x480, NULL, (640-x)/2, y/2, d << 3, &colorX, &colorY);
            if(FAILED(res))
                continue;
            if(colorX < 0 || colorX > 639 || colorY < 0 || colorY > 479)
                continue;
            const Vector4 pos = NuiTransformDepthImageToSkeleton(x, y, isUsingSkeleton() ? d : d << 3);
            points.push_back(Point(isUsingSkeleton()?pos.x:-pos.x, pos.y, pos.z, flipColors(rgb[640*colorY + colorX])));
        }
}

void MyKinect::make3DSkeletonPoints( vector<Point> & background, vector<Point> & player1, vector<Point> & player2) const {
    if(!isUsingSkeleton())
        return;
        
    background.clear();
    player1.clear();
    player2.clear();

    int W, H;
    getDepthSize(W,H);

    for(int y = 0; y < H; ++y)
        for( int x = 0; x < W; ++x){
            const uint16_t d = depth[W*y + x]; 
            if( d == 0)
                continue;
            LONG colorX, colorY;
            HRESULT res = NuiImageGetColorPixelCoordinatesFromDepthPixel( NUI_IMAGE_RESOLUTION_640x480, NULL, x, y, d, &colorX, &colorY);
            if(FAILED(res))
                continue;
            if(colorX < 0 || colorX > 639 || colorY < 0 || colorY > 479)
                continue;
            const Vector4 pos = NuiTransformDepthImageToSkeleton(x, y, d );
            Point p(pos.x, pos.y, pos.z, flipColors(rgb[640*colorY + colorX]));
            switch(d & 7){
            case 0: 
                background.push_back(p);
                break;
            case 1:
                player1.push_back(p);
                break;
            case 2:
                player2.push_back(p);
                break;
            }
        }
}

const Vector4 * MyKinect::getSkeleton(const int number) const{
    return m_SkeletonFrame.SkeletonData[number].SkeletonPositions;
}
