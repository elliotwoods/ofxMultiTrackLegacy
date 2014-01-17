#include "Recording.h"

namespace ofxMultiTrack {
	namespace ServerData {
		//----------
		void Recording::recordIncoming() {
			this->incomingFramesLock.lock();
			for(auto frame : this->incomingFrames) {
				this->frames.insert(frame);
			}
			this->incomingFrames.clear();
			this->incomingFramesLock.unlock();
		}

		//----------
		void Recording::clearIncoming() {
			this->incomingFramesLock.lock();
			this->incomingFrames.clear();
			this->incomingFramesLock.unlock();
		}

		//----------
		void Recording::addIncoming(UserSet userSet) {
			this->incomingFramesLock.lock();
			this->incomingFrames.insert(std::pair<Timestamp, UserSet>(ofGetElapsedTimeMicros(), userSet));
			this->incomingFramesLock.unlock();
		}

		//----------
		const UserSet & Recording::getFrame(Timestamp timestamp) {
			if (this->frames.empty()) {
				return blankFrame;
			} else {
				auto foundFrame = this->frames.lower_bound(timestamp);
				if (foundFrame == this->frames.end()) {
					foundFrame = this->frames.end();
					foundFrame--;
				}
				return foundFrame->second;
			}
		}

		//----------
		void Recording::clear() {
			this->frames.clear();
			this->clearIncoming();
		}

		//----------
		Recording::FrameSet & Recording::getFrames() {
			return this->frames;
		}

		//----------
		UserSet Recording::blankFrame = UserSet();
	}
}