#include "Mesh.h"
#include "ofxCv.h"

namespace ofxMultiTrack {
	namespace Modules {
		//----------
		string Mesh::getType() const {
			return "Mesh";
		}
			
		//----------
		Mesh::Mesh(shared_ptr<Devices::KinectSDK> kinect) {
			this->kinect = kinect;
		}

		//----------
		void Mesh::init() {
		}

		//----------
		void Mesh::update() {
		}
	}
}