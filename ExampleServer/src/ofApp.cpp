#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
	ofSetFrameRate(100.0f);
	glAlphaFunc(GL_GREATER, 0.1f);
	glEnable(GL_ALPHA_TEST);

	this->drawMode = View;

	//read config
	auto configText = ofFile("config.json").readToBuffer().getText();
	Json::Reader reader;
	Json::Value configJson;
	reader.parse(configText, configJson);

	//initialise the server (the important bit)
	server.init();
	for(auto node : configJson["nodes"]) {
		server.addNode(node["address"].asString(), node["index"].asInt());
	}

	//setup osc sender
	this->oscSender.setup(configJson["osc"]["address"].asString(), configJson["osc"]["port"].asInt());

	//--
	//gui code
	//
	gui.init();
	auto leftColumn = gui.addGrid();
	auto statusPanel = gui.addScroll("Status");

	auto worldPanel = gui.makeWorld("View");
	this->worldPanel = worldPanel;
	auto recorderPanel = gui.makeScroll("Recorder");
	leftColumn->setColsCount(1);
	leftColumn->add(worldPanel);
	leftColumn->add(recorderPanel);
	
	//draw 3d scene
	worldPanel->onDrawWorld += [this] (ofCamera&) {
		if (this->drawMode == View) {
			this->server.drawViews();
		} else {
			this->server.drawWorld();
		}
		
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
		bool useWorld = this->drawMode == World;
		auto & views = useWorld ? data.world : data.views;
		for(auto & view : views) {
			int userIndex = 0;
			for(auto & user : view) {
				if (user.find(markerJointName) != user.end()) {
					//draw label
					string label = ofToString(nodeIndex) + " : " + ofToString(userIndex);
					auto textBounds = ofxCvGui::AssetRegister.drawText(label, 20, y, "", true, 20, 100);
					y+=40;

					//save label as hit target
					Target target;
					target.bounds = textBounds;
					target.selection.node = nodeIndex;
					target.selection.user = userIndex;
					this->targets.push_back(target);

					bool selectedSource = this->source.node == nodeIndex && this->source.user == userIndex;
					bool selectedTarget = this->target.node == nodeIndex && this->target.user == userIndex;
					bool anySelection = selectedSource || selectedTarget;

					//draw line to hand
					ofPushStyle();
					if (selectedSource) {
						ofSetColor(200, 90, 90);
					} else if (selectedTarget) {
						ofSetColor(90, 200, 90);
					} else {
						ofSetColor(90);
					}
					ofSetLineWidth(2.0f);
					auto endOfTextPoint = textBounds.getCenter() + ofVec2f(textBounds.width / 2.0f, 0.0f);
					auto & camera = worldPanel->getCamera();
					auto bounds = worldPanel->getBounds();
					bounds.x = 0; // we want local bounds
					bounds.y = 0;
					ofLine(endOfTextPoint, camera.worldToScreen(user[markerJointName].position, bounds));
					ofPopStyle();

					//draw outline if selected
					if (anySelection) {
						ofPushStyle();
						ofSetColor(selectedSource ? ofColor(200, 90, 90) : ofColor(90, 200, 90));
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
				auto & calibNode = (args.button == 0 ? this->source : this->target);
				calibNode = target.selection;
				calibNode.selectionValid = true;
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
		if (this->source.selectionValid && this->target.selectionValid) {
			this->server.addAlignment(this->target.node, this->source.node, this->target.user, this->source.user);
		}
	};
	recorderPanel->add(calibrateButton);
	this->calibrateButton = calibrateButton;
	recorderPanel->ofFilesDragged += [this] (ofxCvGui::FilesDraggedArguments & args) {
		for(auto & file : args.files) {
			cout << "Loading [" << file << "]";
			this->server.getRecorder().load(file);
		}
	};

	//draw status in a scrollable panel
	auto statusElement = ofxCvGui::ElementPtr(new ofxCvGui::Element);
	statusPanel->add(statusElement);
	statusElement->onDraw += [this, statusElement] (ofxCvGui::DrawArguments&) {
		auto status = server.getStatusString();
		status = "Build #: " + ofToString(VERSION_BUILD_NUMBER) + "\n" + status;
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
	//necessary server update
	server.update();

	//enable the calibrate button if we have a source and target selected
	if (this->source.selectionValid && this->target.selectionValid) {
		this->calibrateButton->enable();
	} else {
		this->calibrateButton->disable();
	}

	auto currentFrame = this->server.getCurrentFrame();
	ofxMultiTrack::ServerData::UserSet userSet;

	//--
	//send data
	//

	//check if we're calibrated or note
	if (currentFrame.calibrated) {
		//if so, get the combined user set to send
		userSet = currentFrame.combined;
	} else {
		//otherwise just add all users from all views
		auto data = currentFrame.views;
		vector<ofxMultiTrack::ServerData::User> userSet;
		for(auto & node : data) {
			for(auto & user : node) {
				userSet.push_back(user);
			}
		}
	}

	//send users
	auto userIndex = 0;
	ofxOscBundle oscBundle;
	for(auto & user : userSet) {
			for(auto & joint : user) {
				ofxOscMessage jointMessage;
				//userIndex = 1;	// MEMO HACK TO MAKE VVVV WORK
				jointMessage.setAddress("/daikon/user/" + ofToString(userIndex) + "/skeleton/" + joint.first + "/pos");
				
				// MEMO HACK (to make coordinate system same as mocap data, worldup == +ve z, 1 unit == cm)
				// flip y and z
				ofVec3f p(joint.second.position[0], joint.second.position[2], joint.second.position[1]);
				p *= 100.0;// convert to cm

				for(int i=0; i<3; i++) {
					jointMessage.addFloatArg(p[i]);
				}
				oscBundle.addMessage(jointMessage);
			}
			userIndex++;
	}
	this->oscSender.sendBundle(oscBundle);
	//
	//--
}

//--------------------------------------------------------------
void ofApp::draw(){
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
	auto & recorder = this->server.getRecorder();
	switch (key) {
	case ' ':
		switch(recorder.getState()) {
		case ofxMultiTrack::ServerData::Recorder::Playing:
		case ofxMultiTrack::ServerData::Recorder::Recording:
			recorder.stop();
			break;
		case ofxMultiTrack::ServerData::Recorder::Waiting:
			recorder.play();
			break;
		}
		break;
	case 'c':
		this->server.clearNodeUsers();
		break;
	case 'v':
		this->drawMode = View;
		this->worldPanel->setCaption("View");
		break;
	case 'w':
		this->drawMode = World;
		this->worldPanel->setCaption("World");
		break;
	case 'a':
		ofxAssets::AssetRegister.refresh();
		break;
	}
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
