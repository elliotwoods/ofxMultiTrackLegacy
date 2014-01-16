#include "RecordingControl.h"

using namespace ofxMultiTrack;
using namespace ofxCvGui;

//---------
RecordingControl::RecordingControl(Server::Recording & recording) : recording(recording) {
	this->setBounds(ofRectangle(0,0,50,100));

	this->onDraw += [this] (DrawArguments & args) {
		ofPushStyle();
		ofSetLineWidth(1);
		ofNoFill();
		ofSetColor(255,100,100);
		ofRect(0, 0, this->getWidth(), this->getHeight());
		ofPopStyle();
	};
}