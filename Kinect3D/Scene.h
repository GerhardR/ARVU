#ifndef SCENE_H
#define SCENE_H

#include "helpers.h"

class Scene {
public:
	virtual void handle_events( const GLWindow::EventSummary & events) {}
	virtual void render( MyKinect & kinect ) {}
};

class KinectScene : public Scene {
public:
	vector<Point> points;
	float point_size;

	KinectScene() : point_size(2) {}

	void handle_events( const GLWindow::EventSummary & events){
		if(events.key_up.count('1'))
			point_size = 1;
		if(events.key_up.count('2'))
			point_size = 2;
		if(events.key_up.count('3'))
			point_size = 3;
		if(events.key_up.count('4'))
			point_size = 4;
		if(events.key_up.count('5'))
			point_size = 5;
	}

	void render( MyKinect & kinect ){
		// get 3D points
		kinect.make3DPoints(points);
		// render 3D points
		glPointSize(point_size);
		if(points.size() > 0){
			render_points_colored( points );
		}
		// now render any valid skeletons
		vector<int> valid_skeletons;
		kinect.getTrackedSkeletons(valid_skeletons);
		for(unsigned i = 0; i < valid_skeletons.size(); ++i){
			glColor3f(0,1,0);
			glLineWidth(4);
			render_skeleton( reinterpret_cast<const float *>( kinect.getSkeleton(valid_skeletons[i])));
			glColor3f(1,1,0);
			glPointSize(10);
			render_skeleton_points(reinterpret_cast<const float *>( kinect.getSkeleton(valid_skeletons[i])));
		}
	}
};

class Balls : public KinectScene {
public:
	struct State {
		float x, y, z;
		float r,g,b;
	};

	GLUquadric * sphere;
	vector<State> balls;

	Balls() {
		sphere = gluNewQuadric();
	}

	void handle_events(const GLWindow::EventSummary & events) {
		KinectScene::handle_events(events);
		GLWindow::EventSummary::mouse_iterator left = events.mouse_up.find(GLWindow::BUTTON_LEFT);
		if(left != events.mouse_up.end()){
			State new_ball;
			new_ball.z = 0.03f;
			new_ball.x = -(-1 + 2*left->second.first.x/float(events.window_size.x)) * 0.56f * new_ball.z;
			new_ball.y = -(-1 + 2*left->second.first.y/float(events.window_size.y)) * 0.422f * new_ball.z;
			new_ball.r = 0.1f;
			new_ball.g = 0.2f;
			new_ball.b = 0.3f;
			balls.push_back(new_ball);
		}
	}

	void render( MyKinect & kinect ){
		// render the background first
		KinectScene::render(kinect);

		glEnable(GL_LIGHTING);
		GLfloat LightAmbient[] = {0.1f, 0.1f, 0.1f, 1.0f};
		GLfloat LightDiffuse[] = {1.0f, 1.0f, 1.0f, 1.0f};
		GLfloat LightSpecular[] = {1.0f, 1.0f, 1.0f, 1.0f};
		GLfloat LightPosition[] = {0.0f, 2.0f, 2.0f, 1.0f};

		glLightfv(GL_LIGHT0, GL_AMBIENT, LightAmbient);
		glLightfv(GL_LIGHT0, GL_DIFFUSE, LightDiffuse);
		glLightfv(GL_LIGHT0, GL_SPECULAR, LightSpecular);
		glLightfv(GL_LIGHT0, GL_POSITION, LightPosition);
		glEnable(GL_LIGHT0);
		glEnable(GL_COLOR_MATERIAL);
		glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
		glShadeModel(GL_SMOOTH);

		glColor3f(1,0,0);
		for(unsigned int i = 0; i < balls.size();){
			// render ball
			glTranslatef(balls[i].x, balls[i].y, balls[i].z); 
			gluSphere(sphere, 0.1, 15, 8);
			glTranslatef(-balls[i].x, -balls[i].y, -balls[i].z);
			// update position
			balls[i].x += balls[i].x/balls[i].z * 0.05f;
			balls[i].y += balls[i].y/balls[i].z * 0.05f;
			balls[i].z += balls[i].z/balls[i].z * 0.05f;
			// retire if beyond the far plane
			if(balls[i].z > 5)
				balls.erase(balls.begin()+i);
			else 
				++i;
		}
		glDisable(GL_LIGHTING);
	}

	~Balls() {
		gluDeleteQuadric(sphere);
	}
};

#endif // SCENE_H
