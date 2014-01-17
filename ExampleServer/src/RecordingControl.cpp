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
		
		ofMesh markers;
		auto & frames = this->recording.getFrames();
		for(auto frame : frames) {
			auto time = frame.first;
			auto x = ofMap(time, startTime, endTime, 0, this->getWidth());
			markers.addVertex(ofVec3f(x, this->getHeight() / 2.0f, 0));
		}

		glPointSize(1.0f);
		markers.drawVertices();
	};
}