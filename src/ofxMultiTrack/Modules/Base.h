#pragma once

#include "ofFileUtils.h"
#include "ofTypes.h"
#include "ofxJSON/libs/jsoncpp/include/json/json.h"

namespace ofxMultiTrack {
	namespace Modules {
		class Base {
		public:
			virtual ~Base() { }
			virtual string getType() const = 0;
			virtual void init() = 0;
			virtual void update() = 0;
			virtual Json::Value serialize() = 0;
			virtual void deserialize(const Json::Value& data) = 0;
			virtual Json::Value getStatus() = 0;
			virtual void setConfig(const Json::Value &) { };
		protected:
		};
	}
}