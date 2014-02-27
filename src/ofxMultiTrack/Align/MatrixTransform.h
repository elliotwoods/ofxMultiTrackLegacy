#pragma once

#include "Base.h"
#include "ofxNonLinearFit/src/ofxNonLinearFit.h"

namespace ofxMultiTrack {
	namespace Align {
		class MatrixTransform : public Base {
		public:
			string getType() const override;
			void calibrate(const vector<ofVec3f> & thisSpace, const vector<ofVec3f> & originSpace) override;
			ofVec3f applyTransform(const ofVec3f &) const override;

			Json::Value serialise() const override;
			void deserialise(const Json::Value &) override;

			void setTransform(const ofMatrix4x4 &);
			const ofMatrix4x4 & getTransform() const;
		protected:
			ofMatrix4x4 transform;
		};
	}
}