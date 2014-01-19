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
		
		//draw ground plane
		ofPushStyle();
		ofPushMatrix();
		ofSetColor(50, 100, 50);
		ofRotate(-90, 0.0f, 0.0f, 1.0f);
		ofDrawGridPlane(8.0f, 8.0f, true);
		ofPopMatrix();
		ofPopStyle();
	};
	worldPanel->setGridEnabled(false);

	//draw markers to hands
	worldPanel->onDraw += [this, worldPanel] (ofxCvGui::DrawArguments & args) {
		auto markerJointName = OFXMULTITRACK_SERVER_ALIGN_REFERENCE_JOINT;

		//get vector of UserSet's (one per node) from the server
		auto data = this->server.getCurrentFrame();

		//init drawing of node labels from 60px down
		float y = 60.0f;
		int nodeIndex = 0;
		this->targets.clear(); //<each time we draw the world, let's remake this target list
		for(auto & node : data) {
			int userIndex = 0;
			for(auto & user : node) {
				//check if we have a selection for this node
				auto selection = this->userSelection.find(nodeIndex);
				int selectionIndex = selection == this->userSelection.end() ? -1 : selection->second;
				if (user.find(markerJointName) != user.end()) {
					//draw label
					string label = ofToString(nodeIndex) + " : " + ofToString(userIndex);
					auto textBounds = ofxCvGui::AssetRegister.drawText(label, 20, y, "", true, 20, 100);
					y+=40;

					//save label as hit target
					Target target;
					target.bounds = textBounds;
					target.node = nodeIndex;
					target.user = userIndex;
					this->targets.push_back(target);

					//draw line to hand
					ofPushStyle();
					ofSetColor(selectionIndex == userIndex ? 255 : 90);
					ofSetLineWidth(2.0f);
					auto endOfTextPoint = textBounds.getCenter() + ofVec2f(textBounds.width / 2.0f, 0.0f);
					auto & camera = worldPanel->getCamera();
					auto bounds = worldPanel->getBounds();
					bounds.x = 0; // we want local bounds
					bounds.y = 0;
					ofLine(endOfTextPoint, camera.worldToScreen(user[markerJointName].position, bounds));
					ofPopStyle();

					//draw outline if selected
					if (selectionIndex == userIndex) {
						ofPushStyle();
						ofSetLineWidth(2.0f);
						ofNoFill();
						ofRect(textBounds);
						ofPopStyle();
					}
				}
				userIndex++;
			}
			nodeIndex++;
		}
	};

	//check if we click one of the targets for selecting calibration
	worldPanel->onMouseReleased += [this] (ofxCvGui::MouseArguments & args) {
		for(auto & target : this->targets) {
			if (target.bounds.inside(args.local)) {
				this->userSelection[target.node] = target.user;
			}
		}
	};

	//draw recorder tracks in interactive scrollable panel
	this->recorderControl = ofPtr<RecorderControl>(new RecorderControl(server.getRecorder()));
	recorderPanel->add(this->recorderControl);
	auto & nodes = this->server.getNodes();
	for(auto node : nodes) {
		auto track = ofxCvGui::ElementPtr(new RecordingControl(server.getRecorder(), node->getRecording()));
		recorderPanel->add(track);
	}
	auto calibrateButton = ofPtr<ofxCvGui::Utils::Button>(new ofxCvGui::Utils::Button);
	calibrateButton->setBounds(ofRectangle(0,0,100,30));
	calibrateButton->onDraw += [calibrateButton] (ofxCvGui::DrawArguments &) {
		ofxCvGui::AssetRegister.drawText("Calibrate", 0, 0, "", true, 30, 100);
		ofPushStyle();
		ofNoFill();
		ofSetLineWidth(calibrateButton->getMouseState() == ofxCvGui::Element::Down ? 2.0f : 1.0f);
		ofRect(0,0,100,30);
		ofPopStyle();
	};
	calibrateButton->onHit += [this] (ofVec2f&) {
		//check we have selections for both
		if (this->userSelection.count(0) == 0 || this->userSelection.count(1) == 0) {
			return;
		}
		this->server.addAlignment(1, 0, this->userSelection[1], this->userSelection[0]);
	};
	recorderPanel->add(calibrateButton);
	this->calibrateButton = calibrateButton;

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

	if (this->userSelection.count(0) == 0 || this->userSelection.count(1) == 0) {
		this->calibrateButton->disable();
	} else {
		this->calibrateButton->enable();
	}
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
