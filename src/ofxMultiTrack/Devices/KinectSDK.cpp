#include "KinectSDK.h"

#ifdef TARGET_WIN32

namespace ofxMultiTrack {
	namespace Devices {
		//---------
		KinectSDK::KinectSDK() {
			this->deviceID = 0;
			this->sensor = 0;
		}

		//---------
		KinectSDK::KinectSDK(int deviceID) {
			this->deviceID = deviceID;
		}

		//---------
		string KinectSDK::getType() const {
			return "KinectSDK";
		}

		//---------
		void KinectSDK::init() {
			if (!this->kinect.initSensor(this->deviceID)) {
				string error = "Cannot initialise Kinect sensor ID " + ofToString(this->deviceID);
				throw(Exception(error));
			}
			if (!kinect.initColorStream(640, 480, false)) {
				string error = "Cannot init Kinect color stream ID " + ofToString(this->deviceID);
				throw(Exception(error));
			}
			if (!kinect.initDepthStream(640, 480, true)) {
				string error = "Cannot init Kinect depth stream ID " + ofToString(this->deviceID);
				throw(Exception(error));
			}
			if (!kinect.initSkeletonStream(false)) {
				string error = "Cannot init Kinect skeleton stream ID " + ofToString(this->deviceID);
				throw(Exception(error));
			}
			if (!this->kinect.start()) {
				string error = "Cannot start Kinect sensor ID " + ofToString(this->deviceID);
				throw(Exception(error));
			}

			this->sensor = & kinect.getNuiSensor();
		}

		//---------
		void KinectSDK::update() {
			this->kinect.update();
		}

		//---------
		bool KinectSDK::isFrameNew() {
			return this->kinect.isFrameNew();
		}

		//---------
		Json::Value KinectSDK::getStatus() {
			Json::Value status;
			status["Device ID"] = ofToString(deviceID);
			status["isFrameNew"] = this->kinect.isFrameNew();
			return status;
		}

		//---------
		void KinectSDK::setElevation(float angle) {
			if (!this->sensor) {
				ofLogError("ofxMultiTrack::Devices::Kinect") << "Cannot set elevation, no device present";
			}
			long longAngle = ofClamp(angle, NUI_CAMERA_ELEVATION_MINIMUM, NUI_CAMERA_ELEVATION_MAXIMUM);
			bool waitAndTry = true;
			int tries = 0;
			while (waitAndTry) {
				if (tries > 10) {
					break;
				}
				const auto result = this->sensor->NuiCameraElevationSetAngle(longAngle);
				if (result == ERROR_RETRY || result == ERROR_TOO_MANY_CMDS) {
					ofSleepMillis(100);
					tries++;
				} else {
					waitAndTry = false;
				}
			}
		}

		//---------
		ofxKinectCommonBridge& KinectSDK::getDevice() {
			return this->kinect;
		}

		//---------
		void KinectSDK::setConfig(const Json::Value & json) {
			const auto parameterNames = json.getMemberNames();
			for(auto parameterName : parameterNames) {
				if(parameterName == "tilt") {
					this->setElevation(json[parameterName].asFloat());
				}
			}
		}
	}
}

#endif