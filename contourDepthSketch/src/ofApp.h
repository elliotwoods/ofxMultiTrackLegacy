#pragma once

#include "ofMain.h"
#include "ofxKinectCommonBridge.h"
#include "ofxCvGui.h"
#include "ofxCv.h"
#include "ofxDelaunay.h"

using namespace cv;
using namespace ofxCv;
using namespace ofxCvGui;

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
		
		ofxKinectCommonBridge kinect;
		INuiSensor * sensor;
		ofxCvGui::Builder gui;


		ofImage previewImage;
		ofParameter<float> ddepth;
		ofParameter<float> ddx;
		ofParameter<float> ksize;
		ofParameter<float> threshold;
		ofParameter<float> dilateIterations;
		ofParameter<float> erodeIterations;
		ofParameter<float> areaThreshold;
		ofParameter<float> erodeDepth;
		ofParameter<float> decimateContour;

		vector<vector<cv::Point> > contours;
		vector<ofMesh> worldContours;

		vector<ofVec3f> testPoints;
		bool enableTestPoint;
		bool enableUpdate;
};
