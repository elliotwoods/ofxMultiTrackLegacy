#pragma once

#include "Base.h"
#include "ofxNonLinearFit/src/ofxNonLinearFit.h"

namespace ofxMultiTrack {
	namespace Align {
		class NonLinear : public Base {
		public:
			NonLinear();
			~NonLinear();
			string getType() const;
			void calibrate(const vector<ofVec3f> & thisSpace, const vector<ofVec3f> & originSpace) override;
			ofVec3f applyTransform(const ofVec3f &) const override;

			Json::Value serialise() const override;
			void deserialise(const Json::Value &) override;
		protected:
			typedef ofxNonLinearFit::Models::RigidBody Model;

			Model model;
			ofxNonLinearFit::Fit<Model> fit;
		};
	}
}