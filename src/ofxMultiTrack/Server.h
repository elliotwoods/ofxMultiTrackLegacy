#pragma once

#include "ofMain.h"
#include "ofxNetwork/src/ofxNetwork.h"

#include "Utils/Constants.h"
#include "Utils/Types.h"
#include "Modules/Set.h"
#include "Modules/Skeleton.h"

#include "ServerData/Recording.h"
#include "ServerData/NodeConnection.h"
#include "ServerData/Recorder.h"

namespace ofxMultiTrack {
	class Server {
	public:
		Server();
		~Server();

		bool init();
		void update();

		void addNode(string address, int index);
		void clearNodes();
		
		const ServerData::NodeSet & getNodes();
		ServerData::Recorder & getRecorder();

		vector<UserSet> getCurrentFrame();
		void drawWorld();

		Json::Value getStatus();
		string getStatusString();
	protected:
		ServerData::NodeSet nodes;
		ServerData::Recorder recorder;
	};
}