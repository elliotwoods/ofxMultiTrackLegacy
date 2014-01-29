#pragma once

#include "ofMain.h"
#include "ofxCvGui2/src/ofxCvGui.h"
#include "ofxMultiTrack/src/ofxMultiTrack/Server.h"

class RecordingControl : public ofxCvGui::ElementGroup {
public:
	RecordingControl(ofxMultiTrack::ServerData::Recorder &, ofxMultiTrack::ServerData::Recording &);
protected:
	void update();
	void update(ofxCvGui::UpdateArguments &);
	void draw(ofxCvGui::DrawArguments &);
	void boundsChange(ofxCvGui::BoundsChangeArguments &);

	ofxMultiTrack::ServerData::Recorder & recorder;
	ofxMultiTrack::ServerData::Recording & recording;
	ofFbo fbo;
	bool trackDirty;
	int cachedCount;
};