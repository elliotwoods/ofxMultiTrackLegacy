#include "ofApp.h"

using namespace ofxCvGui;

//--------------------------------------------------------------
void ofApp::setup(){
	this->deviceIndex = 0;
	while (!kinect.initSensor(deviceIndex)) { // this is currently always returning true
		this->deviceIndex++;
	}

	kinect.initColorStream(640, 480, true);
	kinect.initDepthStream(640, 480, false);
	kinect.initSkeletonStream(false);
	kinect.start();

	ofDisableAlphaBlending();

	gui.init();
	auto instructions = gui.addInstructions();
	gui.add(kinect.getDepthTexture(), "Depth");
	gui.add(kinect.getColorTexture(), "RGB");

	instructions->onDraw += [this] (DrawArguments &args) {
		AssetRegister.drawText("FPS : " + ofToString(ofGetFrameRate()), 20, 200);
		AssetRegister.drawText("Device index : " + ofToString(this->deviceIndex), 20, 220);
	};

	auto worldPanel = gui.addWorld("World");
	worldPanel->setCursorEnabled(true);
	worldPanel->setGridEnabled(true);
	auto camera = worldPanel->getCamera();

	ofEventArgs nullArgs;
	camera.setPosition(ofVec3f(3.0f,3.0f,3.0f)); // <- not working for some reason?

	worldPanel->onDrawWorld += [this] (ofCamera&) {
		for(auto & skeleton : this->kinect.getSkeletons()) {
			for(auto bone : skeleton) {
				auto startPosition = bone.second.getStartPosition();

				ofPushMatrix();
				ofTranslate(startPosition);
				ofSphere(0.025f);
				ofPopMatrix();
			}
		}
	};
}

//--------------------------------------------------------------
void ofApp::update(){
	kinect.update();
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
