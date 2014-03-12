#include "Server.h"
#include "Utils/Constants.h"

namespace ofxMultiTrack {
	//----------
	Server::Server() : recorder(this->nodes) {
	}

	//----------
	Server::~Server() {
		this->clearNodes();
	}

	//----------
	bool Server::init() {
		//this could be moved out to a singleton later
		ofDisableArbTex(); // all our assets are pow2, and this is required for point sprites
		ofxAssets::AssetRegister.addAddon("ofxMultiTrack");
		ofEnableArbTex();
		return true;
	}

	//----------
	void Server::update() {
		this->recorder.update();
		
		//make a copy of previous frame
		this->previousFrame = this->currentFrame;

		//get data in view spaces
		ServerData::OutputFrame frame;
		int nodeIndex = 0;
		if (this->recorder.hasData() && !this->recorder.isRecording()) {
			//get data from recording
			for(auto node : this->nodes) {
				if (node->isEnabled()) {
					auto nodeFrame = node->getRecording().getFrame(this->recorder.getPlayHead());
					frame.views.push_back(nodeFrame);
				} else {
					frame.views.push_back(ServerData::UserSet());
				}
			}
		} else {
			frame.views = this->nodes.getUsersView();
		}

		//get data in world space
		frame.world = this->nodes.getUsersWorld(currentFrame.views);

		//get combined user set
		frame.combined = this->nodes.getUsersCombined(currentFrame.world);
		frame.combined.matchFromPreviousFrame(this->previousFrame.combined);
		frame.combined.assignForEmptyGlobalIndices();

		this->currentFrame = frame;
	}

	//----------
	void Server::addNode(string address, int index) {
		auto newNode = new ServerData::NodeConnection(address, index, this->nodes);
		this->nodes.push_back(shared_ptr<ServerData::NodeConnection>(newNode));
		newNode->loadConfig();
	}

	//----------
	void Server::clearNodes() {
		this->nodes.clear(); // presume smart pointers do their thing
	}

	//----------
	void Server::clearNodeUsers() {
		for(auto node : this->nodes) {
			node->clearUsers();
		}
	}

	//----------
	const ServerData::NodeSet & Server::getNodes() const {
		return this->nodes;
	}

	//----------
	ServerData::Recorder & Server::getRecorder() {
		return this->recorder;
	}

	//----------
	ServerData::OutputFrame Server::getCurrentFrame() const {
		return this->currentFrame;
	}
	
	//----------
	void draw(const vector<ServerData::UserSet> & views) {
		int nodeIndex = 0;
		for(auto & view : views) {
			//--
			//point sprites
			//
			auto & pointSprite = ofxAssets::image("ofxMultiTrack::" + ofToString(nodeIndex));
			pointSprite.bind();
			ofEnablePointSprites();
			//
			//--

			view.draw();
			
			//--
			//clear point sprites
			//
			ofDisablePointSprites();
			pointSprite.unbind();
			//
			//--

			nodeIndex++;
		}
	}

	//----------
	void Server::drawViews() const {
		this->drawViewConeView();
		
		glPushAttrib(GL_POINT_BIT);
		glPointSize(16.0f);
		
		draw(this->currentFrame.views);
		
		glPopAttrib();
	}

	//----------
	void Server::drawWorld() const {
		this->drawViewConesWorld();

		glPushAttrib(GL_POINT_BIT);
		glPointSize(12.0f);

		//draw sources
		draw(this->currentFrame.world);

		//draw combined in geen
		ofPushStyle();
		ofSetColor(0, 255, 0);
		glEnable(GL_POINT_SMOOTH);
		glPointSize(16.0f);
		this->currentFrame.combined.draw(false);

		ofPopStyle();
		glPopAttrib();
	}

