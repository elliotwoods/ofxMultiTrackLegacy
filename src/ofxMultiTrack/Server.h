#pragma once

#include "ofMain.h"
#include "ofxNetwork/src/ofxNetwork.h"

#include "Utils/Constants.h"
#include "Utils/Types.h"
#include "Modules/Set.h"
#include "Modules/Skeleton.h"

namespace ofxMultiTrack {
	class Server {
	public:
		class Recording {
		public:
			typedef map<Timestamp, UserSet> FrameSet;
			void recordIncoming();
			void clearIncoming();

			void addIncoming(UserSet);
			void clear();

			FrameSet & getFrames();
		protected:
			FrameSet frames; //frames available on main thread as recording
			FrameSet incomingFrames; //frames coming in on network thread
			ofMutex incomingFramesLock;
		};

		class NodeConnection : public ofThread {
		public:
			NodeConnection(string address, int index);
			~NodeConnection();
			void update();
			bool isConnected();
			int getUserCount();

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

			bool cachedConnected;
			int cachedSkeletonCount;
			bool threadEnded;

			UserSet users;
			ofMutex lockUsers;

			Recording recording;
		};

		typedef vector<shared_ptr<NodeConnection>> NodeSet;
		
		class Recorder {
		public:
			enum State {
				Waiting,
				Recording,
				Playing
			};
			Recorder(const NodeSet &);
			
			void update();

			void record();
			void play();
			void stop();
			
			bool isRecording() { return this->state == Recording; }
			bool isPlaying() { return this->state == Playing; }
			void clear();

			Timestamp getPlayHead();
			Timestamp getStartTime();
			Timestamp getEndTime();
		protected:
			State state;
			const NodeSet & nodes;
			Timestamp playHead;
			Timestamp startTime;
			Timestamp endTime;
		};

		Server();
		~Server();

		bool init();
		void update();

		void addNode(string address, int index);
		void clearNodes();
		
		const NodeSet & getNodes();
		Recorder & getRecorder();

		void drawWorld();
		string getStatus();
	protected:
		NodeSet nodes;
		Recorder recorder;
	};
}