#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
	ofBackground(20);
	ofSetVerticalSync(true);

	server.init();
	server.addNode("localhost", 0);
	server.addNode("localhost", 1);

	//--
	//gui code
	//
	gui.init();
	auto statusPanel = ofPtr<ofxCvGui::Panels::Scroll>(new ofxCvGui::Panels::Scroll);
	gui.add(statusPanel);
	auto worldPanel = gui.makeWorld();
	gui.add(worldPanel);

	auto statusElement = ofxCvGui::ElementPtr(new ofxCvGui::Element);
	statusPanel->add(statusElement);
	statusElement->onDraw += [this, statusElement] (ofxCvGui::DrawArguments&) {
		auto status = server.getStatus();
		ofDrawBitmapString(status, 10, 20);
		auto resizeBounds = statusElement->getBounds();
		resizeBounds.height = (std::count(status.begin(), status.end(), '\n') + 1 ) * 14;
		statusElement->setBounds(resizeBounds);
	};

	worldPanel->onDrawWorld += [this] (ofCamera&) {
		this->server.drawWorld();
	};
	//
	//--
}

//--------------------------------------------------------------
void ofApp::update(){
	server.update();
}

//--------------------------------------------------------------
void ofApp::draw(){
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
