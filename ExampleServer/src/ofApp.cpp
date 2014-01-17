#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
	ofSetFrameRate(100.0f);

	server.init();
	server.addNode("192.168.10.101", 0);
	server.addNode("192.168.10.101", 1);

	//--
	//gui code
	//
	gui.init();
	auto leftColumn = gui.addGrid();
	auto statusPanel = gui.addScroll("Status");

	auto worldPanel = gui.makeWorld("World");
	auto recorderPanel = gui.makeScroll("Recorder");
	leftColumn->setColsCount(1);
	leftColumn->add(worldPanel);
	leftColumn->add(recorderPanel);
	
	//draw 3d scene
	worldPanel->onDrawWorld += [this] (ofCamera&) {
		this->server.drawWorld();
	};
	worldPanel->setGridEnabled(true);
	worldPanel->setGridLabelsEnabled(false);

	//draw recorder tracks in interactive scrollable panel
	recorderPanel->add(ofxCvGui::ElementPtr(new RecorderControl(server.getRecorder())));
	auto & nodes = this->server.getNodes();
	for(auto node : nodes) {
		auto track = ofxCvGui::ElementPtr(new RecordingControl(server.getRecorder(), node->getRecording()));
		recorderPanel->add(track);
	}

	//draw status in a scrollable panel
	auto statusElement = ofxCvGui::ElementPtr(new ofxCvGui::Element);
	statusPanel->add(statusElement);
	statusElement->onDraw += [this, statusElement] (ofxCvGui::DrawArguments&) {
		auto status = server.getStatusString();
		ofDrawBitmapString(status, 10, 20);
		auto resizeBounds = statusElement->getBounds();
		resizeBounds.height = (std::count(status.begin(), status.end(), '\n') + 1 ) * 14;
		statusElement->setBounds(resizeBounds);
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
