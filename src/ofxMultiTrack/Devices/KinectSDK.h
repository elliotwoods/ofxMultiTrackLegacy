#pragma once

#include "ofxKinectCommonBridge/src/ofxKinectCommonBridge.h"
#include "../Utils/Types.h"

#include "Base.h"

namespace ofxMultiTrack {
	namespace KinectCommonBridge {
		typedef Skeleton Skeleton;
		typedef SkeletonBone SkeletonBone;
	}

	namespace Devices {
		class KinectSDK : public Base {
		public:
			KinectSDK();
			KinectSDK(int deviceID);
			string getType() const override;
			void init() override;
			void update() override;
			bool isFrameNew() override;
			Json::Value getStatus() override;

			ofxKinectCommonBridge& getDevice();
		protected:
			int deviceID;
			ofxKinectCommonBridge kinect;
		};
	}
}