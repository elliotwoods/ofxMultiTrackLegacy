#pragma once

#include "Base.h"
#include "../Devices/KinectSDK.h"

namespace ofxMultiTrack {
	namespace Modules {
		class Skeleton : public Base {
		public:
			string getType() const override;
			Skeleton() { } ///< use this constructor for ofxMultiTrack::Server
			Skeleton(Devices::KinectSDK* kinect); ///< use this constructor for ofxMultiTrack::Node
			void init() override;
			void update() override;
			Json::Value serialize() override;
			void deserialize(const Json::Value& data) override;
			string getStatus() override;

			static string indexToName(int index);
			static _NUI_SKELETON_POSITION_INDEX indexToEnum(int index);
			static Json::Value serialize(KinectCommonBridge::Skeleton&);
		protected:
			ofxKinectCommonBridge* kinect;
			vector<KinectCommonBridge::Skeleton> skeletons;
		};
	}
}