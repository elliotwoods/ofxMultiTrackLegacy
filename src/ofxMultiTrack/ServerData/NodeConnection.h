#pragma once

#include "ofMain.h"
#include "ofxNetwork/src/ofxNetwork.h"

#include "../Utils/Constants.h"
#include "../Utils/Types.h"
#include "../Modules/Set.h"
#include "../Modules/Skeleton.h"
#include "Recording.h"

namespace ofxMultiTrack {
	namespace ServerData {
		class NodeConnection : public ofThread {
		public:
			NodeConnection(string address, int index);
			~NodeConnection();
			void update();
			bool isConnected();
			int getUserCount();
			void clearUsers();

			UserSet getLiveData();
			Recording & getRecording();

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

			Json::Value remoteStatus;
			ofMutex remoteStatusLock;

			bool cachedConnected;
			int cachedSkeletonCount;
			bool threadEnded;

			UserSet users;
			ofMutex lockUsers;

			Recording recording;
		};
	}
}