	//----------
	void Server::drawViewConesWorld() const {
		ofPushStyle();
		ofEnableAlphaBlending();
		glPushAttrib(GL_POINT_BIT);
		glPointSize(24.0f);
		ofEnablePointSprites();

		const auto fovY = 43.0f;
		const auto fovX = 57.0f;
		const auto gradientX = tan(DEG_TO_RAD * fovX / 2.0f);
		const auto gradientY = tan(DEG_TO_RAD * fovY / 2.0f);

		int nodeIndex = 0;
		for(auto node : this->nodes) {
			//a transform goes from kienct view to world, so we can use it to draw our cone
			ofMesh viewCone;

			const auto position = this->nodes[nodeIndex]->applyTransform(ofVec3f(0,0,0));
			viewCone.addVertex(position);
			viewCone.addColor(ofColor(255));
			viewCone.addVertex(this->nodes[nodeIndex]->applyTransform(ofVec3f(-gradientX,-gradientY,1)));
			viewCone.addColor(ofColor(0, 0, 0, 0));
			viewCone.addVertex(this->nodes[nodeIndex]->applyTransform(ofVec3f(+gradientX,-gradientY,1)));
			viewCone.addColor(ofColor(0, 0, 0, 0));
			viewCone.addVertex(this->nodes[nodeIndex]->applyTransform(ofVec3f(-gradientX,+gradientY,1)));
			viewCone.addColor(ofColor(0, 0, 0, 0));
			viewCone.addVertex(this->nodes[nodeIndex]->applyTransform(ofVec3f(+gradientX,+gradientY,1)));
			viewCone.addColor(ofColor(0, 0, 0, 0));

			const ofIndexType indices[] = {0, 1, 0, 2, 0, 3, 0, 4};
			viewCone.addIndices(indices, 8);
			viewCone.setMode(OF_PRIMITIVE_LINES);
			viewCone.draw();

			ofMesh viewIndex;
			viewIndex.setMode(OF_PRIMITIVE_POINTS);
			viewIndex.addVertex(position);
			auto & pointSprite = ofxAssets::image("ofxMultiTrack::" + ofToString(nodeIndex));
			pointSprite.bind();
			viewIndex.draw();
			pointSprite.unbind();

			nodeIndex++;
		}

		ofDisablePointSprites();
		glPopAttrib();
		ofPopStyle();
	}

	//----------
	void Server::drawViewConeView() const {
		ofPushStyle();
		ofEnableAlphaBlending();

		const auto fovY = 43.0f;
		const auto fovX = 57.0f;
		const auto gradientX = tan(DEG_TO_RAD * fovX / 2.0f);
		const auto gradientY = tan(DEG_TO_RAD * fovY / 2.0f);

		int nodeIndex = 0;
		for(auto node : this->nodes) {
			//a transform goes from kienct view to world, so we can use it to draw our cone
			ofMesh viewCone;

			viewCone.addVertex(ofVec3f(0,0,0));
			viewCone.addColor(ofColor(255));
			viewCone.addVertex(ofVec3f(-gradientX,-gradientY,1));
			viewCone.addColor(ofColor(0, 0, 0, 0));
			viewCone.addVertex(ofVec3f(+gradientX,-gradientY,1));
			viewCone.addColor(ofColor(0, 0, 0, 0));
			viewCone.addVertex(ofVec3f(-gradientX,+gradientY,1));
			viewCone.addColor(ofColor(0, 0, 0, 0));
			viewCone.addVertex(ofVec3f(+gradientX,+gradientY,1));
			viewCone.addColor(ofColor(0, 0, 0, 0));

			const ofIndexType indices[] = {0, 1, 0, 2, 0, 3, 0, 4};
			viewCone.addIndices(indices, 8);
			viewCone.setMode(OF_PRIMITIVE_LINES);
			viewCone.draw();

			nodeIndex++;
		}
		ofPopStyle();
	}

	//----------
	Json::Value Server::getStatus() {
		Json::Value status;
		status["fps"] = ofGetFrameRate();
		status["nodeCount"] = this->nodes.size();
		auto & nodes = status["nodes"];
		int i = 0;
		for(auto node : this->nodes) {
			auto nodeStatus = node->getStatus();
			nodes[i++] = nodeStatus;
		}
		return status;
	}

	//----------
	string Server::getStatusString() {
		Json::StyledWriter writer;
		return "status = " + writer.write(this->getStatus());
	}

	//----------
	void Server::autoCalibrate() {
		auto graph = ofxTSP::Problem();
		auto solver = ofxTSP::BruteForce();

		//generate a problem space
		int sourceNodeIndex = 0;
		for(auto node : this->nodes) {
			for(int otherNodeIndex = sourceNodeIndex+1; otherNodeIndex < this->nodes.size(); otherNodeIndex++) {
				// search for valid pairs, like we did in addAlignment. 
				// in fact, we should be splitting that code out into findUsefulTimestampPairs(int sourceIndex, int targetIndex)
			}
			sourceNodeIndex++;
		}
	}

