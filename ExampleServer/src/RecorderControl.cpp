#include "RecorderControl.h"

using namespace ofxMultiTrack;
using namespace ofxCvGui;

//---------
RecorderControl::RecorderControl(Server::Recorder & recorder) : recorder(recorder) {
	this->setBounds(ofRectangle(0,0,50,50));

	auto playButton = ofPtr<Utils::Button>(new Utils::Button());
	auto recordButton = ofPtr<Utils::Button>(new Utils::Button());
	auto clearButton = ofPtr<Utils::Button>(new Utils::Button());

	this->add(playButton);
	this->add(recordButton);
	this->add(clearButton);

	auto drawButton = [] (ofPtr<Utils::Button> button, string title) {
		auto width = button->getWidth();
		auto height = button->getHeight();
		auto bounds = AssetRegister.drawText(title, 0, 0, "", false, height, width);
		ofPushStyle();
		ofEnableSmoothing();
		ofNoFill();
		ofSetColor(200);
		ofSetLineWidth(button->isDown() ? 2.0f : 1.0f);
		ofRect(0,0,width,height);
		ofPopStyle();
	};
	
	playButton->onDraw += [this, drawButton, playButton] (DrawArguments &) {
		bool engaged = this->recorder.isPlaying();
		string title = engaged ? "Stop" : "Play";
		ofPushStyle();
		ofSetLineWidth(0);
		ofSetColor(engaged ? ofColor(50, 100, 50) : ofColor(50, 50, 50));
		ofRect(0, 0, playButton->getWidth(), playButton->getHeight());
		ofPopStyle();
		drawButton(playButton, title);
	};
	recordButton->onDraw += [this, drawButton, recordButton] (DrawArguments &) {
		bool engaged = this->recorder.isRecording();
		string title = engaged ? "Stop" : "Record";
		ofPushStyle();
		ofSetLineWidth(0);
		ofSetColor(engaged ? ofColor(100, 50, 50) : ofColor(50, 50, 50));
		ofRect(0, 0, recordButton->getWidth(), recordButton->getHeight());
		ofPopStyle();
		drawButton(recordButton, title);
	};
	clearButton->onDraw += [this, drawButton, clearButton] (DrawArguments &) {
		drawButton(clearButton, "Clear");
	};
	
	playButton->onHit += [this] (ofVec2f&) {
		if(this->recorder.isPlaying()) {
			this->recorder.stop();
		} else {
			this->recorder.play();
		}
	};

	recordButton->onHit += [this] (ofVec2f&) {
		if(this->recorder.isRecording()) {
			this->recorder.stop();
		} else {
			this->recorder.record();
		}
	};

	clearButton->onHit += [this] (ofVec2f&) {
		this->recorder.clear();
	};

	this->onBoundsChange += [recordButton, playButton, clearButton] (BoundsChangeArguments & args) {
		playButton->setBounds(ofRectangle(0,0,100,30));
		recordButton->setBounds(ofRectangle(100,0,100,30));
		clearButton->setBounds(ofRectangle(200,0,100,30));
	};
}