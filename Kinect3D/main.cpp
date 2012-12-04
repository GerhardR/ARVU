#include <iostream>
#include <iomanip>
#include <sstream>
#include <vector>
#include <cassert>

#include <Windows.h>
#include <gl/GL.h>
#include <gl/GLU.h>
#include <glwindow.h>

using namespace std;

#include "Kinect3DDevice.h"
#include "Viewers.h"
#include "Scene.h"

int main(int argc, char ** argv){
	// open OpenGL Window
	GLWindow window(ImageRef(640+640, 480), "Kinect3D");
	GLWindow::EventSummary events;

	// setup viewers
	vector<Viewer *> viewers;
	viewers.push_back(new ImageViewer);
	viewers.push_back(new ARViewer);
	viewers.push_back(new SceneViewer);

	// setup scenes
	vector<Scene *> scenes;
	scenes.push_back(new KinectScene);
	scenes.push_back(new Balls);

	int viewer_mode = 0;
	int scene_mode = 0;

	// setup kinect;
	MyKinect kinect(true);

	// run event loop and re-render if new buffers are received
	while(!events.should_quit()){
		events.clear();
		window.get_events(events);
		viewers[viewer_mode]->handle_events(events);
		scenes[scene_mode]->handle_events(events);

		if(kinect.haveVideoBuffer() || kinect.haveDepthBuffer()){
			viewers[viewer_mode]->render(kinect);
			if(viewer_mode > 0){
				scenes[scene_mode]->render(kinect);
			}
			window.swap_buffers();
		}

		if(events.key_up.count(' ')){
			viewer_mode = (++viewer_mode) % viewers.size();
		}
		if(events.key_up.count('s')){
			scene_mode = (++scene_mode) % scenes.size();
		}
		Sleep(1);
	}

	return 0;
}
