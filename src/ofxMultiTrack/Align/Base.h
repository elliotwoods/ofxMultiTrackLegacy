#pragma once
#include "ofVec3f.h"
#include "ofxJSON/libs/jsoncpp/include/json/json.h"
#include <memory>

namespace ofxMultiTrack {
	namespace Align {
		///Routines to align Node spaces inherit this class
		class Base {
		public:
			virtual ~Base() { }
			virtual string getType() const = 0;
			virtual void calibrate(const vector<ofVec3f> & source, const vector<ofVec3f> & target) = 0;
			virtual ofVec3f transform(const ofVec3f &) const = 0;

			virtual Json::Value serialise() const = 0;
			virtual void deserialise(const Json::Value &) = 0;
		};

		typedef shared_ptr<Base> Ptr;
	}
}