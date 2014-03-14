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

			//calculate the bounds of movement per joint
			map<int, ofVec3f> leftBottomFronts;
			map<int, ofVec3f> rightTopBacks;
			for(const auto & path : jointPathPerUser) {
				auto userIndex = path.first;

				if (path.second.empty()) {
					continue;
				}

				float length = 0;
				for(auto point : path.second) {
					if (leftBottomFronts.count(userIndex) == 0) {
						leftBottomFronts[userIndex] = point;
					} else {
						auto & leftBottomFront = leftBottomFronts[userIndex];
						leftBottomFront.x = min(leftBottomFront.x, point.x);
						leftBottomFront.y = min(leftBottomFront.y, point.y);
						leftBottomFront.z = min(leftBottomFront.z, point.z);
					}
					if (rightTopBacks.count(userIndex) == 0) {
						rightTopBacks[userIndex] = point;
					} else {
						auto & rightTopBack = rightTopBacks[userIndex];
						rightTopBack.x = max(rightTopBack.x, point.x);
						rightTopBack.y = max(rightTopBack.y, point.y);
						rightTopBack.z = max(rightTopBack.z, point.z);
					}
				}
			}

			//find maximum path length
			float maxSpread = 0;
			int maxUserIndex = -1;
			for(auto path : jointPathPerUser) {
				const auto userIndex = path.first;
				const auto spread = (leftBottomFronts[userIndex] - rightTopBacks[userIndex]).length();
				if (spread > maxSpread) {
					maxUserIndex = path.first;
					maxSpread = spread;
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