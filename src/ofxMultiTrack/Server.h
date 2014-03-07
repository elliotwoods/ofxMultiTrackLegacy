#pragma once

#include "ofMain.h"
#include "ofxNetwork/src/ofxNetwork.h"
#include "ofxTSP/src/ofxTSP.h"
#include "ofxAssets/src/ofxAssets.h"

#include "Utils/Constants.h"
#include "Utils/Types.h"
#include "Modules/Set.h"
#include "Modules/Skeleton.h"

#include "ServerData/Recording.h"
#include "ServerData/NodeConnection.h"
#include "ServerData/NodeSet.h"
#include "ServerData/Recorder.h"
#include "ServerData/User.h"
#include "ServerData/OutputFrame.h"

#include "Align/Factory.h"

namespace ofxMultiTrack {
	class Server {
	public:
		Server();
		~Server();

		bool init();
		void update();

		void addNode(string address, int index);
		void clearNodes();
		
		/// Clear all current incoming users from all nodes
		void clearNodeUsers();

		const ServerData::NodeSet & getNodes() const;
		ServerData::Recorder & getRecorder();

		ServerData::OutputFrame getCurrentFrame() const;

		/// Draw untransformed view
		void drawViews() const;
		void drawWorld() const;

		void drawViewConesWorld() const;
		void drawViewConeView() const;

		Json::Value getStatus();
		string getStatusString();

		void autoCalibrate();
		void addAlignment(int nodeIndex, int originNodeIndex, int userIndex = 0, int originUserIndex = 0,
			Align::Ptr routine = Align::Factory::makeDefault());
		
		/// Call this when you are standing in the world origin pose to calibrate any root nodes
		void applyOriginPose();

		void serialise(Json::Value &) const;
		void deserialise(const Json::Value &);

		void saveCalibration(string filename = "") const;
		void loadCalibration(string filenmae = "");

		void addNodeInitialiseMessage(const Json::Value &) const;
	protected:
		ServerData::NodeSet nodes;
		ServerData::Recorder recorder;
		ServerData::OutputFrame currentFrame;
		ServerData::OutputFrame previousFrame;
	};
}