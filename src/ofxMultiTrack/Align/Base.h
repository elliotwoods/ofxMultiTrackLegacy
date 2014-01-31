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
			virtual void calibrate(const vector<ofVec3f> & thisSpace, const vector<ofVec3f> & originSpace) = 0;
			virtual ofVec3f applyTransform(const ofVec3f &) const = 0; ///<transform a point from this space to parent origin space

			virtual Json::Value serialise() const = 0;
			virtual void deserialise(const Json::Value &) = 0;
		};

		typedef shared_ptr<Base> Ptr;
	}
}