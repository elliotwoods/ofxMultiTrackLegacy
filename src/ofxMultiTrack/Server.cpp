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
		return "status = " + writer.write(this->getStatus());
	}
}