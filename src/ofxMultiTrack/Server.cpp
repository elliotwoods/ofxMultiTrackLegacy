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
	const UserSet & Server::Recording::getFrame(Timestamp timestamp) {
		if (this->frames.empty()) {
			return blankFrame;
		} else {
			auto foundFrame = this->frames.lower_bound(timestamp);
			if (foundFrame == this->frames.end()) {
				foundFrame = this->frames.end();
				foundFrame--;
			}
			return foundFrame->second;
		}
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

	//----------
	UserSet Server::Recording::blankFrame = UserSet();

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
	UserSet Server::NodeConnection::getLiveData() {
		this->lockUsers.lock();
		auto users = this->users;
		this->lockUsers.unlock();
		return users;
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

		this->remoteStatusLock.lock();
		status["remoteStatus"] = this->remoteStatus;
		this->remoteStatusLock.unlock();

		return status;
	}

	//----------
	void Server::NodeConnection::threadedFunction() {
		this->client.setVerbose(false);
		while(this->running) {
			if (!this->client.isConnected()) {
				this->client.setup(this->address, OFXMULTITRACK_NODE_LISTEN_PORT + index, false);
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

					this->remoteStatusLock.lock();
					this->remoteStatus = json["status"];
					this->remoteStatusLock.unlock();

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
		this->startTime = 0;
		this->endTime = 0;
		this->playHead = 0;
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

		auto currentTime = ofGetElapsedTimeMicros();

		if (this->state == Server::Recorder::Recording) {
			if (currentTime > this->endTime) {
				this->endTime = currentTime;
			}
		}
	}

	//----------
	void Server::Recorder::record() {
		//This is to reset the recording start time
		//But we might want to consider not doing this later (e.g. to avoid losing user data)
		//That would instead suggest having a local timespan, rather than using the app's time
		this->clear();
		this->state = Server::Recorder::Recording;
	}

	//----------
	void Server::Recorder::play() {
		this->state = Server::Recorder::Playing;
	}

	//----------
	void Server::Recorder::stop() {
		this->state = Server::Recorder::Waiting;
	}

	//----------
	Json::Value Server::Recorder::serialise() const {
		Json::Value json;
		int nodeIndex = 0;
		for(auto node : this->nodes) {
			auto & nodeJson = json[nodeIndex++];
			auto & frames = node->getRecording().getFrames();
			for(auto & frame : frames) {
				auto frameJson = frame.second.serialise();
				nodeJson[ofToString(frame.first)] = frameJson;
			}
		}
		return json;
	}

	//----------
	void Server::Recorder::deserialise(const Json::Value & json) {
		if (json.size() != this->nodes.size()) {
			throw(std::exception("Mismatch on deserialise : number of nodes connected does not equal number of nodes in json"));
		}
		int nodeIndex = 0;
		for(auto node : this->nodes) {
			auto & nodeJson = json[nodeIndex++];
			auto & recording = node->getRecording();
			recording.clear();
			auto & frameSet = recording.getFrames();
			auto timestamps = nodeJson.getMemberNames();
			for(auto timestamp : timestamps) {
				Timestamp rawTimestamp = stoll(timestamp);
				auto frame = UserSet();
				frame.deserialise(nodeJson[timestamp]);
				frameSet[rawTimestamp] = frame;
			}
		}
	}

	//----------
	void Server::Recorder::save(string filename) const {
		if (filename=="") {
			auto response = ofSystemSaveDialog("recording.json", "Save recording");
			if (!response.bSuccess) {
				ofLogWarning("ofxMultiTrack") << "No file selected for save";
				return;
			}
			filename = response.filePath;
		}

		auto json = this->serialise();

		Json::FastWriter writer;
		ofFile output;
		output.open(filename, ofFile::WriteOnly, false);
		output << writer.write(json);
	}

	//----------
	void Server::Recorder::load(string filename) {
		if (filename=="") {
			auto response = ofSystemLoadDialog("Load recording");
			if (!response.bSuccess) {
				ofLogWarning("ofxMultiTrack") << "No file selected for load";
				return;
			}
			filename = response.filePath;
		}

		ofFile input;
		input.open(filename, ofFile::ReadOnly, false);
		string jsonRaw = input.readToBuffer().getText();

		try {
			Json::Reader reader;
			Json::Value json;
			reader.parse(jsonRaw, json);
			this->deserialise(json);
		} catch (std::exception e) {
			ofLogError("ofxMultiTrack") << e.what();
		}
	}

	//----------
	void Server::Recorder::clear() {
		for(auto node : this->nodes) {
			node->getRecording().clear();
		}
		this->startTime = ofGetElapsedTimeMicros();
	}
	
	//----------
	Timestamp Server::Recorder::getPlayHead() {
		return this->playHead;
	}

	//----------
	Timestamp Server::Recorder::getStartTime() {
		return this->startTime;
	}

	//----------
	Timestamp Server::Recorder::getEndTime() {
		return this->endTime;
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
	vector<UserSet> Server::getCurrentFrame() {
		vector<UserSet> currentFrame;

		if (!this->recorder.isPlaying()) {
			//get live data
			for(auto node : this->nodes) {
				currentFrame.push_back(node->getLiveData());
			}
		} else {
			//get data from recording
			for(auto node : this->nodes) {
				currentFrame.push_back(node->getRecording().getFrame(this->recorder.getPlayHead()));
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
}