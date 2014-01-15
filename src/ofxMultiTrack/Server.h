#pragma once

#include "ofMain.h"
#include "ofxNetwork/src/ofxNetwork.h"

#include "Utils/Constants.h"
#include "Utils/Types.h"
#include "Modules/Set.h"
#include "Modules/Skeleton.h"

namespace ofxMultiTrack {
	class Server {
		class NodeConnection : public ofThread {
		public:
			NodeConnection(string address, int index);
			~NodeConnection();
			bool isConnected();
			int getUserCount();
			Json::Value getStatus();
		protected:
			void threadedFunction() override;
			void performBlocking(function<void()>);

			deque<function<void()>> actionQueue;
			ofMutex lockActionQueue;

			Json::Reader jsonReader;
			ofxTCPClient client;
			string address;
			int index;
			bool running;

			bool cachedConnected;
			int cachedSkeletonCount;

			vector<Data::User> users;
			ofMutex lockUsers;
		};
	public:
		Server();
		~Server();
		bool init();
		void update();
		void addNode(string address, int index);
		void clearNodes();
		void drawWorld();
		string getStatus();
	protected:
		vector<ofPtr<NodeConnection>> nodes;
	};
}