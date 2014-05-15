#include "Node.h"
#include "Utils/Constants.h"
#include "Utils/Utils.h"
#include <sys/timeb.h>

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
				ofSleepMillis(ofRandomuf() * 10000.0f);
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
			//this->modules.push_back(shared_ptr<Modules::Mesh>(new Modules::Mesh(kinect, localNodeIndex)));

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
		//update devices and block until a device updates or we timeout at 20fps
		auto timeout = 1000 / 20;
		auto startTimeoutTimer = ofGetElapsedTimeMillis();
		bool newFrameFound = false;
		while(!newFrameFound && ofGetElapsedTimeMillis() - startTimeoutTimer <= timeout) {
			for(auto device : this->devices) {
				device->update();
				auto deviceNewFrame = device->isFrameNew();
				newFrameFound |= deviceNewFrame;
			}
			if (!newFrameFound) {
				ofSleepMillis(1);
			}
		}

		//update modules
		for(auto module : this->modules) {
			module->update();
		}

		//only send if data received
		if (newFrameFound) {
			//get local dataset
			auto json = this->serialise();

			//send local status also for remote viewing
			json["status"] = this->getStatus();

			Json::StyledWriter writer;
			string result = writer.write(json);

			//send payload to all clients
			server.sendToAll(result); //always sends a [/TCP] from oF (note for other clients, e.g. VVVV)
		}

		Json::Value received;
		int connectedCount = 0;
		for(int iClient = 0; iClient < this->server.getNumClients(); iClient++) {
			if (!this->server.isClientConnected(iClient)) {
				continue;
			} else {
				connectedCount++;
			}
			Json::Reader reader;
			auto rxString = server.receive(iClient);
			if (!rxString.empty()) {
				Json::Value serverJson;
				reader.parse(rxString, serverJson);
				for(const auto & message : serverJson) {
					received[received.size()] = message;
				}
			}
		}
		if (connectedCount < this->server.getNumClients()) {
			//strange hack where ofxNetwork reports a client as being present but not connected
			//sendToAll still works, but receive won't work in this case.
			//So restart our server in this case
			ofLogNotice("ofxMultiTrack") << "Restarting node's network connection." << endl;
			auto port = this->server.getPort();
			this->server.close();
			this->server.setup(port, false);
		}

		if (!received.empty()) {
			this->parseIncoming(received);
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

		//iterate through modules and get a data payload from each
		auto & jsonModules = json["modules"];
		int moduleIndex = 0;
		for(auto module : this->modules) {
			auto & jsonModule = jsonModules[module->getType()];
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
		status["timestamp"] = ofGetElapsedTimeMillis();
		status["absoluteTime"] = Utils::getAbsoluteTime();

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

	//----------
	void Node::parseIncoming(const Json::Value & json) {
		cout << "incoming = " << json << endl << endl;

		for(auto message : json) {
			if (message.isObject()) {
				const auto categories = message.getMemberNames();
				for(auto category : categories) {
					if (category == "devices") {
						this->devices.setConfig(message[category]);
					} else if (category == "modules") {
						this->modules.setConfig(message[category]);
					}
				}
			}
		}
	}
}