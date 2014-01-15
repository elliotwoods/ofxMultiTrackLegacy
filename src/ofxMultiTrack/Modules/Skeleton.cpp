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
				this->skeletons = this->kinect->getSkeletons();
			}
		}

		//---------
		Json::Value Skeleton::serialize() {
			Json::Value json;
			auto & users = json["users"];
			int index = 0;
			for(auto & skeleton : this->skeletons) {
				auto & user = users[index];
				user = serialize(skeleton);
				index++;
			}
			return json;
		}

		//---------
		void Skeleton::deserialize(const Json::Value& data) {
		}

		//---------
		string Skeleton::getStatus() {
			return "Skeletons : " + ofToString(this->skeletons.size());
		}

		//---------
		string Skeleton::indexToName(int index) {
			switch(index) {
			case 0:	return "HIP_CENTER";
			case 1:	return "SPINE";
			case 2: return "SHOULDER_CENTER";
			case 3: return "HEAD";
			case 4: return "SHOULDER_LEFT";
			case 5: return "ELBOW_LEFT";
			case 6: return "WRIST_LEFT";
			case 7: return "HAND_LEFT";
			case 8: return "SHOULDER_RIGHT";
			case 9: return "ELBOW_RIGHT";
			case 10: return "WRIST_RIGHT";
			case 11: return "HAND_RIGHT";
			case 12: return "HIP_LEFT";
			case 13: return "KNEE_LEFT";
			case 14: return "ANKLE_LEFT";
			case 15: return "FOOT_LEFT";
			case 16: return "HIP_RIGHT";
			case 17: return "KNEE_RIGHT";
			case 18: return "ANKLE_RIGHT";
			case 19: return "FOOT_RIGHT";
			}
			return "error: joint index out of range";
		}

		//---------
		_NUI_SKELETON_POSITION_INDEX Skeleton::indexToEnum(int index) {
			return (_NUI_SKELETON_POSITION_INDEX) index;
		}

		//---------
		Json::Value Skeleton::serialize(KinectCommonBridge::Skeleton& skeleton) {
			Json::Value json;
			for(int i=0; i<skeleton.size(); i++) {
				auto & jointData = skeleton.at(indexToEnum(i));
				auto & jointJson = json[indexToName(i)];

				auto positionVector = jointData.getStartPosition();
				auto & positionJson = jointJson["position"];
				for(int j=0; j<3; j++) {
					positionJson[j] = positionVector[j];
				}

				auto rotationQuaternion = jointData.getRotation();
				auto & rotationJson = jointJson["rotation"];
				for(int j=0; j<4; j++) {
					rotationJson[j] = rotationQuaternion[j];
				}
			}
			return json;
		}
	}
}