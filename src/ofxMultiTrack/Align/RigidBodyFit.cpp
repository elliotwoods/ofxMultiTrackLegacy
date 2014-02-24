#include "RigidBodyFit.h"

using namespace ofxNonLinearFit;

namespace ofxMultiTrack {
	namespace Align {
		//----------
		RigidBodyFit::RigidBodyFit() :
			fit(Algorithm(nlopt::LN_NEWUOA, Algorithm::LocalGradientless)) {
		}

		//----------
		RigidBodyFit::~RigidBodyFit() {
		}

		//----------
		string RigidBodyFit::getType() const {
			return "RigidBodyFit";
		}

		//----------
		void RigidBodyFit::calibrate(const vector<ofVec3f> & thisSpace, const vector<ofVec3f> & originSpace) {
			this->model.clearParameters();
			auto dataSet = Model::DataSet();

			int iPoint = 0;
			for(int i=0; i<thisSpace.size(); i++) {
				Model::DataPoint dataPoint;
				dataPoint.x = thisSpace[i];
				dataPoint.xdash = originSpace[i];
				dataSet.push_back(dataPoint);
			}

			double residual = 0.0;
			fit.optimise(this->model, &dataSet, &residual);
			cout << "Residual : " << sqrt(residual / (double) dataSet.size()) << "m" << endl;
		}

		//----------
		ofVec3f RigidBodyFit::applyTransform(const ofVec3f & thisSpace) const {
			return this->model.evaluate(thisSpace);
		}

		//----------
		Json::Value RigidBodyFit::serialise() const {
			Json::Value json;

			bool ready = this->model.isReady();
			json["ready"] = ready;

			if (ready) {
				const auto parameters = this->model.getParameters();
				
				auto & jsonTranslate = json["translate"];
				for(int i=0; i<3; i++) {
					jsonTranslate[i] = parameters[i];
				}

				auto & jsonRotate = json["rotate"];
				for(int i=0; i<3; i++) {
					jsonRotate[i] = parameters[i + 3];
				}
			}

			return json;
		}

		//----------
		void RigidBodyFit::deserialise(const Json::Value & json) {
			if(json["ready"].asBool()) {
				Model::Parameter parameters[6];
				std::fill(parameters, parameters + 6, 0);

				auto jsonTranslate = json["translate"];
				for(int i=0; i<3; i++) {
					parameters[i] = jsonTranslate[i].asDouble();
				}

				auto jsonRotate = json["rotate"];
				for(int i=0; i<3; i++) {
					parameters[i + 3] = jsonRotate[i].asDouble();
				}

				this->model.setParameters(&parameters[0]);
			}
		}
	}
}