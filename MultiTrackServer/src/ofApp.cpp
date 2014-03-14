#include "ofApp.h"
#include "Version.h"
#include "ofxMultiTrack/src/ofxMultiTrack/Utils/Constants.h"

//--------------------------------------------------------------
void ofApp::setup(){
	ofSetFrameRate(100.0f);

	//turn on alpha testing
	glAlphaFunc(GL_GREATER, 0.1f);
	glEnable(GL_ALPHA_TEST);

	this->drawMode = World;
	std::fill(this->userSlots, this->userSlots + 6, -1); //set the user slots to be blank

	//read config
	auto configText = ofFile("config.json").readToBuffer().getText();
	Json::Reader reader;
	Json::Value configJson;
	reader.parse(configText, configJson);

	//initialise the server (the important bit)
	server.init();
	{
		int nodeIndex = 0;
		for(auto & node : configJson["nodes"]) {
			const auto deviceIndex = node["deviceIndex"].asInt();
			const auto nodeAddress = node["address"].asString();
			server.addNode(nodeAddress, deviceIndex);
			if (!node["name"].empty()) {
				this->server.getNodes().back()->setName(node["name"].asString());
			}
			if (!node["defaultParent"].empty()) {
				const auto defaultParent = node["defaultParent"].asInt();
				this->defaultParents.insert(pair<int, int>(nodeIndex, defaultParent));
			}
			nodeIndex++;
		}
	}

	//setup osc sender
	oscSender.setup(configJson["osc"]["address"].asString(), configJson["osc"]["port"].asInt());

	//setup mesh osc senders from nodes. this should be moved to API
	Json::Value jsonSetupOscRoot;
	auto & jsonSetupOsc = jsonSetupOscRoot["modules"]["Mesh"]["client"];
	jsonSetupOsc["address"] = configJson["osc"]["address"];
	jsonSetupOsc["port"] = configJson["osc"]["meshPort"];
	this->server.addNodeConfig(jsonSetupOscRoot);

	//load calibration file if defined in config
	if(configJson["calibrationFile"].isString()) {
		server.loadCalibration(configJson["calibrationFile"].asString());
	}

	//--
	//gui code
	//
	gui.init();
	auto leftColumn = gui.addGrid();

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

		ofDrawAxis(1.0f);
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
					auto textBounds = ofxCvGui::Utils::drawText(label, 20, y, true, 20, 100);
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

	//set the initial view positio to be standing up next to the first kinect
	auto & camera = worldPanel->getCamera();
	camera.setPosition(ofVec3f(0.5f, 1.0f, -1.0f));
	camera.lookAt(ofVec3f(0.0f, 0.0f, 3.0f));

	//draw recorder tracks in interactive scrollable panel
	this->recorderControl = ofPtr<RecorderControl>(new RecorderControl(server.getRecorder()));
	recorderPanel->add(this->recorderControl);
	auto & nodes = this->server.getNodes();
	for(auto node : nodes) {
		auto track = ofxCvGui::ElementPtr(new RecordingControl(server.getRecorder(), node->getRecording(), node, nodes));
		recorderPanel->add(track);
	}
	auto calibrateButton = ofPtr<ofxCvGui::Utils::Button>(new ofxCvGui::Utils::Button);
	calibrateButton->setBounds(ofRectangle(0,0,100,30));
	calibrateButton->onDraw += [calibrateButton] (ofxCvGui::DrawArguments &) {
		ofxCvGui::Utils::drawText("Calibrate", 0, 0, true, 30, 100);
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
			this->server.getRecorder().load(file);
		}
	};

	/*
	//draw status in a scrollable panel
	auto statusPanel = gui.addScroll("Status");
	auto statusElement = ofxCvGui::ElementPtr(new ofxCvGui::Element);
	statusPanel->add(statusElement);
	statusElement->onDraw += [this, statusElement] (ofxCvGui::DrawArguments&) {
		auto status = server.getStatusString();
		status = "Build #: " + ofToString(getBuildNumber()) + "\n" + status;
		ofDrawBitmapString(status, 10, 20);
		auto resizeBounds = statusElement->getBounds();
		resizeBounds.height = (std::count(status.begin(), status.end(), '\n') + 1 ) * 14;
		statusElement->setBounds(resizeBounds);
	};
	*/

	auto inspector = gui.addInspector();
	inspector->onClear += [this] (ofxCvGui::ElementGroupPtr & elements) {
		elements->add(shared_ptr<ofxCvGui::Widgets::Title>(new ofxCvGui::Widgets::Title("Global", ofxCvGui::Widgets::Title::Level::H2)));
		
		auto buildValue = shared_ptr<ofxCvGui::Widgets::LiveValue<float> >(new ofxCvGui::Widgets::LiveValue<float>("Build number", [] () { return getBuildNumber(); }));
		elements->add(buildValue);

		auto fpsValue = shared_ptr<ofxCvGui::Widgets::LiveValue<float> >(new ofxCvGui::Widgets::LiveValue<float>("Server framerate", [] () { return ofGetFrameRate(); }));
		elements->add(fpsValue);

		elements->add(shared_ptr<ofxCvGui::Widgets::Spacer>(new ofxCvGui::Widgets::Spacer()));
	};

	//arrange grid on resize
	auto rootGrid = (ofxCvGui::Panels::Groups::Grid *) gui.getController().getRootGroup().get();
	rootGrid->onBoundsChange += [rootGrid, leftColumn] (ofxCvGui::BoundsChangeArguments &) {
		vector<float> widths;
		if (ofGetWidth() > 1024) {
			widths.push_back(ofGetWidth() - 512);
			widths.push_back(512);
		}
		rootGrid->setWidths(widths);

		vector<float> heights;
		if (ofGetHeight() > 768) {
			heights.push_back(ofGetHeight() - (768 / 2));
			heights.push_back(768 / 2);
		}
		leftColumn->setHeights(heights);
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

	const auto currentFrame = this->server.getCurrentFrame();
	const auto userSet = currentFrame.combined;

	//--
	//update userSlots

	//get a list of globalIndices available
	set<ofxMultiTrack::ServerData::User::GlobalIndex> currentGlobalIndices;
	for(const auto user : userSet) {
		currentGlobalIndices.insert(user.getGlobalIndex());
	}
	auto unusedGlobalIndices = currentGlobalIndices;

	//clear out dead users
	for(int i=0; i < USER_SLOT_COUNT; i++) {
		bool nowDead = true;
		for(const auto user : userSet) {
			if (user.getGlobalIndex() == this->userSlots[i]) {
				unusedGlobalIndices.erase(user.getGlobalIndex());
				nowDead = false;
				break; //this won't be true for anything else, so don't waste time
			}
		}
		if (nowDead) {
			this->userSlots[i] = -1;
		}
	}

	//assign any empty slots
	for(int i=0; i < USER_SLOT_COUNT; i++) {
		if (this->userSlots[i] == -1) {
			//we have an empty slot
			if (!unusedGlobalIndices.empty()) {
				//we have a user to put in this slot
				auto globalIndex = * unusedGlobalIndices.begin();
				this->userSlots[i] = globalIndex;
				unusedGlobalIndices.erase(globalIndex);
			}
		}
	}

	//fill empty slots with users where available
	//--

	//--
	//send users
	//
	for(auto slot = 0; slot < USER_SLOT_COUNT; slot++) {
		bool foundUser = false;
		ofxOscBundle oscBundle;
		string baseUserAddress = "/daikon/user/" + ofToString(slot);
		for(auto & user : userSet) {
			if (user.getGlobalIndex() == this->userSlots[slot]) {
				foundUser = true;

				ofxOscMessage indexMessage;
				indexMessage.setAddress(baseUserAddress + "/index");
				indexMessage.addIntArg(user.getGlobalIndex());
				oscBundle.addMessage(indexMessage);

				for(auto & joint : user) {
					ofxOscMessage jointMessage;
					jointMessage.setAddress(baseUserAddress + "/skeleton/" + joint.first + "/pos");
				
					auto jointPosition = joint.second.position;

					// flip y and z to match mocap data
					// memo : (to make coordinate system same as mocap data, worldup == +ve z, 1 unit == cm)
					// elliotwoods : i'd prefer to stick with our coords, and have any hacks like this on client side
					// elliotwoods : i'd prefer to stick with meters
					jointPosition = ofVec3f(jointPosition.x, jointPosition.z, jointPosition.y) * 100.0f;

					for(int i=0; i<3; i++) {
						jointMessage.addFloatArg(jointPosition[i]);
					}
					oscBundle.addMessage(jointMessage);
				}
			}
		}
		ofxOscMessage activeMessage;
		activeMessage.setAddress(baseUserAddress + "/active");
		activeMessage.addIntArg(foundUser ? 1 : 0);
		oscBundle.addMessage(activeMessage);

		this->oscSender.sendBundle(oscBundle);
	}
	//
	//--
}

