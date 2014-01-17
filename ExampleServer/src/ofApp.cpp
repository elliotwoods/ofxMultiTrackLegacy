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
	auto statusPanel = gui.addScroll();
	auto worldPanel = gui.addWorld();
	auto recorderPanel = gui.addScroll();

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
	worldPanel->setGridEnabled(false);

	recorderPanel->add(ofxCvGui::ElementPtr(new RecorderControl(server.getRecorder())));
	auto & nodes = this->server.getNodes();
	for(auto node : nodes) {
		auto track = ofxCvGui::ElementPtr(new RecordingControl(server.getRecorder(), node->getRecording()));
		recorderPanel->add(track);
	}
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
