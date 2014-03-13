#include "Recording.h"
#include "../Utils/Constants.h"

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
				if (abs(foundFrame->first - timestamp) <= OFXMULTITRACK_SERVER_RECORDING_MAX_FRAME_DURATION) {
					return foundFrame->second;
				} else {
					return blankFrame;
				}
			}
		}

		//----------
		void Recording::clear() {
			this->frames.clear();
			this->clearIncoming();
		}

		//----------
		int Recording::getLikelyCalibrationUserIndex() const {
			//collate calibration joint paths
			map<int, vector<ofVec3f> > jointPathPerUser;
			for(const auto & frame : this->frames) {
				for(int userIndex = 0; userIndex < frame.second.size(); userIndex++) {
					const auto & user = frame.second[userIndex];
					if (user.find(OFXMULTITRACK_SERVER_ALIGN_REFERENCE_JOINT) == user.end()) {
						continue;
					}
					jointPathPerUser[userIndex].push_back(user.at(OFXMULTITRACK_SERVER_ALIGN_REFERENCE_JOINT).position);
				}
			}

			//calculate length of movement per joint
			map<int, float> pathLengthPerUser;
			for(const auto & path : jointPathPerUser) {
				if (path.second.empty()) {
					continue;
				}

				float length = 0;
				auto lastPosition = path.second.front();
				for(auto point : path.second) {
					length += (point - lastPosition).length();
					lastPosition = point;
				}
				pathLengthPerUser[path.first] = length;
			}

			//find maximum path length
			float maxLength = 0;
			int maxUserIndex = -1;
			for(auto length : pathLengthPerUser) {
				if (length.second > maxLength) {
					maxUserIndex = length.first;
					maxLength = length.second;
				}
			}

			if (maxUserIndex == -1) {
				throw(ofxMultiTrack::Exception("No users found in recording"));
			} else {
				return maxUserIndex;
			}
		}

		//----------
		Recording::FrameSet & Recording::getFrames() {
			return this->frames;
		}

		//----------
		UserSet Recording::blankFrame = UserSet();
	}
}