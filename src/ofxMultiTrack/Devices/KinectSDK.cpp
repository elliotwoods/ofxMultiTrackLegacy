#include "KinectSDK.h"

namespace ofxMultiTrack {
	namespace Devices {
		//---------
		KinectSDK::KinectSDK(int deviceID) {
			this->deviceID = deviceID;
		}

		//---------
		string KinectSDK::getType() {
			return "KinectSDK";
		}

		//---------
		void KinectSDK::init() {
			if (!this->kinect.initSensor(this->deviceID)) {
				string error = "Cannot initialise Kinect sensor ID " + ofToString(this->deviceID);
				throw(std::exception(error.c_str()));
			}
			if (!kinect.initColorStream(640, 480, true)) {
				string error = "Cannot init Kinect color stream ID " + ofToString(this->deviceID);
				throw(std::exception(error.c_str()));
			}
			if (!kinect.initDepthStream(640, 480, false)) {
				string error = "Cannot init Kinect color stream ID " + ofToString(this->deviceID);
				throw(std::exception(error.c_str()));
			}
			if (!kinect.initSkeletonStream(false)) {
				string error = "Cannot init Kinect color stream ID " + ofToString(this->deviceID);
				throw(std::exception(error.c_str()));
			}
			if (!this->kinect.start()) {
				string error = "Cannot start Kinect sensor ID " + ofToString(this->deviceID);
				throw(std::exception(error.c_str()));
			}
		}

		//---------
		void KinectSDK::update() {
			this->kinect.update();
		}

		//---------
		string KinectSDK::getStatus() {
			return "Kinect ID#" + ofToString(deviceID) + (this->kinect.isFrameNew() ? " ." : "");
		}

		//---------
		ofxKinectCommonBridge& KinectSDK::getDevice() {
			return this->kinect;
		}
	}
}