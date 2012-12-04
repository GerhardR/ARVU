#include <iostream>
#include <iomanip>
#include <sstream>
#include <vector>
#include <algorithm>
#include <functional>
#include <cassert>

#include <libfreenect.h>

#include <Windows.h>
#include <gl/GL.h>

#include "glwindow.h"
#include "KinectDevice.h"
#include "image_io.h"

using namespace std;

void transformDepth2Rgb( const uint16_t * depth, uint8_t * rgb ){
	static vector<uint16_t> gamma;
	if(gamma.empty()){
		gamma.resize(2048);
		for( unsigned int i = 0 ; i < 2048 ; i++) {
			double v = i/2048.0;
			v = std::pow(v, 3)* 6;
			gamma[i] = uint16_t(v*6*256);
		}
	}

	for( unsigned int i = 0 ; i < 640*480 ; i++) {
		int pval = gamma[depth[i]];
		int lb = pval & 0xff;
		switch (pval>>8) {
		case 0:
			rgb[3*i+0] = 255;
			rgb[3*i+1] = 255-lb;
			rgb[3*i+2] = 255-lb;
			break;
		case 1:
			rgb[3*i+0] = 255;
			rgb[3*i+1] = lb;
			rgb[3*i+2] = 0;
			break;
		case 2:
			rgb[3*i+0] = 255-lb;
			rgb[3*i+1] = 255;
			rgb[3*i+2] = 0;
			break;
		case 3:
			rgb[3*i+0] = 0;
			rgb[3*i+1] = 255;
			rgb[3*i+2] = lb;
			break;
		case 4:
			rgb[3*i+0] = 0;
			rgb[3*i+1] = 255-lb;
			rgb[3*i+2] = 255;
			break;
		case 5:
			rgb[3*i+0] = 0;
			rgb[3*i+1] = 0;
			rgb[3*i+2] = 255-lb;
			break;
		default:
			rgb[3*i+0] = 0;
			rgb[3*i+1] = 0;
			rgb[3*i+2] = 0;
			break;
		}
	}
}

class MyKinect : public FreenectDevice {
public:
	MyKinect(const int index ): FreenectDevice(index), rgb(640*488*3), depth(640*480), depth_texture(640*480*3) {
	}

	void VideoCallback(void *video, uint32_t timestamp){
		//cout << "rgb\t" << timestamp << "\t" << getVideoBufferSize() << endl;
		const uint8_t * data = static_cast<uint8_t *>(video);
		copy(data, data + getVideoBufferSize(), rgb.data());
		rgb_valid = true;
	}

	void DepthCallback(void *depth, uint32_t timestamp){
		//cout << "depth\t" << timestamp << "\t" << getDepthBufferSize() << endl;
		const uint16_t * data = static_cast<uint16_t *>(depth);
		// copy raw depth data for saving later
		copy(data, data + this->depth.size(), this->depth.data());
		// alternatively, this scales the depth data to full 16 bit scale for visualization
		// transform(data, data + this->depth.size(), this->depth.data(), bind1st(multiplies<unsigned short>(), 64));
		// this creates a color map representing the texture for rendering
		transformDepth2Rgb(data, this->getDepthTexture());
		depth_valid = true;
	}

	void process(){
		rgb_valid = false;
		depth_valid = false;
		FreenectDevice::process();
	}

	bool haveVideoBuffer() { return rgb_valid; }
	bool haveDepthBuffer() { return depth_valid; }

	uint8_t * getVideoBuffer() { return rgb.data(); }
	uint16_t * getDepthBuffer() { return depth.data(); }
	uint8_t * getDepthTexture() { return depth_texture.data(); }

protected:
	vector<uint8_t> rgb;
	vector<uint16_t> depth;
	vector<uint8_t> depth_texture;
	bool rgb_valid, depth_valid;
};

int main(int argc, char ** argv){

	cout << "Welcome to KinectViewer for ARVU @ TU Graz, 2011\n"
			"Usage:\n"
			"a\tswitch to infrared mode\n"
			"s\tswitch to RGB mode\n"
			"Space\trecord a snapshot\n"
			"i\tprint information\n"
			"esc\texit\n" << endl;

	GLWindow window(ImageRef(640+640,488), "KinectViewer");
	GLWindow::EventSummary events;

	MyKinect kinect(0);

	kinect.startVideo();
	kinect.startDepth();

	int mode = 0;

	while(!events.should_quit()){
		events.clear();
		window.get_events(events);
		
		if(kinect.haveVideoBuffer() || kinect.haveDepthBuffer()){
			glRasterPos2i(0,0);
			if(!mode)
				glDrawPixels(640, 480, GL_RGB, GL_UNSIGNED_BYTE,  kinect.getVideoBuffer());
			else
				glDrawPixels(640, 488, GL_LUMINANCE, GL_UNSIGNED_BYTE, kinect.getVideoBuffer());
			glRasterPos2i(640,0);
			glDrawPixels(640, 480, GL_RGB, GL_UNSIGNED_BYTE, kinect.getDepthTexture());
			window.swap_buffers();
		}

		kinect.process();

		// V D V D		ok
		// D V V D		fail
		// V D D V		ok
		// D V D V		ok ?

		if(events.key_up.count('a')){
			kinect.stopVideo(); 
			kinect.stopDepth();
			kinect.setVideoFormat(FREENECT_VIDEO_IR_8BIT);
			kinect.startVideo();
			kinect.startDepth();
			cout << "set to infrared" << endl;
			mode = 1;
		}
		if(events.key_up.count('s')){
			kinect.stopVideo();
			kinect.stopDepth();
			kinect.setVideoFormat(FREENECT_VIDEO_RGB);
			kinect.startVideo();
			kinect.startDepth();
			cout << "set to rgb" << endl;
			mode = 0;
		}

		if(events.key_up.count(' ')){
			static int counter = 0;
			wostringstream filename;
			filename << setfill(L'0');

			if(mode == 0){
				filename << "rgb_" << setw(4) << counter << ".png";
				save_image(kinect.getVideoBuffer(), ImageRef(640, 480), 3, filename.str());
			} else {
				filename << "int_" << setw(4) << counter << ".png";
				save_image(kinect.getVideoBuffer(), ImageRef(640, 488), 1, filename.str());
			}
			filename.str(L"");
			filename << "depth_" << setw(4) << counter << ".png";
			save_image(kinect.getDepthBuffer(), ImageRef(640, 480), 2, filename.str());

			vector<uint8_t> depth_scaled(640*480);
			for(unsigned int i = 0; i < depth_scaled.size(); ++i)
				depth_scaled[i] = kinect.getDepthBuffer()[i] >> 3;

			filename.str(L"");
			filename << "depth_scaled_" << setw(4) << counter << ".png";
			save_image(depth_scaled.data(), ImageRef(640, 480), 1, filename.str());

			cout << "saved snapshots " << counter << endl;
			++counter;
		}
		if(events.key_up.count('i')){
			int x, y;
			kinect.getVideoSize(x,y);
			cout << "rgb\t" << mode << "\t" << x << " , " << y << endl;
			kinect.getDepthSize(x,y);
			cout << "depth\t\t" << x << " , " << y << endl;
		}
	}

	kinect.stopDepth();
	kinect.stopVideo();

	return 0;
}
