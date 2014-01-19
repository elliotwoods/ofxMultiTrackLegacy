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
	const ServerData::NodeSet & Server::getNodes() {
		return this->nodes;
	}

	//----------
	ServerData::Recorder & Server::getRecorder() {
		return this->recorder;
	}

	//----------
	vector<UserSet> Server::getCurrentFrame() {
		vector<UserSet> currentFrame;

		int nodeIndex = 0;
		if (this->recorder.hasData() && !this->recorder.isRecording()) {
			//get data from recording
			for(auto node : this->nodes) {
				auto nodeFrame = node->getRecording().getFrame(this->recorder.getPlayHead());
				currentFrame.push_back(nodeFrame);
			}
		} else {
			//get live data
			for(auto node : this->nodes) {
				auto nodeFrame = node->getLiveData();
				currentFrame.push_back(nodeFrame);
			}
		}

		this->transformFrame(currentFrame);

		return currentFrame;
	}

	//----------
	void Server::transformFrame(vector<UserSet> & frame) {
		int nodeIndex = 0;
		for(auto & nodeFrame : frame) {
			this->nodes.applyTransform(nodeFrame, nodeIndex++);
		}
	}

	//----------
	void Server::drawWorld() {
		auto currentFrame = this->getCurrentFrame();
		for(auto & node : currentFrame) {
			node.draw();
		}
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
				throw(std::exception("nodeIndex out of bounds"));
			}
			if (originNodeIndex > this->nodes.size()) {
				throw(std::exception("originNodeIndex out of bounds"));
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
				throw(std::exception("No frames recorded for target node"));
			}
			if (originFrames.empty()) {
				throw(std::exception("No frames recorded for origin node"));
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
				throw(std::exception("insufficient correlation points"));
			}

			//perform the calibration. target points will be transformed to match origin space
			routine->calibrate(targetPoints, originPoints);

			//assign the calibration in the NodeSet
			this->nodes.setTransform(nodeIndex, originNodeIndex, routine);

		} catch (std::exception e) {
			ofLogError("ofxMultiTrack") << "Failed to create alignment for node #" << nodeIndex << " from origin node #" << originNodeIndex;
			ofLogError("ofxMultiTrack") << e.what();
		}
	}
}