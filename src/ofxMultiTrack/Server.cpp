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

		if (this->recorder.hasData()) {
			//get data from recording
			for(auto node : this->nodes) {
				currentFrame.push_back(node->getRecording().getFrame(this->recorder.getPlayHead()));
			}
		} else {
			//get live data
			for(auto node : this->nodes) {
				currentFrame.push_back(node->getLiveData());
			}
		}

		return currentFrame;
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
		return "status : " + writer.write(this->getStatus());
	}

	//----------
	void Server::addAlignment(int nodeIndex, int sourceNodeIndex, int userIndex, int sourceUserIndex, Align::Ptr routine) {
		try {
			//check nodes exist
			if (nodeIndex > this->nodes.size()) {
				throw(std::exception("nodeIndex out of bounds"));
			}
			if (sourceNodeIndex > this->nodes.size()) {
				throw(std::exception("sourceNodeIndex out of bounds"));
			}

			//local references
			auto & targetNode = this->nodes[nodeIndex];
			auto & sourceNode = this->nodes[nodeIndex];

			//--
			//build the training set
			//
			vector<ofVec3f> sourcePoints;
			vector<ofVec3f> targetPoints;

			//iterate frames in target's recording
			auto & targetFrames = targetNode->getRecording().getFrames();
			auto & sourceFrames = sourceNode->getRecording().getFrames();

			//first check we've got frames in both
			if (targetFrames.empty()) {
				throw(std::exception("No frames recorded for target node"));
			}
			if (sourceFrames.empty()) {
				throw(std::exception("No frames recorded for source node"));
			}

			for(auto & targetFrame : targetFrames) {
				auto targetFrameTimestamp = targetFrame.first;

				//--
				//get closest frame
				//

				//get sourceFrame >= to targetFrame
				auto sourceFrame = sourceFrames.lower_bound(targetFrameTimestamp);
				if (sourceFrame == sourceFrames.end())
				{
					//all source frames are before this target frame
					//closest is last
					sourceFrame--;
				} else if (sourceFrame == sourceFrames.begin()) {
					//first sourceFrame is after target frame, we have our answer
				} else {
					auto after = sourceFrame;
					auto before = after;
					before--;

					auto beforeTime = before->first;
					auto afterTime = after->first;

					if(abs(beforeTime - targetFrameTimestamp) < abs(afterTime - targetFrameTimestamp)) {
						//before is closer
						sourceFrame = before;
					} else {
						//after is closer, but that's already accounted for
						//presume optimiser gets rid of this
					}
				}

				//
				//--

				//check if source and target frames are too far apart in time
				if(abs(sourceFrame->first - targetFrameTimestamp) > OFXMULTITRACK_SERVER_ALIGN_MAX_TIME_DIFFERENCE) {
					continue;
				}

				auto & targetUserSet = targetFrame.second;
				auto & sourceUserSet = sourceFrame->second;

				//check these frames have these users
				if (targetUserSet.size() <= userIndex || sourceUserSet.size() <= userIndex) {
					continue;
				}

				auto & targetUser = targetUserSet[userIndex];
				auto & sourceUser = sourceUserSet[sourceUserIndex];
				auto & targetJoint = targetUser.find(OFXMULTITRACK_SERVER_ALIGN_REFERENCE_JOINT);
				auto & sourceJoint = sourceUser.find(OFXMULTITRACK_SERVER_ALIGN_REFERENCE_JOINT);

				//check we have the joints
				if (targetJoint == targetUser.end() || sourceJoint == sourceUser.end()) {
					continue;
				}

				targetPoints.push_back(targetJoint->second.position);
				sourcePoints.push_back(sourceJoint->second.position);
			}
			//
			//--

			//check we have enough data
			if (sourcePoints.size() < 3) {
				throw(std::exception("insufficient correlation points"));
			}

			//perform the calibration
			routine->calibrate(sourcePoints, targetPoints);

			//assign the calibration in the NodeSet
			this->nodes.setTransform(nodeIndex, sourceNodeIndex, routine);

		} catch (std::exception e) {
			ofLogError("ofxMultiTrack") << "Failed to create alignment for node #" << nodeIndex << " from source node #" << sourceNodeIndex;
			ofLogError("ofxMultiTrack") << e.what();
		}
	}
}