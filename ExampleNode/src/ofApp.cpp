#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
	ofSetFrameRate(100);
	ofSetCircleResolution(3);
	this->kinect = 0;
	this->state = FirstFrame;

	//--
	//build gui
	//--
	//
	gui.init();
	gui.addInstructions();
	auto statusPanel = gui.addScroll();

	//status renders to a gui element on a scrollable panel
	auto statusElement = ofxCvGui::ElementPtr(new ofxCvGui::Element);
	statusPanel->add(statusElement);
	statusElement->onDraw += [this, statusElement] (ofxCvGui::DrawArguments&) {
		auto status = node.getStatus();

		//resize element to accomodate text
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
	//initialise kinect on frame 5 (to allow GUI to appear)
	if (ofGetFrameNum() == 5) {
		this->state = Initialising;
	}

	switch(state) {
	case FirstFrame:
			break;
		case Initialising:
		{
			//--------------------------------------------
			//THE ONLY LINES OF CODE YOU ACTUALLY NEED 1/2
			//--------------------------------------------
			//
			node.init();
			//
			//--------------------------------------------

			//get a pointer to the kinect device to draw previews
			auto kinectDevice = this->node.getDevices().get<ofxMultiTrack::Devices::KinectSDK>();
			this->kinect = & kinectDevice->getDevice();

			//add gui panels for depth and rgb
			auto depthPanel = gui.add(this->kinect->getDepthTexture(), "Depth");
			auto colorPanel = gui.add(this->kinect->getColorTexture(), "RGB");

			//override the depth draw to draw skeletons also
			depthPanel->onDraw += [this] (ofxCvGui::DrawArguments& args) {
				if (this->kinect != 0) {
					ofPushMatrix();
					ofScale(args.globalBounds.getWidth() / 640.0f, args.globalBounds.getHeight() / 480.0f);
					int skeletonCount = this->kinect->getSkeletons().size();
					for(int i=0; i<skeletonCount; i++) {
						this->kinect->drawSkeleton(i);
					}
					ofPopMatrix();
				}
			};
			this->state = Running;
			break;
		}
		case Running:
			//--------------------------------------------
			//THE ONLY LINES OF CODE YOU ACTUALLY NEED 2/2
			//--------------------------------------------
			//
			node.update();
			//
			//--------------------------------------------
			break;
	}
}

//--------------------------------------------------------------
void ofApp::draw(){
	switch(state) {
		case FirstFrame:
		case Initialising:
			ofDrawBitmapString("Initialising...", 10, 100);
			break;
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