	//----------
	void Server::addAlignment(int nodeIndex, int originNodeIndex, int userIndex, int originUserIndex, Align::Ptr routine) {
		try {
			//check nodes exist
			if (nodeIndex > this->nodes.size()) {
				throw(Exception("nodeIndex out of bounds"));
			}
			if (originNodeIndex > this->nodes.size()) {
				throw(Exception("originNodeIndex out of bounds"));
			}

			//local references
			auto & targetNode = this->nodes[nodeIndex];
			auto & originNode = this->nodes[originNodeIndex];

			//--
			//build the training set
			//
			vector<ofVec3f> targetPoints;
			vector<ofVec3f> originPoints;

			//iterate frames in target's recording
			auto & targetFrames = targetNode->getRecording().getFrames();
			auto & originFrames = originNode->getRecording().getFrames();

			//first check we've got frames in both
			if (targetFrames.empty()) {
				throw(Exception("No frames recorded for target node"));
			}
			if (originFrames.empty()) {
				throw(Exception("No frames recorded for origin node"));
			}

			cout << "Frames for " << nodeIndex << ":" << userIndex << " with origin defined as" << originNodeIndex << ":" << originUserIndex << " = ";
			for(auto & targetFrame : targetFrames) {
				auto targetFrameTimestamp = targetFrame.first;

				if (targetFrameTimestamp < this->recorder.getInPoint() || targetFrameTimestamp > this->recorder.getOutPoint()) {
					cout << "-";
					continue;
				}

				//--
				//get closest frame
				//

				//get originFrame >= to targetFrame
				auto originFrame = originFrames.lower_bound(targetFrameTimestamp);
				if (originFrame == originFrames.end())
				{
					//all source frames are before this target frame
					//closest is last
					originFrame--;
				} else if (originFrame == originFrames.begin()) {
					//first sourceFrame is after target frame, we have our answer
				} else {
					auto after = originFrame;
					auto before = after;
					before--;

					auto beforeTime = before->first;
					auto afterTime = after->first;

					if(abs(beforeTime - targetFrameTimestamp) < abs(afterTime - targetFrameTimestamp)) {
						//before is closer
						originFrame = before;
					} else {
						//after is closer, but that's already accounted for.
						//presume optimiser gets rid of this else block
					}
				}

				//
				//--

				//check if source and target frames are too far apart in time
				if(abs(originFrame->first - targetFrameTimestamp) > OFXMULTITRACK_SERVER_ALIGN_MAX_TIME_DIFFERENCE) {
					cout << "T";
					continue;
				}

				auto & targetUserSet = targetFrame.second;
				auto & sourceUserSet = originFrame->second;

				//check these frames have these users
				if (targetUserSet.size() <= userIndex || sourceUserSet.size() <= userIndex) {
					cout << "U";
					continue;
				}

				auto & targetUser = targetUserSet[userIndex];
				auto & originUser = sourceUserSet[originUserIndex];
				auto & targetJoint = targetUser.find(OFXMULTITRACK_SERVER_ALIGN_REFERENCE_JOINT);
				auto & targetOtherJoint = targetUser.find(OFXMULTITRACK_SERVER_ALIGN_OTHER_JOINT);
				auto & originJoint = originUser.find(OFXMULTITRACK_SERVER_ALIGN_REFERENCE_JOINT);
				auto & originOtherJoint = originUser.find(OFXMULTITRACK_SERVER_ALIGN_OTHER_JOINT);

				//check we have the joints
				if (targetJoint == targetUser.end() || originJoint == originUser.end() || targetOtherJoint == targetUser.end() || originOtherJoint == originUser.end()) {
					cout << "J";
					continue;
				}

				//check if right hand is above left hand
				//generally a back-to-front checker
				/*if(targetOtherJoint->second.position.y < targetJoint->second.position.y || originOtherJoint->second.position.y < originOtherJoint->second.position.y) {
					cout << "U";
					continue;
				}*/

				//check that we're not using inferred or untracked joints
				if (targetJoint->second.inferred || originJoint->second.inferred) {
					cout << "I";
					continue;
				}
				if (!targetJoint->second.tracked || !originJoint->second.tracked) {
					cout << "N";
					continue;
				}

				targetPoints.push_back(targetJoint->second.position);
				originPoints.push_back(originJoint->second.position);
				cout << "+";
			}
			cout << endl;
			//
			//--

			//check we have enough data
			if (targetPoints.size() < 3) {
				throw(Exception("insufficient correlation points"));
			}

			//perform the calibration. target points will be transformed to match origin space
			routine->calibrate(targetPoints, originPoints);

			//check if the new parent would reference this node
			const auto parentInfluenceList = this->nodes[originNodeIndex]->getInfluenceList();
			const auto findInList = std::find(parentInfluenceList.begin(), parentInfluenceList.end(), nodeIndex);
			if (findInList != parentInfluenceList.end()) {
				//if so, clear all influences from parent
				this->nodes[originNodeIndex]->clearTransform();
			}

			//assign the calibration in the NodeSet
			this->nodes[nodeIndex]->setTransform(ServerData::NodeConnection::Transform(originNodeIndex, routine));
		} catch (std::exception e) {
			ofLogError("ofxMultiTrack") << "Failed to create alignment for node #" << nodeIndex << " from origin node #" << originNodeIndex;
			ofLogError("ofxMultiTrack") << e.what();
		}
	}

