#pragma once

#include "Base.h"
#include "../Devices/KinectSDK.h"

namespace ofxMultiTrack {
	namespace Modules {
		class Skeleton : public Base {
		public:
			string getType() override;
			Skeleton() { } ///< use this constructor for ofxMultiTrack::Server
			Skeleton(Devices::KinectSDK* kinect); ///< use this constructor for ofxMultiTrack::Node
			void init() override;
			void update() override;
			void serialize(ofBuffer& data) override;
			void deserialize(const ofBuffer& data) override;
		protected:
			ofxKinectCommonBridge* kinect;
		};
	}
}