#pragma once

#include "ofMain.h"
#include "ofxCvGui2/src/ofxCvGui.h"
#include "ofxMultiTrack/src/ofxMultiTrack/Server.h"

class RecorderControl : public ofxCvGui::ElementGroup {
public:
	RecorderControl(ofxMultiTrack::ServerData::Recorder &);
protected:
	ofxMultiTrack::ServerData::Recorder & recorder;
	float hoverPct;
};