	// Taken from ofxRay
	ofVec3f closestPointOnLineToPoint(const ofVec3f & s, const ofVec3f & t, const ofVec3f & p) {
		return s + (t * (p - s).dot(t) / t.lengthSquared());
	}
	float distanceLineToPoint(const ofVec3f & s, const ofVec3f & t, const ofVec3f & p) {
		return (p - s).cross(p - (s+t)).length() / t.length();
	}

	//----------
	void Server::applyOriginPose() {
		auto const & nodeViews = this->currentFrame.views;
		for(int nodeIndex = 0; nodeIndex < this->nodes.size(); nodeIndex++) {
			auto const & nodeView = nodeViews[nodeIndex];
			auto & node = this->nodes[nodeIndex];

			//check if there's a user in this node
			if (node->isRoot()) {
				//count seen users
				int seenUsers = 0;
				int firstSeenUser = -1;
				for(const auto & user : nodeView) {
					if (seenUsers == 0) {
						firstSeenUser++;
					}

					if (user.isAlive() && user.hasTrackedJoints()) {
						seenUsers++;
					}
				}
				if (seenUsers == 1) {
					const auto & user = nodeView[firstSeenUser];
					node->applyOriginPose(user);
				} else {
					ofLogWarning("ofxMultiTrack") << "Cannot calibrate node [" << nodeIndex << "] as it does not have 1 user seen (" << nodeView.size() << " users seen)";
				}
			}
		}
	}

	//----------
	void Server::serialise(Json::Value & json) const {
		json["nodeCount"] = this->nodes.size();
		for(int i=0; i<this->nodes.size(); i++) {
			auto & jsonNode = json["nodes"][i];
			const auto transform = this->nodes[i]->getTransform();
			bool hasTransform = (transform.getTransform());
			jsonNode["hasTransform"] = hasTransform;
			if (hasTransform) {
				auto & jsonTransform = jsonNode["transform"];
				jsonTransform["parent"] = transform.getParent();
				jsonTransform["type"] = transform.getTransform()->getType();
				jsonTransform["parameters"] = transform.getTransform()->serialise();
			}
		}
	}

	//----------
	void Server::deserialise(const Json::Value & json) {
		int jsonNodeCount = json["nodeCount"].asInt();
		for(int i=0; i<jsonNodeCount; i++) {
			if (i < this->nodes.size()) {
				auto & jsonNode = json["nodes"][i];
				bool hasTransform = jsonNode["hasTransform"].asBool();
				ServerData::NodeConnection::Transform newTransform;
				if (hasTransform) {
					auto & jsonTransform = jsonNode["transform"];
					auto parent = jsonTransform["parent"].asInt();
					auto type = jsonTransform["type"].asString();
					auto newAlign = Align::Factory::make(type);
					newAlign->deserialise(jsonTransform["parameters"]);
					newTransform = ServerData::NodeConnection::Transform(parent, newAlign);
				}
				this->nodes[i]->setTransform(newTransform);
			}
		}
	}

	//----------
	void Server::saveCalibration(string filename) const {
		if (filename=="") {
			auto response = ofSystemSaveDialog("calibration.json", "Save calibration");
			if (!response.bSuccess) {
				ofLogWarning("ofxMultiTrack") << "No file selected for save";
				return;
			}
			filename = response.filePath;
		}

		Json::Value json;
		this->serialise(json);

		Json::StyledWriter writer;
		ofFile output;
		output.open(filename, ofFile::WriteOnly, false);
		output << writer.write(json);
	}

	//----------
	void Server::loadCalibration(string filename) {
		if (filename=="") {
			auto response = ofSystemLoadDialog("Load calibration");
			if (!response.bSuccess) {
				ofLogWarning("ofxMultiTrack") << "No file selected for load";
				return;
			}
			filename = response.filePath;
		}

		ofFile input;
		input.open(ofToDataPath(filename, true), ofFile::ReadOnly, false);
		string jsonRaw = input.readToBuffer().getText();

		try {
			Json::Reader reader;
			Json::Value json;
			reader.parse(jsonRaw, json);
			this->deserialise(json);
		} catch (std::exception e) {
			ofSystemAlertDialog(e.what());
		}
	}

	//----------
	void Server::addNodeConfig(const Json::Value & message) const {
		for(auto node : this->nodes) {
			node->addNodeConfig(message);
		}
	}
}