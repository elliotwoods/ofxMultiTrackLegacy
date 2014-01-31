#pragma once

#include "ofMain.h"
#include "ofxNetwork/src/ofxNetwork.h"

#include "../Utils/Constants.h"
#include "../Utils/Types.h"
#include "../Modules/Set.h"
#include "../Modules/Skeleton.h"
#include "Recording.h"
#include "../Align/Base.h"

namespace ofxMultiTrack {
	namespace ServerData {
		class NodeConnection : public ofThread {
		public:
			struct Transform {
				Transform(unsigned int source, Align::Ptr transform);
				const unsigned int source;
				const Align::Ptr transform;
			};

			typedef vector<shared_ptr<NodeConnection>> Collection;

			NodeConnection(string address, int index, Collection &otherNodes);
			~NodeConnection();
			void update();
			bool isConnected();
			int getUserCount();
			void clearUsers();

			UserSet getLiveData();
			Recording & getRecording();

			Json::Value getStatus();

			void setTransform(shared_ptr<Transform>);
			void applyTransform(UserSet & users) const;
			ofVec3f applyTransform(const ofVec3f &) const;
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
			shared_ptr<Transform> transform;

			Collection & otherNodes;
		};
	}
}