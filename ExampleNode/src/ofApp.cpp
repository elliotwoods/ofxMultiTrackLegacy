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
	auto leftColumn = gui.addGrid();
	leftColumn->add(gui.makeInstructions());
	auto statusPanel = gui.addScroll();

	//set the left grid to be a single column and store for later
	leftColumn->setColsCount(1);
	this->leftColumn = leftColumn;

	//status renders to a gui element on a scrollable panel
	auto statusElement = ofxCvGui::ElementPtr(new ofxCvGui::Element);
	statusPanel->add(statusElement);
	statusElement->onDraw += [this, statusElement] (ofxCvGui::DrawArguments&) {
		auto status = node.getStatusString();

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
	if (ofGetFrameNum() >= 5 && this->state == FirstFrame) {
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

			//position window on screen
			int nodeIndex = this->node.getSettings().localIndex;
			ofSetWindowPosition(nodeIndex * 1920 / 2, 0);

			//get a pointer to the kinect device to draw previews
			auto kinectDevice = this->node.getDevices().get<ofxMultiTrack::Devices::KinectSDK>();
			this->kinect = & kinectDevice->getDevice();

			//add gui panels for depth and rgb
			auto depthPanel = gui.makePanel(this->kinect->getDepthTexture(), "Depth");
			auto colorPanel = gui.makePanel(this->kinect->getColorTexture(), "RGB");

			//add these panels to the left column
			this->leftColumn->add(depthPanel);
			this->leftColumn->add(colorPanel);

			//override the depth draw to draw skeletons also
			depthPanel->onDrawCropped += [this] (ofxCvGui::Panels::BaseImage::DrawCroppedArguments& args) {
				if (this->kinect != 0) {
					ofPushMatrix();
					int skeletonCount = this->kinect->getSkeletons().size();
					for(int i=0; i<skeletonCount; i++) {
						this->kinect->drawSkeleton(i);
					}
					ofPopMatrix();
				}
			};

			//disable the alpha channel whilst renderering color frame by adding early and late listeners to the draw call
			colorPanel->onDraw.addListener([this] (ofxCvGui::DrawArguments&) { ofDisableAlphaBlending(); }, -100, this);
			colorPanel->onDraw.addListener([this] (ofxCvGui::DrawArguments&) { ofEnableAlphaBlending(); }, +100, this);

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
			ofxCvGui::AssetRegister.drawText("Initialising...", 0, 0, "", true, ofGetHeight(), ofGetWidth());
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
