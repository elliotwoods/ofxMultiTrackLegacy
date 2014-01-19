#pragma once

#include "ofMain.h"
#include "ofxMultiTrack/src/ofxMultiTrack.h"
#include "ofxCvGui2/src/ofxCvGui.h"

#include "RecordingControl.h"
#include "RecorderControl.h"

class ofApp : public ofBaseApp{
	struct Target {
		ofRectangle bounds;
		int node;
		int user;
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

	map<int, int> userSelection; //which user from which node to use for calibration
	vector<Target> targets;
	ofPtr<RecorderControl> recorderControl;
	ofxCvGui::ElementPtr calibrateButton;
};
