#ifndef KINECT3DDEVICE_H
#define KINECT3DDEVICE_H

#include <Windows.h>
#include <NuiApi.h>

#include "helpers.h"

class DepthDevice {
public:
    virtual void getVideoSize( int & width, int & height ) const = 0;
    virtual void getDepthSize( int & width, int & height ) const = 0;
    virtual bool isUsingSkeleton() const = 0;

    virtual bool haveVideoBuffer() const = 0;
    virtual bool haveDepthBuffer() const = 0;

    virtual uint32_t * getVideoBuffer() = 0;
    virtual uint16_t * getDepthBuffer() = 0;
    virtual uint8_t * getDepthTexture() = 0;
    virtual void getTrackedSkeletons(std::vector<int> & valid_skeletons) = 0;
    virtual const Vector4 * getSkeleton(const int number) const = 0;

    virtual void make3DPoints( std::vector<Point> & points ) const = 0;
    virtual void make3DSkeletonPoints( std::vector<Point> & background, std::vector<Point> & player1, std::vector<Point> & player2) const = 0;
};

class Kinect3DDevice : public DepthDevice {
public:
    Kinect3DDevice(bool skeleton = false);
    virtual ~Kinect3DDevice();

    virtual void VideoCallback(void *video) = 0;
    virtual void DepthCallback(void *depth) = 0;
    virtual void SkeletonCallback( NUI_SKELETON_DATA  * data) = 0;

    void getVideoSize( int & width, int & height ) const {
        width = 640;
        height = 480;
    }

    void getDepthSize( int & width, int & height ) const {
        if(use_skeleton){
            width = 320;
            height = 240;
        } else {
            width = 640;
            height = 480;
        }
    }	

    bool isUsingSkeleton() const { 
        return use_skeleton;
    }

protected:
    static DWORD WINAPI run(LPVOID pParam);
    void callVideoCallback();
    void callDepthCallback();
    void callSkeletonCallback();

    bool use_skeleton;

    HANDLE        m_hNextDepthFrameEvent;
    HANDLE        m_hNextVideoFrameEvent;
    HANDLE        m_hNextSkeletonEvent;
    HANDLE        m_pDepthStreamHandle;
    HANDLE        m_pVideoStreamHandle;

    NUI_SKELETON_FRAME m_SkeletonFrame;

    // thread handling
    HANDLE        m_hThNuiProcess;
    HANDLE        m_hEvNuiProcessStop;
};

class MyKinect : public Kinect3DDevice {
public:
    MyKinect(bool use_skel = false);

    void VideoCallback(void *video);
    void DepthCallback(void *depth);
    void SkeletonCallback(NUI_SKELETON_DATA  * data);

    bool haveVideoBuffer() const { return rgb_valid; }
    bool haveDepthBuffer() const { return depth_valid; }

    uint32_t * getVideoBuffer() { rgb_valid = false; return rgb.data();  }
    uint16_t * getDepthBuffer() { depth_valid = false; return depth.data(); }
    uint8_t * getDepthTexture() { depth_valid = false; return depth_texture.data(); }
    void getTrackedSkeletons(std::vector<int> & valid_skeletons) { 
        valid_skeletons.clear();
        for(unsigned i = 0; i < NUI_SKELETON_COUNT; ++i){
            if(m_SkeletonFrame.SkeletonData[i].eTrackingState == NUI_SKELETON_TRACKED)
                valid_skeletons.push_back(i);
        } 
    }
    const Vector4 * getSkeleton(const int number) const;

    void make3DPoints( std::vector<Point> & points ) const;
    void make3DSkeletonPoints( std::vector<Point> & background, std::vector<Point> & player1, std::vector<Point> & player2) const;

protected:
    std::vector<uint32_t> rgb;
    std::vector<uint16_t> depth;
    std::vector<uint8_t> depth_texture;
    bool rgb_valid, depth_valid;
};

class FakeDevice : public DepthDevice {
public:
    FakeDevice() {
        int w, h;

        getVideoSize(w,h);
        rgb.resize(w*h);
        
        for(int y = 0; y < h; ++y)
            for(int x = 0; x < w; ++x){
                rgb[y*w+x] = RGB(x,y,x+y);
            }

        getDepthSize(w,h);
        depth_texture.resize(w*h);
        for(int y = 0; y < h; ++y)
            for(int x = 0; x < w; ++x){
                depth_texture[y*w+x] = 128;
            }
    }

    void getVideoSize( int & width, int & height ) const {
        width = 640;
        height = 480;
    }
    
    void getDepthSize( int & width, int & height ) const  {
        width = 640;
        height = 480;
    }

    bool isUsingSkeleton() const { return false; }

    bool haveVideoBuffer() const { return true; }
    bool haveDepthBuffer() const { return true; }

    uint32_t * getVideoBuffer() { return rgb.data(); }
    uint16_t * getDepthBuffer() { return NULL; }
    uint8_t * getDepthTexture() { return depth_texture.data(); }
    void getTrackedSkeletons(std::vector<int> & valid_skeletons) { valid_skeletons.clear(); };
    const Vector4 * getSkeleton(const int number) const { return NULL; };

    void make3DPoints( std::vector<Point> & points ) const {
        int w, h;
        getVideoSize(w, h);

        if(points.size() < unsigned(w*h)){
            points.clear();
            const float f = 0.003f;
            for(int y = 0; y < h; ++y)
                for(int x = 0; x < w; ++x){
                    float xx = (x-w/2)*f;
                    float yy = (y-h/2)*f;
                    float z = 4 - 2*sqrt(xx*xx + yy*yy);
                    points.push_back(Point(xx*z, yy*z, z, rgb[y*w+x])); 
                }
        }
    }

    void make3DSkeletonPoints( std::vector<Point> & background, std::vector<Point> & player1, std::vector<Point> & player2) const {
        player1.clear();
        player2.clear();
    }

protected:
    std::vector<uint32_t> rgb;
    std::vector<uint8_t> depth_texture;
};

#endif // KINECT3DDEVICE_H
