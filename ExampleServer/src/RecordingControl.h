#pragma once

#include "ofMain.h"
#include "ofxCvGui2/src/ofxCvGui.h"
#include "ofxMultiTrack/src/ofxMultiTrack/Server.h"

class RecordingControl : public ofxCvGui::ElementGroup {
public:
	RecordingControl(ofxMultiTrack::ServerData::Recorder &, ofxMultiTrack::ServerData::Recording &);
protected:
	ofxMultiTrack::ServerData::Recorder & recorder;
	ofxMultiTrack::ServerData::Recording & recording;
	ofMesh markers;
};