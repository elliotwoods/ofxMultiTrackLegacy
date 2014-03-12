#pragma once

#include "ofMain.h"
#include "ofxCvGui2/src/ofxCvGui.h"
#include "ofxCvGui2/src/ofxCvGui.h"
#include "ofxMultiTrack/src/ofxMultiTrack/Server.h"

class RecordingControl : public ofxCvGui::Element, public ofxCvGui::Widgets::IInspectable {
public:
	RecordingControl(ofxMultiTrack::ServerData::Recorder &, ofxMultiTrack::ServerData::Recording &, ofxMultiTrack::ServerData::NodeConnection::Ptr);
	ofxMultiTrack::ServerData::NodeConnection::Ptr getNode();
protected:
	void update();
	void update(ofxCvGui::UpdateArguments &);
	void draw(ofxCvGui::DrawArguments &);
	void boundsChange(ofxCvGui::BoundsChangeArguments &);

	void populate(ofxCvGui::ElementGroupPtr) override;

	ofxMultiTrack::ServerData::Recorder & recorder;
	ofxMultiTrack::ServerData::Recording & recording;
	ofxMultiTrack::ServerData::NodeConnection::Ptr node;

	ofFbo fbo;
	bool trackDirty;
	int cachedCount;

	Json::Value status;
};