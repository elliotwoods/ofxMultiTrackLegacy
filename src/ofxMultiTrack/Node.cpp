#include "Node.h"

namespace ofxMultiTrack {
	//----------
	Node::Node() {
		this->settings.localIndex = 0;
	}

	//----------
	bool Node::init() {
		try
		{
			auto & localNodeIndex = this->settings.localIndex;
			//check for existing running clients on this machine
			bool stillLooking = true;
			while(stillLooking) {
				ofxTCPClient checkExisting;
				checkExisting.setup("127.0.0.1", OFXMULTITRACK_NODE_LISTEN_PORT + localNodeIndex, true);
				if (checkExisting.isConnected()) {
					localNodeIndex++;
				} else {
					stillLooking = false;
				}
				if (localNodeIndex >= OFXMULTITRACK_NODE_LOCAL_COUNT_MAX) {
					throw(Exception("Too many clients already running on this machine"));
				}
			}
			ofLogNotice("ofxMultiTrack") << "Local Node index is  " << localNodeIndex;

			//open a server on correct port
			this->server.setup(OFXMULTITRACK_NODE_LISTEN_PORT + localNodeIndex, false);
			ofLogNotice("ofxMultiTrack") << "Node is listening on port " << this->server.getPort();

			//add devices
			auto kinect = shared_ptr<Devices::KinectSDK>(new Devices::KinectSDK(this->settings.localIndex));
			this->devices.push_back(kinect);

			//initialise devices
			for(auto device : this->devices) {
				device->init();
			}

			//add modules
			this->modules.push_back(shared_ptr<Modules::Skeleton>(new Modules::Skeleton(kinect)));

			//initialise modules
			for(auto module : this->modules) {
				module->init();
			}

			//print modules and devices
			ofLogNotice("ofxMultiTrack") << "Initialised Devices:";
			for(auto device : this->devices) {
				ofLogNotice("ofxMultiTrack") << device->getType();
			}
			ofLogNotice("ofxMultiTrack") << "Initialised Modules:";
			for(auto module : this->modules) {
				ofLogNotice("ofxMultiTrack") << module->getType();
			}

			return true;
		} catch (std::exception e) {
			ofLogError("ofxMultiTrack") << "Failed to initialise ofxMultiTrack::Node";
			ofLogError("ofxMultiTrack") << e.what();
			return false;
		}
	}

	//----------
	void Node::update() {
		//check for instructions from any connected ofxMultiTrack::Server's
		for(int iClient = 0; iClient < this->server.getNumClients(); iClient++) {
			if (!this->server.isClientConnected(iClient)) {
				continue;
			}
			size_t numBytes = this->server.getNumReceivedBytes(iClient);
			while(numBytes > 0) {
				ofBuffer rx;
				rx.allocate(numBytes);
				char* bytes = rx.getBinaryBuffer();
				
				this->server.receiveRawBytes(iClient, bytes, rx.size());
				
				if (numBytes >= sizeof(ServerToNodeHeader)) {
					auto * header = (ServerToNodeHeader*) (void*) bytes;
					bytes += sizeof(ServerToNodeHeader);
				}
			}
		}

		//update devices
		for(auto device : this->devices) {
			device->update();
		}

		//update modules
		for(auto module : this->modules) {
			module->update();
		}

		//check if we have any data to send
		bool needsSend = false;
		for(auto device : this->devices) {
			needsSend |= device->isFrameNew();
		}
		
		if (needsSend) {
			//get local dataset
			auto json = this->serialise();

			//send local status also for remote viewing
			json["status"] = this->getStatus();

			Json::StyledWriter writer;
			string result = writer.write(json);

			//send payload to all clients
			server.sendToAll(result); //always sends a [/TCP] from oF (note for other clients, e.g. VVVV)
		}
	}

	//----------
	const NodeSettings & Node::getSettings() {
		return this->settings;
	}

	//----------
	Devices::Set & Node::getDevices() {
		return this->devices;
	}

	//----------
	Modules::Set & Node::getModules() {
		return this->modules;
	}

	//----------
	Json::Value Node::serialise() const {
		Json::Value json;

		json["timestamp"] = ofGetElapsedTimeMillis();
		json["localIndex"] = this->settings.localIndex;

		//iterate through modules and get a data payload from each
		auto & jsonModules = json["modules"];
		int moduleIndex = 0;
		for(auto module : this->modules) {
			auto & jsonModule = jsonModules[moduleIndex++];
			jsonModule["type"] = module->getType();
			jsonModule["data"] = module->serialize();
		}

		return json;
	}

	//----------
	Json::Value Node::getStatus() {
		Json::Value status;
		status["fps"] = ofGetFrameRate();
		status["localIndex"] = this->settings.localIndex;
		status["listeningPort"] = this->server.getPort();
		status["connectionCount"] = this->server.getNumClients();

		auto & devices = status["devices"];
		int iDevice = 0;
		for(auto deviceObject : this->devices) {
			auto & deviceJson = devices[iDevice++];
			deviceJson["type"] = deviceObject->getType();
			deviceJson["status"] = deviceObject->getStatus();
		}

		auto & modules = status["modules"];
		int iModule = 0;
		for(auto moduleObject : this->modules) {
			auto & moduleJson = modules[iModule++];
			moduleJson["type"] = moduleObject->getType();
			moduleJson["status"] = moduleObject->getStatus();
		}

		return status;
	}

	//----------
	string Node::getStatusString() {
		Json::StyledWriter writer;
		return "status = " + writer.write(this->getStatus());
	}
}