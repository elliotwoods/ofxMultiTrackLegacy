#pragma once

#include "ofMain.h"
#include "ofxNetwork/src/ofxNetwork.h"

#include "../Utils/Types.h"
#include "../Modules/Set.h"
#include "../Modules/Skeleton.h"
#include "User.h"

namespace ofxMultiTrack {
	namespace ServerData {
		class Recording {
		public:
			typedef map<Timestamp, UserSet> FrameSet;
			void recordIncoming();
			void clearIncoming();

			void addIncoming(UserSet);
			const UserSet & getFrame(Timestamp);
			void clear();

			int getLikelyCalibrationUserIndex(string calibrationJoint) const;

			FrameSet & getFrames();
		protected:
			static UserSet blankFrame;
			FrameSet frames; //frames available on main thread as recording
			FrameSet incomingFrames; //frames coming in on network thread
			ofMutex incomingFramesLock;
		};
	}
}