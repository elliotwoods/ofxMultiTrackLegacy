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
			const UserSet & getFrame(Timestamp);
			void clear();

			FrameSet & getFrames();
		protected:
			static UserSet blankFrame;
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
			
			Json::Value serialise() const;
			void deserialise(const Json::Value &);

			void save(string filename="") const; ///<empty filename means ask user
			void load(string filename=""); ///<empty filename means ask user

			bool isWaiting() { return this->state == Waiting; }
			bool isRecording() { return this->state == Recording; }
			bool isPlaying() { return this->state == Playing; }
			void clear();

			Timestamp getPlayHead() const;
			void setPlayHead(Timestamp);
			float getPlayHeadNormalised() const;
			void setPlayHeadNormalised(float);
			Timestamp getStartTime() const;
			Timestamp getEndTime() const;
			Timestamp getDuration() const;
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

		vector<UserSet> getCurrentFrame();
		void drawWorld();

		Json::Value getStatus();
		string getStatusString();
	protected:
		NodeSet nodes;
		Recorder recorder;
	};
}