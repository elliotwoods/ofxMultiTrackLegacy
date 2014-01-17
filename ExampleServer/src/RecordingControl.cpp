#include "RecordingControl.h"

using namespace ofxMultiTrack;
using namespace ofxCvGui;

//---------
RecordingControl::RecordingControl(Server::Recorder & recorder, Server::Recording & recording) : recorder(recorder), recording(recording) {
	this->setBounds(ofRectangle(0,0,100,10));

	this->onDraw += [this] (DrawArguments & args) {
		auto startTime = (float) this->recorder.getStartTime();
		auto endTime = (float) this->recorder.getEndTime();
		auto timeScale = endTime - startTime;
		
		auto & frames = this->recording.getFrames();
		auto cachedCount = this->markers.getNumVertices();
		if (frames.size() > cachedCount) {
			auto nextFrame = frames.begin();
			for(int i=0; i<cachedCount; i++) {
				nextFrame++;
			}
			for(auto frame = nextFrame; frame != frames.end(); frame++) {
				markers.addVertex(ofVec3f(frame->first, 0, 0));
			}
		} else {
			markers.clearVertices(); //presume build next draw frame
		}

		ofPushMatrix();
		ofScale(this->getWidth() / timeScale, 1.0f, 1.0f);
		ofTranslate(-startTime, this->getHeight() / 2.0f, 1.0f);
		glPointSize(1.0f);
		markers.drawVertices();
		ofPopMatrix();

		ofPushStyle();
		ofSetLineWidth(1);
		ofSetColor(100);
		ofLine(0,this->getHeight(), this->getWidth(), this->getHeight()); 
		ofPopStyle();
	};
}