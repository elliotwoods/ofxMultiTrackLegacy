#include "MatrixTransform.h"
#include "../Utils/Types.h"

namespace ofxMultiTrack {
	namespace Align {
		//----------
		string MatrixTransform::getType() const {
			return "MatrixTransform";
		}

		//----------
		void MatrixTransform::calibrate(const vector<ofVec3f> & thisSpace, const vector<ofVec3f> & originSpace) {
			throw(Exception("MatrixTransform cannot be calibrated from control points. You shouldn't even try."));
		}

		//----------
		ofVec3f MatrixTransform::applyTransform(const ofVec3f & x) const {
			return x * transform;
		}

		//----------
		Json::Value MatrixTransform::serialise() const {
			Json::Value json;
			json["ready"] = true;
			auto & jsonTransform = json["transform"];
			for(int i=0; i<4; i++) {
				for(int j=0; j<4; j++) {
					jsonTransform[i][j] = this->transform(i, j);
				}
			}
			return json;
		}

		//----------
		void MatrixTransform::deserialise(const Json::Value & json) {
			if (json["ready"].asBool()) {
				const auto & jsonTransform = json["transform"];
				for(int i=0; i<4; i++) {
					for(int j=0; j<4; j++) {
						this->transform(i, j) = jsonTransform[i][j].asFloat();
					}
				}
			}
		}

		//----------
		void MatrixTransform::setTransform(const ofMatrix4x4 & transform) {
			this->transform = transform;
		}

		//----------
		const ofMatrix4x4 MatrixTransform::getMatrixTransform() const {
			return this->transform;
		}
	}
}