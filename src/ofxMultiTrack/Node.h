#pragma once

#include "ofMain.h"
#include "ofxNetwork/src/ofxNetwork.h"

#include "Utils/Constants.h"
#include "Utils/Types.h"
#include "Devices/Set.h"
#include "Devices/KinectSDK.h"
#include "Modules/Set.h"
#include "Modules/Skeleton.h"
#include "Modules/Mesh.h"

namespace ofxMultiTrack {
	class Node {
	public:
		Node();
		bool init();
		void update();

		const NodeSettings & getSettings();
		Devices::Set & getDevices();
		Modules::Set & getModules();

		Json::Value serialise() const;

		Json::Value getStatus();
		string getStatusString();

	protected:
		void parseIncoming(const Json::Value &);

		ofxTCPServer server;
		NodeSettings settings;

		Devices::Set devices;
		Modules::Set modules;
	};
}