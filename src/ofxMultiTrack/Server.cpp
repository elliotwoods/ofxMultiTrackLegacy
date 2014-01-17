#include "Server.h"

namespace ofxMultiTrack {
#pragma mark Recording
	//----------
	void Server::Recording::recordIncoming() {
		this->incomingFramesLock.lock();
		for(auto frame : this->incomingFrames) {
			this->frames.insert(frame);
		}
		this->incomingFrames.clear();
		this->incomingFramesLock.unlock();
	}

	//----------
	void Server::Recording::clearIncoming() {
		this->incomingFramesLock.lock();
		this->incomingFrames.clear();
		this->incomingFramesLock.unlock();
	}

	//----------
	void Server::Recording::addIncoming(UserSet userSet) {
		this->incomingFramesLock.lock();
		this->incomingFrames.insert(std::pair<Timestamp, UserSet>(ofGetElapsedTimeMicros(), userSet));
		this->incomingFramesLock.unlock();
	}

	//----------
	void Server::Recording::clear() {
		this->frames.clear();
		this->clearIncoming();
	}

	//----------
	Server::Recording::FrameSet & Server::Recording::getFrames() {
		return this->frames;
	}

#pragma mark NodeConnection
	//----------
	Server::NodeConnection::NodeConnection(string address, int index) {
		this->address = address;
		this->index = index;
		this->running = true;
		this->threadEnded = false;

		this->cachedConnected = false;

		this->startThread(true, false);
	}

	//----------
	Server::NodeConnection::~NodeConnection() {
		this->running = false;
		this->stopThread();
		while(!this->threadEnded) {
			ofSleepMillis(1);
		}
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
	Server::Recording & Server::NodeConnection::getRecording() {
		return this->recording;
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
					this->recording.addIncoming(this->users);
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
		this->threadEnded = true;
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

#pragma mark Recorder
	//----------
	Server::Recorder::Recorder(const NodeSet & nodes) : nodes(nodes) {
		this->state = Server::Recorder::Waiting;
	}

	//----------
	void Server::Recorder::update() {
		if (this->playHead < this->getStartTime()) {
			this->playHead = this->getStartTime();
		}
		if (this->playHead > this->getEndTime()) {
			this->playHead = this->getEndTime();
		}

		if (this->isRecording()) {
			for(auto node : this->nodes) {
				node->getRecording().recordIncoming();
			}
		} else {
			for(auto node : this->nodes) {
				node->getRecording().clearIncoming();
			}
		}
	}

	//----------
	void Server::Recorder::clear() {
		for(auto node : this->nodes) {
			node->getRecording().clear();
		}
	}
	
	//----------
	Timestamp Server::Recorder::getStartTime() {
		auto maximum = std::numeric_limits<Timestamp>::max();
		auto start = maximum;
		for(auto node : this->nodes) {
			auto & frames = node->getRecording().getFrames();
			if (frames.size() == 0) {
				continue;
			}
			auto firstForThisNode = frames.begin()->first;
			if (firstForThisNode < start) {
				start = firstForThisNode;
			}
		}
		if (start == maximum) {
			return 0.0f;
		} else {
			return start;
		}
	}

	//----------
	Timestamp Server::Recorder::getEndTime() {
		auto minimum = std::numeric_limits<Timestamp>::min();
		auto end = minimum;
		for(auto node : this->nodes) {
			auto & frames = node->getRecording().getFrames();
			if (frames.size() == 0) {
				continue;
			}
			auto last = frames.end();
			last--;
			auto firstForThisNode = last->first;
			if (firstForThisNode > end) {
				end = firstForThisNode;
			}
		}
		if (end == minimum) {
			return 0.0f;
		} else {
			return end;
		}
	}

#pragma mark Server
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
		auto newNode = new NodeConnection(address, index);
		this->nodes.push_back(shared_ptr<NodeConnection>(newNode));
	}

	//----------
	void Server::clearNodes() {
		this->nodes.clear(); // presume smart pointers do their thing
	}

	//----------
	const Server::NodeSet & Server::getNodes() {
		return this->nodes;
	}

	//----------
	Server::Recorder & Server::getRecorder() {
		return this->recorder;
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