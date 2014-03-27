#include "RigidBodyFit.h"

using namespace ofxNonLinearFit;

namespace ofxMultiTrack {
	namespace Align {
		//----------
		RigidBodyFit::RigidBodyFit() :
		fit(Algorithm(nlopt::LN_NEWUOA, Algorithm::LocalGradientless)) {
			for(int i=0; i<6; i++) {
				this->parameters.push_back(shared_ptr<ofParameter<float>>(new ofParameter<float>()));
				this->parameters[i]->addListener(this, & RigidBodyFit::callbackTransformParameter);
			}
			
			this->parameterCallbackEnabled = false;

			this->parameters[0]->set("Translate X", 0.0, -20.0f, 20.0f);
			this->parameters[1]->set("Translate Y", 0.0, -20.0f, 20.0f);
			this->parameters[2]->set("Translate Z", 0.0, -20.0f, 20.0f);
			
			this->parameters[3]->set("Rotate X", 0.0, -180.0f, 180.0f);
			this->parameters[4]->set("Rotate Y", 0.0, -180.0f, 180.0f);
			this->parameters[5]->set("Rotate Z", 0.0, -180.0f, 180.0f);

			this->parameterCallbackEnabled = true;

			Model::Parameter modelParameters[6];
			std::fill(modelParameters, modelParameters + 6, 0.0);
			this->model.setParameters(modelParameters);
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

			this->updateParameters();
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
			} else {
				this->model.clearParameters();
			}

			this->updateParameters();
		}

		//----------
		const ofMatrix4x4 RigidBodyFit::getMatrixTransform() const {
			return this->model.getCachedTransform();
		}

		//----------
		const vector<shared_ptr<ofParameter<float>>> & RigidBodyFit::getParameters() {
			return this->parameters;
		}

		//----------
		void RigidBodyFit::updateParameters() {
			this->parameterCallbackEnabled = false;
			const auto values = this->model.getParameters();
			for(int i=0; i<6; i++) {
				this->parameters[i]->set((float) values[i]);
			}
			this->parameterCallbackEnabled = true;
		}

		//----------
		void RigidBodyFit::callbackTransformParameter(float &) {
			if (! this->parameterCallbackEnabled) {
				return;
			}

			double values[6];
			for(int i=0; i<6; i++) {
				values[i] = (double) this->parameters[i]->get();
			}
			this->model.setParameters(values);
		}
	}
}