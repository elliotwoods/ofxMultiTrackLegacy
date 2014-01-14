#include "Skeleton.h"

namespace ofxMultiTrack {
	namespace Modules {
		//---------
		string Skeleton::getType() {
			return "Skeleton";
		}

		//---------
		Skeleton::Skeleton(Devices::KinectSDK* kinect) {
			this->kinect = &kinect->getDevice();
		}

		//---------
		void Skeleton::init() {
		}

		//---------
		void Skeleton::update() {
			if (this->kinect->isNewSkeleton()) {
				auto skeletons = this->kinect->getSkeletons();
				for(auto & skeleton : skeletons) {
					for(auto & bone : skeleton) {
						cout << bone.second.getStartPosition() << endl;
					}
				}
			}
		}

		//---------
		void Skeleton::serialize(ofBuffer& data) {
		}

		//---------
		void Skeleton::deserialize(const ofBuffer& data) {
		}
	}
}