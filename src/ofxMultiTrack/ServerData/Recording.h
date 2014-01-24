#pragma once

#include "ofMain.h"
#include "ofxNetwork/src/ofxNetwork.h"

#include "../Utils/Constants.h"
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

			FrameSet & getFrames();
		protected:
			static UserSet blankFrame;
			FrameSet frames; //frames available on main thread as recording
			FrameSet incomingFrames; //frames coming in on network thread
			ofMutex incomingFramesLock;
		};
	}
}