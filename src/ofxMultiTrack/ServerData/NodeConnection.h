#pragma once

#include "ofMain.h"
#include "ofxNetwork/src/ofxNetwork.h"

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
			unsigned int getLastRxInterval() const;
			long long getAbsoluteTimeOffset() const;
			int getIndex() const; /// < Make sure not to call this from constructor (or before we're in the collection)

			const Transform & getTransform() const;
			void setTransform(const Transform &);
			void clearTransform();
			void applyTransform(UserSet & users) const;
			ofVec3f applyTransform(const ofVec3f &) const;
			void applyOriginPose(const User &);

			list<int> getInfluenceList(int circularInfluenceCheck = -2) const;
			bool isRoot() const;

			bool isEnabled() const;
			void setEnabled(bool);
			void toggleEnabled();

			void addNodeConfig(const Json::Value &);
			void send(const Json::Value &);

			void saveConfig(string filename = "") const;
			void loadConfig(string filename = "");
			string getDefaultConfigFilename() const;
			
			string getName() const;
			void setName(string);

			ofParameter<float> & getTiltParameter();
		protected:
			void threadedFunction() override;
			void receiveMessages();
			void sendMessages();

			void performBlocking(function<void()>);
			void onTiltParameterChange(float &);

			deque<function<void()>> actionQueue; // a list of actions to be performed on the network thread
			ofMutex lockActionQueue;

			Json::Reader jsonReader;
			ofxTCPClient client;
			string address;
			int remoteIndex;
			string name;
			bool running;
			unsigned long long lastRxTime;
			unsigned int lastRxInterval;
			long long absoluteTimeOffset;

			Json::Value remoteConfig;
			Json::Value toSend;
			ofMutex toSendMutex;

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

			ofxTCPClient meshClient;

			bool disableSaving;
			ofParameter<float> tiltParameter;

			bool enabled; ///<denotes whether we will supply data
		};
	}
}