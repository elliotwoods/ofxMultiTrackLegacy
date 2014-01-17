#pragma once

#include "ofMain.h"
#include "ofxCvGui2/src/ofxCvGui.h"
#include "ofxMultiTrack/src/ofxMultiTrack/Server.h"

class RecordingControl : public ofxCvGui::ElementGroup {
public:
	RecordingControl(ofxMultiTrack::Server::Recorder &, ofxMultiTrack::Server::Recording &);
protected:
	ofxMultiTrack::Server::Recorder & recorder;
	ofxMultiTrack::Server::Recording & recording;
	ofMesh markers;
};