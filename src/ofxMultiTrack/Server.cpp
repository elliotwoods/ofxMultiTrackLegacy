#include "Server.h"

namespace ofxMultiTrack {
#pragma mark NodeConnection
	//----------
	Server::NodeConnection::NodeConnection(string address, int index) {
		this->address = address;
		this->index = index;
		this->running = true;

		this->cachedConnected = false;

		this->startThread(true, false);
	}

	//----------
	Server::NodeConnection::~NodeConnection() {
		this->running = false;
		this->stopThread();
	}
	
	//----------
	bool Server::NodeConnection::isConnected() {
		return this->cachedConnected;
	}

	//----------
	int Server::NodeConnection::getUserCount() {
		this->lockUsers.lock();
		int count = this->users.size();
		this->lockUsers.unlock();
		return count;
	}

	//----------
	void Server::NodeConnection::threadedFunction() {
		this->client.setVerbose(false);
		while(this->running) {
			if (!this->client.isConnected()) {
				this->client.setup(this->address, OFXMULTITRACK_CLIENT_LISTEN_PORT + index, false);
			}
			auto response = this->client.receive();
			if (response.size() > 0) {
				try
				{
					Json::Value json;
					jsonReader.parse(response, json);
					auto & users = json["modules"][0]["data"]["users"];

					this->lockUsers.lock();
					int userIndex = 0;
					if (this->users.size() < users.size()) {
						this->users.resize(users.size());
					}
					for(auto & user : users) {
						auto & userLocal = this->users[userIndex++];
						auto jointNames = user.getMemberNames();
						for(auto & jointName : jointNames) {
							auto & joint = user[jointName];
							auto & jointLocal = userLocal[jointName];
							auto & position = joint["position"];
							for(int i=0; i<3; i++) {
								jointLocal.position[i] = position[i].asFloat();
							}
							auto & rotation = joint["rotation"];
							for(int i=0; i<3; i++) {
								jointLocal.rotation[i] = rotation[i].asFloat();
							}
						}
					}
				}
				catch(std::exception e)
				{
					ofLogError("ofxMultiTrack") << e.what();
				}
				lockUsers.unlock();
			}

			this->cachedConnected = this->client.isConnected();

			while(actionQueue.size() > 0) {
				lockActionQueue.lock();
				auto function = actionQueue.front();
				function();
				actionQueue.pop_front();
				lockActionQueue.unlock();
			}
		}
	}

	//----------
	void Server::NodeConnection::performBlocking(function<void()> action) {
		lockActionQueue.lock();
		actionQueue.push_back(action);
		lockActionQueue.unlock();

		bool waiting = true;
		while(waiting) {
			lockActionQueue.lock();
			waiting = !actionQueue.empty();
			lockActionQueue.unlock();	
		}
	}

	//----------
	Json::Value Server::NodeConnection::getStatus() {
		Json::Value status;
		status["address"] = this->address;
		status["index"] = this->index;
		status["connected"] = this->isConnected();
		status["users"] = this->getUserCount();
		return status;
	}

#pragma mark Server
	//----------
	Server::Server() {
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
	}

	//----------
	void Server::addNode(string address, int index) {
		auto newNode = new NodeConnection(address, index);
		this->nodes.push_back(ofPtr<NodeConnection>(newNode));
	}

	//----------
	void Server::clearNodes() {
		this->nodes.clear(); // presume smart pointers do their thing
	}

	//----------
	void Server::drawWorld() {
		
	}

	//----------
	string Server::getStatus() {
		Json::Value status;
		status["fps"] = ofGetFrameRate();
		auto & nodes = status["nodes"];
		int i = 0;
		for(auto node : this->nodes) {
			auto nodeStatus = node->getStatus();
			nodes[i++] = nodeStatus;
		}
		Json::StyledWriter writer;
		return writer.write(status);
	}
}