#include "RecorderControl.h"

using namespace ofxMultiTrack;
using namespace ofxCvGui;

//---------
RecorderControl::RecorderControl(Server::Recorder & recorder) : recorder(recorder) {
	this->setBounds(ofRectangle(0,0,50,50));

	auto playButton = ofPtr<Utils::Button>(new Utils::Button());
	auto recordButton = ofPtr<Utils::Button>(new Utils::Button());
	auto clearButton = ofPtr<Utils::Button>(new Utils::Button());
	auto saveButton = ofPtr<Utils::Button>(new Utils::Button());
	auto loadButton = ofPtr<Utils::Button>(new Utils::Button());

	this->add(playButton);
	this->add(recordButton);
	this->add(clearButton);
	this->add(saveButton);
	this->add(loadButton);

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
		if (engaged) {
			ofPushStyle();
			ofSetLineWidth(0);
			ofSetColor(ofColor(50, 100, 50));
			ofRect(0, 0, playButton->getWidth(), playButton->getHeight());
			ofPopStyle();
		}
		drawButton(playButton, title);
	};
	recordButton->onDraw += [this, drawButton, recordButton] (DrawArguments &) {
		bool engaged = this->recorder.isRecording();
		string title = engaged ? "Stop" : "Record";
		if (engaged) {
			ofPushStyle();
			ofSetLineWidth(0);
			ofSetColor(ofColor(100, 50, 50));
			ofRect(0, 0, recordButton->getWidth(), recordButton->getHeight());
			ofPopStyle();
		}
		drawButton(recordButton, title);
	};
	clearButton->onDraw += [this, drawButton, clearButton] (DrawArguments &) {
		drawButton(clearButton, "Clear");
	};
	saveButton->onDraw += [this, drawButton, saveButton] (DrawArguments &) {
		drawButton(saveButton, "");
		AssetRegister.getImage("save").draw(0,0);
	};
	loadButton->onDraw += [this, drawButton, loadButton] (DrawArguments &) {
		drawButton(loadButton, "");
		AssetRegister.getImage("load").draw(0,0);
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
	saveButton->onHit += [this] (ofVec2f&) {
		this->recorder.save();
	};
	loadButton->onHit += [this] (ofVec2f&) {
		this->recorder.load();
	};

	this->onBoundsChange += [recordButton, playButton, clearButton, saveButton, loadButton] (BoundsChangeArguments & args) {
		playButton->setBounds(ofRectangle(0,0,100,30));
		recordButton->setBounds(ofRectangle(100,0,100,30));
		clearButton->setBounds(ofRectangle(200,0,100,30));
		saveButton->setBounds(ofRectangle(300,0,30,30));
		loadButton->setBounds(ofRectangle(330,0,30,30));
	};
}