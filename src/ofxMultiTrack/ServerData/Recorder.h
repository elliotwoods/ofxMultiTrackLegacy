#pragma once

#include "ofMain.h"
#include "ofxNetwork/src/ofxNetwork.h"

#include "../Utils/Types.h"
#include "../Modules/Set.h"
#include "../Modules/Skeleton.h"
#include "NodeConnection.h"
#include "NodeSet.h"
#include "Recording.h"

namespace ofxMultiTrack {
	namespace ServerData {
		class Recorder : public ISerialisable {
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
			
			void serialise(Json::Value &) const;
			void deserialise(const Json::Value &);

			void save(string filename="") const;
			void load(string filename="");

			bool isWaiting() const { return this->state == Waiting; }
			bool isRecording() const { return this->state == Recording; }
			bool isPlaying() const { return this->state == Playing; }
			State getState() const { return this->state; }
			void clear();

			bool hasData() const; ///<currently we just lazily check the record duration
			Timestamp getPlayHead() const;
			void setPlayHead(Timestamp);
			float getPlayHeadNormalised() const;
			void setPlayHeadNormalised(float);
			Timestamp getStartTime() const;
			Timestamp getEndTime() const;
			Timestamp getDuration() const;

			void clearInOutPoints();
			Timestamp getInPoint() const;
			Timestamp getOutPoint() const;
			void setInPoint(Timestamp);
			void setOutPoint(Timestamp);

			void trim();

			Timestamp appToTimeline(Timestamp) const;
			Timestamp timelineToApp(Timestamp) const;
			float timelineToNormalised(Timestamp) const;
			Timestamp normalisedToTimestamp(float) const;

			static string toString(Timestamp);
		protected:
			State state;
			const NodeSet & nodes;
			Timestamp playHead;
			Timestamp startTime;
			Timestamp endTime;
			Timestamp inPoint;
			Timestamp outPoint;
		};
	}
}