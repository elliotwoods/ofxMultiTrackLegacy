#pragma once

#include "ofMain.h"
#include "ofxNetwork/src/ofxNetwork.h"

#include "Utils/Constants.h"
#include "Utils/Types.h"
#include "Modules/Set.h"
#include "Modules/Skeleton.h"

#include "ServerData/Recording.h"
#include "ServerData/NodeConnection.h"
#include "ServerData/NodeSet.h"
#include "ServerData/Recorder.h"
#include "ServerData/User.h"

#include "Align/Base.h"
#include "Align/PolyFit.h"
#include "Align/Default.h" //must be included after other routines

namespace ofxMultiTrack {
	class Server {
	public:
		struct OutputFrame {
			vector<ServerData::UserSet> views;
			vector<ServerData::UserSet> world;
			ServerData::CombinedUserSet combined;
			bool calibrated;

			void draw();
		};

		Server();
		~Server();

		bool init();
		void update();

		void addNode(string address, int index);
		void clearNodes();
		
		const ServerData::NodeSet & getNodes() const;
		ServerData::Recorder & getRecorder();

		OutputFrame getCurrentFrame();
		void drawWorld();

		Json::Value getStatus();
		string getStatusString();

		void addAlignment(int nodeIndex, int originNodeIndex, int userIndex = 0, int originUserIndex = 0,
			Align::Ptr routine = Align::Ptr(new Align::Default()));
	protected:
		ServerData::NodeSet nodes;
		ServerData::Recorder recorder;
	};
}