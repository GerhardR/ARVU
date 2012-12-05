#ifndef VIEWERS_H
#define VIEWERS_H

class Viewer {
public:
	virtual void handle_events( const GLWindow::EventSummary & events) {}
	virtual void render( DepthDevice & kinect ) {}
};

class ImageViewer : public Viewer {
public:
	void render( DepthDevice & kinect ){
		glDisable(GL_DEPTH_TEST);
		glClear(GL_COLOR_BUFFER_BIT);
		
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glOrtho(0, 640+640, 480, 0, -1 , 1);

		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		glPixelZoom(-1, -1);
		glRasterPos2i(640,0);
		glDrawPixels(640, 480, GL_BGRA_EXT, GL_UNSIGNED_BYTE,  kinect.getVideoBuffer());
		if(kinect.isUsingSkeleton()){
			glRasterPos2i(640+640,0);
			glPixelZoom(-2,-2);
		} else { 
			glRasterPos2i(640,0);
			glPixelZoom(1,-1);
		}

		int depthW,depthH;
		kinect.getDepthSize(depthW,depthH);
		glDrawPixels(depthW, depthH, GL_LUMINANCE, GL_UNSIGNED_BYTE, kinect.getDepthTexture());
	}
};

class ARViewer : public Viewer {
public:
	void render( DepthDevice & kinect ){
		glEnable(GL_BLEND);
		glEnable(GL_POINT_SMOOTH);
		glEnable(GL_DEPTH_TEST);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		double nearPlane = 0.01;
		double farPlane = 5;
		double width_2 = 0.562, height_2 = 0.422;
		glFrustum( -nearPlane * width_2, nearPlane * width_2,  -nearPlane * height_2, nearPlane * height_2, nearPlane, farPlane);  

		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		glRotatef(180, 0, 1, 0);
	}
};

class SceneViewer : public Viewer {
public:
	SceneViewer() : yaw(0), pitch(0), zoom(1), tracking_mouse(false) {}

	vector<Point> points;
	double yaw, pitch, zoom;
	bool tracking_mouse;
	ImageRef mouse_start;

	void handle_events( const GLWindow::EventSummary & events ) {
		if(events.mouse_up.count(GLWindow::BUTTON_WHEEL_UP))
			zoom *= 1.1;
		if(events.mouse_up.count(GLWindow::BUTTON_WHEEL_DOWN))
			zoom /= 1.1;
		if(events.mouse_down.count(GLWindow::BUTTON_LEFT)){
			tracking_mouse = true;
			mouse_start = events.cursor;
		} else if(events.mouse_up.count(GLWindow::BUTTON_LEFT)){
			tracking_mouse = false;
		}

		if(tracking_mouse){
			yaw += (events.cursor.x - mouse_start.x) * 0.25;
			pitch += (events.cursor.y - mouse_start.y) * 0.25;
			pitch = min(90.0, max(-90.0, pitch));
			mouse_start = events.cursor;
		}
	}

	void render( DepthDevice & kinect ){
		glEnable(GL_BLEND);
		glEnable(GL_POINT_SMOOTH);
		glEnable(GL_DEPTH_TEST);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		double nearPlane = 0.01;
		double farPlane = 10;
		double size = 0.7;
		double border = nearPlane * size;
		glFrustum( -border, border, -border, border, nearPlane, farPlane);  

		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		glTranslatef(0, 0, -3);
		glRotated(pitch, 1, 0, 0);
		glRotated(yaw, 0, 1, 0);
		glScaled(zoom, zoom, zoom);
		glTranslatef(0, 0, 3);

		glRotatef(180, 0, 1, 0);
	}
};

#endif VIEWERS_H
