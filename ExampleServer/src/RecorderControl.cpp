#include "RecorderControl.h"

using namespace ofxMultiTrack;
using namespace ofxCvGui;

//---------
RecorderControl::RecorderControl(Server::Recorder & recorder) : recorder(recorder) {
	this->setBounds(ofRectangle(0,0,50,50));

	auto playButton = ofPtr<Utils::Button>(new Utils::Button());
	auto recordButton = ofPtr<Utils::Button>(new Utils::Button());
	auto stopButton = ofPtr<Utils::Button>(new Utils::Button());

	//auto drawButton = [] (ofPtr<Utils::Button> button, string title) {
	//	auto width = button->getWidth();
	//	auto height = button->getHeight();
	//	auto bounds = AssetRegister.drawText(title, 0, 0, "", true, width, height);
	//	ofPushStyle();
	//	ofEnableSmoothing();
	//	ofNoFill();
	//	ofSetLineWidth(button->isDown() ? 3.0f : 2.0f);
	//	ofRect(0,0,width,height);
	//	ofPopStyle();
	//};
	//
	//playButton->onDraw += [drawButton, playButton] {
	//	drawButton(playButton, "play");
	//};
	//recordButton->onDraw += [drawButton, recordButton] {
	//	drawButton(recordButton, "record");
	//};
	//stopButton->onDraw += [drawButton, stopButton] {
	//	drawButton(stopButton, "stop");
	//};

	this->onBoundsChange += [this, recordButton, playButton, stopButton] (BoundsChangeArguments & args) {
		playButton->setBounds(ofRectangle(0,0,50,50));
		recordButton->setBounds(ofRectangle(50,0,50,50));
		playButton->setBounds(ofRectangle(100,0,50,50));
	};
}