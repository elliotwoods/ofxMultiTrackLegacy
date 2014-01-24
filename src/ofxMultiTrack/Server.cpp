#include "Server.h"

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
		return true;
	}

	//----------
	void Server::update() {
		this->recorder.update();
	}

	//----------
	void Server::addNode(string address, int index) {
		auto newNode = new ServerData::NodeConnection(address, index);
		this->nodes.push_back(shared_ptr<ServerData::NodeConnection>(newNode));
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
	Server::OutputFrame Server::getCurrentFrame() const {
		OutputFrame currentFrame;

		//get data in view spaces
		int nodeIndex = 0;
		if (this->recorder.hasData() && !this->recorder.isRecording()) {
			//get data from recording
			for(auto node : this->nodes) {
				auto nodeFrame = node->getRecording().getFrame(this->recorder.getPlayHead());
				currentFrame.views.push_back(nodeFrame);
			}
		} else {
			currentFrame.views = this->nodes.getUsersView();
		}

		currentFrame.calibrated = this->nodes.isCalibrated();

		if (currentFrame.calibrated) {
			//get data in world space
			currentFrame.world = this->nodes.getUsersWorld(currentFrame.views);

			//get combined user set
			currentFrame.combined = this->nodes.getUsersCombined();
		}

		return currentFrame;
	}

	//----------
	void Server::drawWorld() const {
		auto currentFrame = this->getCurrentFrame();

		glPushAttrib(GL_POINT_BIT);
		glEnable(GL_POINT_SMOOTH);

		this->drawViewConesWorld();

		if (this->nodes.isCalibrated()) {
			//draw combined skeleton
			glPointSize(5.0f);
			currentFrame.combined.draw();

			//draw world space skeletons per view
			// it would be nice to also draw lines
			// but then we need to know which user
			// index in each view matches each
			// combined index
			auto & sourceMapping = currentFrame.combined.getSourceMapping();
			glPointSize(2.0f);
			ofMesh lines;
			ofMesh points;
			int nodeIndex = 0;
			for(auto & node : currentFrame.world) {
				int userIndex = 0;
				for(auto & user : node) {
					auto combinedUserIndex = sourceMapping.at(userIndex).at(nodeIndex);
					auto & combinedUser = currentFrame.combined[combinedUserIndex];
					for(auto & joint : user) {
						points.addVertex(joint.second.position);
						lines.addVertex(joint.second.position);
						lines.addVertex(combinedUser[joint.first].position);
					}
					userIndex++;
				}
				nodeIndex++;
			}
		} else {
			for(auto & node : currentFrame.views) {
				for(auto & user : node) {
					user.draw();
				}
			}
		}

		glPopAttrib();
	}

	//----------
	void Server::drawViewConesWorld() const {
		ofPushStyle();
		ofEnableAlphaBlending();

		const auto fovY = 43.0f;
		const auto fovX = 57.0f;
		const auto gradientX = tan(DEG_TO_RAD * fovX);
		const auto gradientY = tan(DEG_TO_RAD * fovY);

		int nodeIndex = 0;
		for(auto node : this->nodes) {
			//a transform goes from kienct view to world, so we can use it to draw our cone
			ofMesh viewCone;

			viewCone.addVertex(this->nodes.applyTransform(ofVec3f(0,0,0), nodeIndex));
			viewCone.addColor(ofColor(255);
			viewCone.addVertex(this->nodes.applyTransform(ofVec3f(-gradientX,-gradientY,1), nodeIndex));
			viewCone.addColor(ofColor(0, 0, 0, 0));
			viewCone.addVertex(this->nodes.applyTransform(ofVec3f(+gradientX,-gradientY,1), nodeIndex));
			viewCone.addColor(ofColor(0, 0, 0, 0));
			viewCone.addVertex(this->nodes.applyTransform(ofVec3f(-gradientX,+gradientY,1), nodeIndex));
			viewCone.addColor(ofColor(0, 0, 0, 0));
			viewCone.addVertex(this->nodes.applyTransform(ofVec3f(+gradientX,+gradientY,1), nodeIndex));
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

			cout << "Frames for " << nodeIndex << ":" << userIndex << " with origin defined as" << originNodeIndex << ":" << originUserIndex << "=";
			for(auto & targetFrame : targetFrames) {
				auto targetFrameTimestamp = targetFrame.first;

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
				auto & originJoint = originUser.find(OFXMULTITRACK_SERVER_ALIGN_REFERENCE_JOINT);

				//check we have the joints
				if (targetJoint == targetUser.end() || originJoint == originUser.end()) {
					cout << "J";
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

			//assign the calibration in the NodeSet
			this->nodes.setTransform(nodeIndex, originNodeIndex, routine);

		} catch (Exception e) {
			ofLogError("ofxMultiTrack") << "Failed to create alignment for node #" << nodeIndex << " from origin node #" << originNodeIndex;
			ofLogError("ofxMultiTrack") << e.what();
		}
	}
}