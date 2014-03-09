#include "RecordingControl.h"

using namespace ofxMultiTrack;
using namespace ofxCvGui;

//---------
RecordingControl::RecordingControl(ServerData::Recorder & recorder, ServerData::Recording & recording, ServerData::NodeConnection::Ptr node)
	: recorder(recorder), recording(recording), node(node) {
	this->setBounds(ofRectangle(0,0,100,30));

	this->onDraw += [this] (DrawArguments & args) { this->draw(args); };
	this->onUpdate += [this] (UpdateArguments & args) { this->update(args); };
	this->onBoundsChange += [this] (BoundsChangeArguments & args) { this->boundsChange(args); };
	this->onMouseReleased += [this] (MouseArguments &) { 
		ofxCvGui::Panels::Inspector::select(*this);
	};
	this->trackDirty = false;
	this->cachedCount = 0;

	this->tilt.set("Tilt [degrees]", 0, NUI_CAMERA_ELEVATION_MINIMUM, NUI_CAMERA_ELEVATION_MAXIMUM);

}

//---------
ofxMultiTrack::ServerData::NodeConnection::Ptr RecordingControl::getNode() {
	return this->node;
}

//---------
void RecordingControl::update(UpdateArguments &) {
	
	this->status = this->node->getStatus();

	if(this->cachedCount != this->recording.getFrames().size() || this->recorder.isRecording()) {
		this->trackDirty = true;
	}

	if (trackDirty) {
		auto startTime = (float) this->recorder.getStartTime();
		auto endTime = (float) this->recorder.getEndTime();
		auto timeScale = endTime - startTime;
	
		auto & frames = this->recording.getFrames();
		ofMesh markers;
		for(auto & frame : frames) {
			for(int i=0; i<frame.second.size(); i++) {
				if (frame.second[i].size() > 0) {
					markers.addVertex(ofVec3f(frame.first, i, 0));
				}
			}
		}

		this->fbo.begin();
		ofPushMatrix();
		ofClear(0, 0);
		ofScale(fbo.getWidth() / (float) timeScale, fbo.getHeight() / 6.0f, 1.0f);
		ofTranslate(-startTime, 0, 0);
		markers.drawVertices();
		ofPopMatrix();
		this->fbo.end();

		this->cachedCount = this->recording.getFrames().size();
		this->trackDirty = false;
	}
}

//---------
void RecordingControl::draw(DrawArguments & args) {
	this->fbo.draw(args.localBounds);

	ofPushStyle();
	auto index = this->node->getIndex();
	auto notes = ofToString(index) + " :";
	
	auto influences = this->node->getInfluenceList();
	for(auto influence : influences) {
		notes += " <-- " + ofToString(influence);
	}

	ofSetColor(100);
	ofDrawBitmapString(notes, 5, this->getHeight() - 5);

	ofSetLineWidth(1);
	ofLine(0,this->getHeight(), this->getWidth(), this->getHeight()); 

	if (ofxCvGui::Panels::Inspector::isSelected(*this)) {
		ofFill();
	} else {
		ofNoFill();
	}

	ofCircle(9, 8, 3);
		
	if (!this->node->isEnabled()) {
		ofEnableAlphaBlending();
		ofSetColor(100,100,100,100);
		ofRect(args.localBounds);
	}

	//draw connection indicator
	ofFill();
	ofSetColor(this->node->isConnected() ? ofColor(100, 255, 100) : ofColor(255, 100, 100));
	ofCircle(20, 8, 3);

	ofPopStyle();
}

//---------
void RecordingControl::boundsChange(BoundsChangeArguments & args) {
	this->fbo.allocate(this->getWidth(), this->getHeight(), GL_RGBA);
	this->trackDirty = true;
}

using namespace ofxCvGui::Widgets;

//---------
void RecordingControl::populate(ofxCvGui::ElementGroupPtr inspector) {
	auto header = shared_ptr<Title>(new Title("Node #" + ofToString(this->node->getIndex()), Title::Level::H2));
	
	auto addressValue = shared_ptr<LiveValue<string> >(new LiveValue<string>("Address", [this] () {
		return this->status["address"].asString();
	}));

	auto connectedValue = shared_ptr<LiveValue<string> >(new LiveValue<string>("Connected", [this] () {
		return this->status["connected"].asBool() ? "true" : "false";
	}));

	auto fpsValue = shared_ptr<LiveValue<float> >(new LiveValue<float>("Remote fps", [this] () {
		return this->status["remoteStatus"]["fps"].asFloat();
	}));

	auto tiltSlider = shared_ptr<Slider>(new Slider(this->tilt));
	tiltSlider->onValueChange += [this] (const ofParameter<float> & value) {
		Json::Value json;
		auto & jsonMessage = json["Devices"]["KinectSDK"]["tilt"];
		jsonMessage = value.get();
		this->node->addNodeConfig(json);
	};

	inspector->add(header);
	inspector->add(addressValue);
	inspector->add(connectedValue);
	inspector->add(fpsValue);
	inspector->add(tiltSlider);
}