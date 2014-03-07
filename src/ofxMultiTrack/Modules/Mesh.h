#pragma once

#include "Base.h"
#include "../Devices/KinectSDK.h"
#include "../Utils/Types.h"
#include "../Utils/Constants.h"

namespace ofxMultiTrack {
	namespace Modules {
		class Mesh : public Base {
		public:
			string getType() const override;
			Mesh() { };
			Mesh(shared_ptr<Devices::KinectSDK>);
			void init() override;
			void update() override;
			Json::Value serialize() override;
			void deserialize(const Json::Value& data) override;
			Json::Value getStatus() override;
		protected:
			shared_ptr<Devices::KinectSDK> kinect;
		};
	}
}