//--------------------------------------------------------------
void ofApp::draw(){
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
	auto & recorder = this->server.getRecorder();
	auto selectedNode = this->getSelectedNode();
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
		if (this->source.selectionValid && this->target.selectionValid) {
			this->server.addAlignment(this->target.node, this->source.node, this->target.user, this->source.user);
		}
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
		this->server.autoCalibrate(this->defaultParents);
		break;
	case 'l':
		this->server.loadCalibration();
		break;
	case 's':
		this->server.saveCalibration();
		break;
	case 'o':
		this->server.applyOriginPose();
		break;
	case OF_KEY_BACKSPACE:
		if (selectedNode) {
			selectedNode->clearTransform();
		}
		break;
	case 'e':
		if (selectedNode) {
			selectedNode->setEnabled(!selectedNode->isEnabled());
		}
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

//--------------------------------------------------------------
shared_ptr<ofxMultiTrack::ServerData::NodeConnection> ofApp::getSelectedNode() {
	const auto selections = ofxCvGui::Panels::Inspector::getSelection();
	const auto serverNodes = this->server.getNodes();
	for(auto selection : selections) {
		auto recordingControl = (RecordingControl*) (selection);
		for(auto node : serverNodes) {
			if (node == recordingControl->getNode()) {
				return node;
			}
		}
	}
	return shared_ptr<ofxMultiTrack::ServerData::NodeConnection>();
}
