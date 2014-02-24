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
	this->onMouseReleased += [this] (MouseArguments &) { this->node->toggleEnabled(); };
	this->trackDirty = false;
	this->cachedCount = 0;
}

//---------
void RecordingControl::update(UpdateArguments &) {
	if(this->cachedCount != this->recording.getFrames().size()) {
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

	if (!this->node->isEnabled()) {
		ofEnableAlphaBlending();
		ofSetColor(100,100,100,100);
		ofRect(args.localBounds);
	}
	ofPopStyle();
}

//---------
void RecordingControl::boundsChange(BoundsChangeArguments & args) {
	this->fbo.allocate(this->getWidth(), this->getHeight(), GL_RGBA);
	this->trackDirty = true;
}