#pragma once

#include "ofMain.h"
#include "ofxCvGui2/src/ofxCvGui.h"
#include "ofxMultiTrack/src/ofxMultiTrack/Server.h"

class RecorderControl : public ofxCvGui::Element {
public:
	RecorderControl(ofxMultiTrack::Server::Recorder &);
protected:
	ofxMultiTrack::Server::Recorder & recorder;
};