#pragma once

#include "Base.h"
#include "ofxNonLinearFit/src/ofxNonLinearFit.h"
#include "ofParameter.h"

namespace ofxMultiTrack {
	namespace Align {
		class RigidBodyFit : public Base {
		public:
			RigidBodyFit();
			~RigidBodyFit();
			string getType() const override;
			void calibrate(const vector<ofVec3f> & thisSpace, const vector<ofVec3f> & originSpace) override;
			ofVec3f applyTransform(const ofVec3f &) const override;

			Json::Value serialise() const override;
			void deserialise(const Json::Value &) override;

			const ofMatrix4x4 getMatrixTransform() const override;
			const vector<shared_ptr<ofParameter<float>>> & getParameters();
		protected:
			void updateParameters();
			void callbackTransformParameter(float &);

			typedef ofxNonLinearFit::Models::RigidBody Model;

			Model model;
			ofxNonLinearFit::Fit<Model> fit;
			vector<shared_ptr<ofParameter<float>>> parameters;
			bool parameterCallbackEnabled;
		};
	}
}