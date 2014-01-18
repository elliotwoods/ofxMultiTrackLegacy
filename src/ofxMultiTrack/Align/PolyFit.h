#pragma once

#include "Base.h"
#include "ofxPolyFit/src/ofxPolyFit.h"

namespace ofxMultiTrack {
	namespace Align {
		class PolyFit : public Base {
		public:
			PolyFit();
			~PolyFit();
			string getType() const;
			void calibrate(const vector<ofVec3f> & source, const vector<ofVec3f> & target) override;
			ofVec3f transform(const ofVec3f &) const override;

			Json::Value serialise() const override;
			void deserialise(const Json::Value &) override;
		protected:
			ofxPolyFit fit;
			pfitDataPointf * transformPoint; ///<only allocate once
		};
	}
}