#pragma once

#include "Base.h"
#include "../Devices/KinectSDK.h"
#include "../Utils/Types.h"
#include "../Utils/Constants.h"

namespace ofxMultiTrack {
	namespace Modules {
		class Skeleton : public Base {
		public:
			string getType() const override;
			Skeleton() { } ///< use this constructor for ofxMultiTrack::Server
			Skeleton(shared_ptr<Devices::KinectSDK> kinect); ///< use this constructor for ofxMultiTrack::Node
			void init() override;
			void update() override;
			Json::Value serialize() override;
			void deserialize(const Json::Value& data) override;
			float getFrameRate();
			Timestamp getLastFrameAge();
			Json::Value getStatus() override;

			static string indexToName(int index);
			static _NUI_SKELETON_POSITION_INDEX indexToEnum(int index);
			static Json::Value serialize(KinectCommonBridge::Skeleton&);

			void drawOnDepth();
		protected:
			ofxKinectCommonBridge * kinect;
			vector<KinectCommonBridge::Skeleton> skeletons;
			bool isNewSkeleton;
			bool isNewFrame;
			bool needsSendSkeleton;
			deque<Timestamp> frameTimings;
		};
	}
}