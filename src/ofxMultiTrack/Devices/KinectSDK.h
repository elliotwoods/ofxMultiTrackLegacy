#pragma once

#include "Base.h"
#include "ofxKinectCommonBridge/src/ofxKinectCommonBridge.h"

namespace ofxMultiTrack {
	namespace Devices {
		class KinectSDK : public Base {
		public:
			KinectSDK(int deviceID);
			string getType() override;
			void init() override;
			void update() override;

			ofxKinectCommonBridge& getDevice();
		protected:
			int deviceID;
			ofxKinectCommonBridge kinect;
		};
	}
}