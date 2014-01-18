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
		auto markerJointName = "HAND_RIGHT";
		auto data = this->server.getCurrentFrame();
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
					auto textBounds = ofxCvGui::AssetRegister.drawText(label, 20, y, "", true, 20, 50);
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
