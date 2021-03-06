#pragma once

#include "ofMain.h"
#include "ofxMultiTrack/src/ofxMultiTrack.h"
#include "ofxCvGui2/src/ofxCvGui.h"

#include "Version.h"

class ofApp : public ofBaseApp{
	public:
		void setup();
		void update();
		void draw();

		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);
		
		ofxCvGui::Builder gui;

		ofxMultiTrack::Node node;
		ofxKinectCommonBridge * kinect;

		enum State {
			FirstFrame = 0,
			Initialising,
			Running
		};
		State state;
};
