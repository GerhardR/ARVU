#include "image_io.h"

#include <cassert>

using namespace std;

#include <Windows.h>
#include <Wincodecsdk.h>

// see the following for reference on the API used here, would be cool in libcvd :)
// http://msdn.microsoft.com/en-us/library/ee719902(v=VS.85).aspx

int save_image( void * data, const ImageRef & size, const int depth, const std::wstring & filename ){
	// Initialize COM
    HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);

    UINT uiWidth = size.x;
    UINT uiHeight = size.y;
    WICPixelFormatGUID formatGUID;
	switch(depth){
	case 1: formatGUID = GUID_WICPixelFormat8bppGray;
		break;
	case 3: formatGUID = GUID_WICPixelFormat24bppRGB;
		break;
	case 2: formatGUID = GUID_WICPixelFormat16bppGray;
		break;
	default: assert(false);
	}

	// create a bitmap
	IWICImagingFactory *pFactory = NULL;
    IWICBitmap *pBitmap = NULL;

    hr = CoCreateInstance( CLSID_WICImagingFactory, NULL, CLSCTX_INPROC_SERVER, IID_IWICImagingFactory, (LPVOID*)&pFactory);
    if (SUCCEEDED(hr))
        hr = pFactory->CreateBitmap(uiWidth, uiHeight, formatGUID, WICBitmapCacheOnDemand, &pBitmap);

    if (SUCCEEDED(hr)){
		IWICBitmapLock *pLock = NULL;
        WICRect rcLock = { 0, 0, uiWidth, uiHeight };
		hr = pBitmap->Lock(&rcLock, WICBitmapLockWrite, &pLock);

        if (SUCCEEDED(hr)){
            UINT cbBufferSize = 0;
            BYTE *pv = NULL;

            if (SUCCEEDED(hr))
                hr = pLock->GetDataPointer(&cbBufferSize, &pv);

			// copy the image
			const unsigned char * pcData = static_cast<const unsigned char *>(data);
			copy(pcData, pcData + cbBufferSize, pv);

			// Release the bitmap lock.
            pLock->Release();
        }
    }

	// now save the bitmap

	// Variables used for encoding.
    IWICStream *piFileStream = NULL;
    IWICBitmapEncoder *piEncoder = NULL;
	IWICBitmapFrameEncode *piFrameEncode = NULL;

    // Create the encoder.
    if (SUCCEEDED(hr))
        hr = pFactory->CreateEncoder(GUID_ContainerFormatPng, NULL, &piEncoder);

	// Create a file stream.
    if (SUCCEEDED(hr))
        hr = pFactory->CreateStream(&piFileStream);

    // Initialize our new file stream.
    if (SUCCEEDED(hr))
        hr = piFileStream->InitializeFromFilename(filename.c_str(), GENERIC_WRITE);

	// Initialize the encoder
    if (SUCCEEDED(hr))
        hr = piEncoder->Initialize(piFileStream,WICBitmapEncoderNoCache);

    if (SUCCEEDED(hr))
        hr = piEncoder->CreateNewFrame(&piFrameEncode, NULL);

    // Initialize the frame encoder.
    if (SUCCEEDED(hr))
        hr = piFrameEncode->Initialize(NULL);
	
	if (SUCCEEDED(hr))
        hr = piFrameEncode->WriteSource( pBitmap, NULL); // Using NULL enables JPEG loss-less encoding.
		//hr = piFrameEncode->WritePixels(uiHeight, uiWidth * depth, uiHeight * uiWidth * depth, static_cast<BYTE *>(data));

    // Commit the frame.
    if (SUCCEEDED(hr))
        hr = piFrameEncode->Commit();

	if (SUCCEEDED(hr))
        piEncoder->Commit();

    if (SUCCEEDED(hr))
        piFileStream->Commit(STGC_DEFAULT);

    if (piFileStream)
        piFileStream->Release();

    if (piEncoder)
        piEncoder->Release();

    if (pBitmap)
		pBitmap->Release();

    if (pFactory)
        pFactory->Release();

    return hr;
}
