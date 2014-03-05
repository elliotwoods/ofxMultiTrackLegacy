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

			class Transform {
			public:
				typedef shared_ptr<Transform> Ptr;

				Transform();
				Transform(unsigned int parent, Align::Ptr transform);

				int getParent() const;
				Align::Ptr getTransform() const;
			protected:
				int parent;
				Align::Ptr transform;
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

			const Transform & getTransform() const;
			void setTransform(const Transform &);
			void clearTransform();
			void applyTransform(UserSet & users) const;
			ofVec3f applyTransform(const ofVec3f &) const;
			void applyOriginPose(const User &);

			list<int> getInfluenceList() const;
			bool isRoot() const;

			bool isEnabled() const;
			void setEnabled(bool);
			void toggleEnabled();
		protected:
			void threadedFunction() override;
			void performBlocking(function<void()>);

			deque<function<void()>> actionQueue; // a list of actions to be performed on the network thread
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
			Transform transform;

			Collection & otherNodes;

			bool enabled; ///<denotes whether we will supply data
		};
	}
}