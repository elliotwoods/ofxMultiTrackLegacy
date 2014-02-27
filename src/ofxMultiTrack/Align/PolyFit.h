#pragma once

#include "Base.h"
#include "ofxPolyFit/src/ofxPolyFit.h"

namespace ofxMultiTrack {
	namespace Align {
		class PolyFit : public Base {
		public:
			PolyFit();
			~PolyFit();
			string getType() const override;
			void calibrate(const vector<ofVec3f> & thisSpace, const vector<ofVec3f> & originSpace) override;
			ofVec3f applyTransform(const ofVec3f &) const override;

			Json::Value serialise() const override;
			void deserialise(const Json::Value &) override;
		protected:
			ofxPolyFit fit;
			pfitDataPointf * transformPoint; ///<only allocate once
		};
	}
}