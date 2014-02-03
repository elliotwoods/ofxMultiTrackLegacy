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
			typedef shared_ptr<NodeConnection> Ptr;
			typedef vector<shared_ptr<NodeConnection>> Collection;

			struct Transform {
				typedef shared_ptr<Transform> Ptr;

				Transform(unsigned int parent, Align::Ptr transform);
				const unsigned int parent;
				const Align::Ptr transform;
			};

			NodeConnection(string address, int remoteIndex, Collection &otherNodes);
			~NodeConnection();
			void update();
			bool isConnected();
			int getUserCount();
			void clearUsers();

			UserSet getLiveData();
			Recording & getRecording();

			Json::Value getStatus();
			int getIndex() const;

			Transform::Ptr getTransform() const;
			void setTransform(shared_ptr<Transform>);
			list<int> getInfluenceList() const;
			void applyTransform(UserSet & users) const;
			ofVec3f applyTransform(const ofVec3f &) const;

			bool isEnabled() const;
			void setEnabled(bool);
			void toggleEnabled();
		protected:
			void threadedFunction() override;
			void performBlocking(function<void()>);

			deque<function<void()>> actionQueue;
			ofMutex lockActionQueue;

			Json::Reader jsonReader;
			ofxTCPClient client;
			string address;
			int remoteIndex;
			bool running;

			Json::Value remoteStatus;
			ofMutex remoteStatusLock;

			bool cachedConnected;
			int cachedSkeletonCount;
			bool threadEnded;

			UserSet users;
			ofMutex lockUsers;

			Recording recording;
			Transform::Ptr transform;

			Collection & otherNodes;

			bool enabled; ///<denotes whether we will supply data
		};
	}
}