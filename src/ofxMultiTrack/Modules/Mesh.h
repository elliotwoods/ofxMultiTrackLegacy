#pragma once

#include "Base.h"
#include "../Devices/KinectSDK.h"
#include "../Utils/Types.h"
#include "ofxOsc/src/ofxOsc.h"
#include "ofxNetwork/src/ofxNetwork.h"

namespace ofxMultiTrack {
	namespace Modules {
		class Mesh : public Base {
		public:
			string getType() const override;
			Mesh() { };
			Mesh(shared_ptr<Devices::KinectSDK>, int globalIndex);
			void init() override;
			void update() override;
			Json::Value serialize() override;
			void deserialize(const Json::Value& data) override;
			Json::Value getStatus() override;
			void setConfig(const Json::Value &) override;
			void drawWorld();
		protected:
			void setTarget(const string address, int port);
			void sendBinary();

			shared_ptr<Devices::KinectSDK> kinect;
			INuiSensor * sensor;

			vector<float> contourAreas;
			vector<ofMesh> worldContours;
			ofImage preview;
			
			ofxOscSender sender;
			bool isOscTargetSet;
			int globalIndex;

			shared_ptr<UdpTransmitSocket> udpSender;
			ofBuffer sendBuffer;
			ofMatrix4x4 transform;
			Json::Value status;
		};
	}
}