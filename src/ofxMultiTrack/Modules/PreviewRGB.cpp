#include "PreviewRGB.h"

namespace ofxMultiTrack {
	namespace Modules {
		//----------
		string PreviewRGB::getType() const {
			return "PreviewRGB";
		}

		//----------
		PreviewRGB::PreviewRGB(shared_ptr<Devices::KinectSDK> kinect) {
			this->kinect = kinect;
		}
		
		//----------
		void PreviewRGB::init() {
		}

		//----------
		void PreviewRGB::update() {
			ofSaveImage(this->kinect->getDevice().getColorPixelsRef(), this->buffer, OF_IMAGE_FORMAT_JPEG);
		}
	}
}