#pragma once

#include "Base.h"
#include "../Devices/KinectSDK.h"
#include "../../../addons/ofxNetwork/src/ofxNetwork.h"

namespace ofxMultiTrack {
	namespace Modules {
		class PreviewRGB : Base {
		public:
			string getType() const override;
			PreviewRGB() { };
			PreviewRGB(shared_ptr<Devices::KinectSDK> kinect); ///< use this constructor for ofxMultiTrack::Node
			void init() override;
			void update() override;
			Json::Value serialize() override;
			void deserialize(const Json::Value& data) override;
			Json::Value getStatus() override;
			void drawOnDepth();
		protected:
			shared_ptr<Devices::KinectSDK> kinect;
			ofBuffer buffer;
			ofxTCPServer network;
		};
	}
}