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
				checkExisting.setup("127.0.0.1", OFXMULTITRACK_CLIENT_LISTEN_PORT + localNodeIndex, true);
				if (checkExisting.isConnected()) {
					localNodeIndex++;
				} else {
					stillLooking = false;
				}
				if (localNodeIndex >= OFXMULTITRACK_CLIENT_COUNT_MAX) {
					throw(std::exception("Too many clients already running on this machine"));
				}
			}
			ofLogNotice("ofxMultiTrack") << "Local Node index is  " << localNodeIndex;

			//open a server on correct port
			this->server.setup(OFXMULTITRACK_CLIENT_LISTEN_PORT + localNodeIndex, false);
			ofLogNotice("ofxMultiTrack") << "Node is listening on port " << this->server.getPort();

			//add devices
			auto kinect = new Devices::KinectSDK(this->settings.localIndex);
			this->devices.add(kinect);

			//initialise devices
			for(auto device : this->devices) {
				device->init();
			}

			//add modules
			this->modules.add(new Modules::Skeleton(kinect));

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

		//initialise json parcel
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
		Json::StyledWriter writer;
		string result = writer.write(json);

		//send payload to all clients
		server.sendToAll(result); //always sends a [/TCP] from oF :(
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
	string Node::getStatus() {
		stringstream status;

		status << "Local index:\t" << this->settings.localIndex << endl;
		status << "Listening port:\t" << this->server.getPort() << endl;
		status << "Connections:\t" << this->server.getNumClients() << endl;
		status << endl;

		status << "Fps:\t" << ofGetFrameRate() << endl;
		status << endl;

		status << "Devices:" << endl;
		for(auto device : this->devices) {
			status << "\t" << device->getType() << "\t: " << device->getStatus() << endl;
		}
		status << endl;

		status << "Modules:" << endl;
		for(auto module : this->modules) {
			status << "\t" << module->getType() << "\t: " << module->getStatus() << endl;
		}
		status << endl;

		return status.str();
	}
}