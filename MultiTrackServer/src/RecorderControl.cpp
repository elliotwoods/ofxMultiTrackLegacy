#include "RecorderControl.h"
#include "../../../addons/ofxAssets/src/ofxAssets.h"

using namespace ofxMultiTrack;
using namespace ofxCvGui;
using namespace ofxAssets;

//---------
RecorderControl::RecorderControl(ServerData::Recorder & recorder) : recorder(recorder) {
	this->hoverPct = 0;

	this->setBounds(ofRectangle(0,0,50,50));

	auto newBlankButton = [=] () {
		auto button = make_shared<ofxCvGui::Utils::Button>();
		this->add(button);
		return button;
	};

	auto playButton = newBlankButton();
	auto recordButton = newBlankButton();
	auto clearButton = newBlankButton();
	auto inButton = newBlankButton();
	auto outButton = newBlankButton();
	auto trimButton = newBlankButton();
	auto saveButton = newBlankButton();
	auto loadButton = newBlankButton();

	auto timeTrack = ElementPtr(new Element());
	this->add(timeTrack);

	auto drawButton = [] (shared_ptr<ofxCvGui::Utils::Button> button, string title) {
		auto width = button->getWidth();
		auto height = button->getHeight();
		auto bounds = ofxCvGui::Utils::drawText(title, 0, 0, false, height, width);
		ofPushStyle();
		ofEnableSmoothing();
		ofNoFill();
		ofSetColor(200);
		ofSetLineWidth(button->isDown() ? 2.0f : 1.0f);
		ofRect(0,0,width,height);
		ofPopStyle();
	};
	
	playButton->onDraw += [=] (DrawArguments &) {
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
	recordButton->onDraw += [=] (DrawArguments &) {
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
	inButton->onDraw += [=] (DrawArguments &) {
		drawButton(inButton, "in");
	};
	outButton->onDraw += [=] (DrawArguments &) {
		drawButton(inButton, "out");
	};
	trimButton->onDraw += [=] (DrawArguments &) {
		drawButton(trimButton, "trim");
	};
	clearButton->onDraw += [=] (DrawArguments &) {
		drawButton(clearButton, "Clear");
	};
	saveButton->onDraw += [=] (DrawArguments &) {
		drawButton(saveButton, "");
		image("ofxCvGui::save").draw(0,0);
	};
	loadButton->onDraw += [=] (DrawArguments &) {
		drawButton(loadButton, "");
		image("ofxCvGui::load").draw(0,0);
	};
	timeTrack->onDraw += [=] (DrawArguments &) {
		float width = timeTrack->getWidth();
		float height = timeTrack->getHeight();

		ofPushStyle();
		ofSetLineWidth(1.0f);

		//hover head
		ofSetColor(100);
		ofLine(width * hoverPct, 0, width * hoverPct, height);

		//play head
		ofSetColor(255);
		auto playHeadDrawPosition = this->recorder.getPlayHeadNormalised() * width;
		ofLine(playHeadDrawPosition, 0, playHeadDrawPosition, height);

		//in point
		float inPointDrawPosition = this->recorder.timelineToNormalised(this->recorder.getInPoint()) * width;
		ofLine(0, height / 2.0f, inPointDrawPosition, height / 2.0f);
		ofLine(inPointDrawPosition, 0, inPointDrawPosition, height);
		
		//out point
		float outPointDrawPosition = this->recorder.timelineToNormalised(this->recorder.getOutPoint()) * width;
		ofLine(outPointDrawPosition, height / 2.0f, width, height / 2.0f);
		ofLine(outPointDrawPosition, 0, outPointDrawPosition, height);
		
		ofPopStyle();
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
	inButton->onHit += [this] (ofVec2f&) {
		this->recorder.setInPoint(this->recorder.getPlayHead());
	};
	outButton->onHit += [this] (ofVec2f&) {
		this->recorder.setOutPoint(this->recorder.getPlayHead());
	};
	trimButton->onHit += [this] (ofVec2f&) {
		this->recorder.trim();
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
	
	timeTrack->onMouse += [this, timeTrack] (MouseArguments & args) {
		float pct = args.localNormalised.x;
		bool click = false;
		if (args.isLocal()) {
			if (args.action == MouseArguments::Pressed || args.action == MouseArguments::Dragged) {
				click = true;
			}
			this->hoverPct = args.localNormalised.x;
		}
		if (timeTrack->getMouseState() == ofxCvGui::Element::Dragging || click) {
			this->recorder.setPlayHeadNormalised(pct);
		}
	};
	
	this->onUpdate += [=] (UpdateArguments &) {
		if (this->recorder.hasData()) {
			clearButton->enable();
		} else {
			clearButton->disable();
		}
		if (this->recorder.getInPoint() != this->recorder.getStartTime() || this->recorder.getOutPoint() != this->recorder.getEndTime()) {
			trimButton->enable();
		} else {
			trimButton->disable();
		}
	};

	auto arrangeButton = [=] (ElementPtr button, float width, float &x) {
		button->setBounds(ofRectangle(x, 0, width, 30));
		x+= width;
	};
	this->onBoundsChange += [=] (BoundsChangeArguments & args) {
		float x = 0.0f;
		arrangeButton(playButton, 60, x);
		arrangeButton(recordButton, 60, x);
		arrangeButton(clearButton, 60, x);
		arrangeButton(inButton, 50, x);
		arrangeButton(outButton, 50, x);
		arrangeButton(trimButton, 50, x);
		arrangeButton(saveButton, 30, x);
		arrangeButton(loadButton, 30, x);
		
		timeTrack->setBounds(ofRectangle(0,30,this->getWidth(),20));
	};

	auto toString = [this] (Timestamp timestamp) {
		return this->recorder.toString(this->recorder.appToTimeline(timestamp));
	};
	this->onDraw += [=] (DrawArguments & args) {
		float x = loadButton->getBounds().getRight() + 20;
		ofDrawBitmapString(toString(this->recorder.getPlayHead()), x, 15);
		ofPushStyle();
		ofSetColor(100);
		ofDrawBitmapString(toString(this->recorder.normalisedToTimestamp(this->hoverPct)), x, 30);
		ofPopStyle();
	};
}