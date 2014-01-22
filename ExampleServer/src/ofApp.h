#pragma once

#include "ofMain.h"
#include "ofxMultiTrack/src/ofxMultiTrack.h"
#include "ofxCvGui2/src/ofxCvGui.h"
#include "ofxOsc/src/ofxOsc.h"

#include "Version.h"
#include "RecordingControl.h"
#include "RecorderControl.h"

class ofApp : public ofBaseApp{
	struct NodeUser {
		NodeUser() {
			this->selectionValid = false;
		}
		int node;
		int user;
		bool selectionValid;
	};
	struct Target {
		ofRectangle bounds;
		NodeUser selection;
	};
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
	
	ofxMultiTrack::Server server;
	ofxCvGui::Builder gui;

	NodeUser source;
	NodeUser target;
	vector<Target> targets;
	ofPtr<RecorderControl> recorderControl;
	ofxCvGui::ElementPtr calibrateButton;
	ofxOscSender oscSender;
};
