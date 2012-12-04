#ifndef KINECT3DDEVICE_H
#define KINECT3DDEVICE_H

#include <Windows.h>
#include <MSR_NuiApi.h>

#include "helpers.h"

class Kinect3DDevice {
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

#endif // KINECT3DDEVICE_H
