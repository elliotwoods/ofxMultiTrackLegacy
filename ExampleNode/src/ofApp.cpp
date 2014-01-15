#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
	ofSetFrameRate(100);
}

//--------------------------------------------------------------
void ofApp::update(){
	if (ofGetFrameNum() == 5) {
		node.init();
	}
	if (ofGetFrameNum() >= 5) {
		node.update();
	}
}

//--------------------------------------------------------------
void ofApp::draw(){
	if (ofGetFrameNum() < 5) {
		ofDrawBitmapString("Initialising...", 10, 20);
	} else {
		ofBackground(20);
		ofDrawBitmapString(this->node.getStatus(), 10, 20);
	}
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
