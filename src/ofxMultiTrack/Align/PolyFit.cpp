#include "PolyFit.h"

namespace ofxMultiTrack {
	namespace Align {
		//----------
		string PolyFit::getType() const {
			return "PolyFit";
		}

		//----------
		PolyFit::PolyFit() {
			this->fit.init(1, 3, 3, BASIS_SHAPE_TRIANGLE);
			this->transformPoint = new pfitDataPointf(3, 3);
		}

		//----------
		PolyFit::~PolyFit() {
			delete this->transformPoint;
		}

		//----------
		void PolyFit::calibrate(const vector<ofVec3f> & thisSpace, const vector<ofVec3f> & originSpace) {
			auto dataSet = ofxPolyFit::makeDataSet(thisSpace, originSpace);
			//this->fit.correlate(dataSet);
			this->fit.RANSAC(dataSet, 100, 0.2f, 0.1f, 0.7f);
			cout << "Residual : " << this->fit.residualRMS(dataSet) << "m" << endl;
		}

		//----------
		ofVec3f PolyFit::applyTransform(const ofVec3f & thisSpace) const {
			* ( (ofVec3f*) this->transformPoint->getInput() ) = thisSpace;
			this->fit.evaluate(*this->transformPoint);
			return * (ofVec3f*) this->transformPoint->getOutput();
		}

		//----------
		Json::Value PolyFit::serialise() const {
			Json::Value json;
			for(int i=0; i<3; i++) {
				for(int j=0; j<this->fit.nBases; j++) {
					json[i][j] = this->fit.coefficients[i][j];
				}
			}
			return json;
		}

		//----------
		void PolyFit::deserialise(const Json::Value & json) {
			for(int i=0; i<3; i++) {
				for(int j=0; j<this->fit.nBases; j++) {
					this->fit.coefficients[i][j] = json[i][j].asFloat();
				}
			}
		}
	}
}