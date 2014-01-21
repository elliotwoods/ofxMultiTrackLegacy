#pragma once

#include "ofMain.h"
#include "ofxNetwork/src/ofxNetwork.h"

#include "../Utils/Constants.h"
#include "../Utils/Types.h"
#include "../Modules/Set.h"
#include "../Modules/Skeleton.h"
#include "NodeConnection.h"
#include "NodeSet.h"
#include "Recording.h"

namespace ofxMultiTrack {
	namespace ServerData {
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
			State getState() { return this->state; }
			void clear();

			bool hasData() const; ///<currently we just lazily check the record duration
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
	}
}