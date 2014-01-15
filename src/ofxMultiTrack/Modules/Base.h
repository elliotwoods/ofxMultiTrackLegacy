#pragma once

#include "ofFileUtils.h"
#include "ofTypes.h"
#include "ofxJSON/libs/jsoncpp/include/json/json.h"

namespace ofxMultiTrack {
	namespace Modules {
		class Base {
		public:
			virtual string getType() = 0;
			virtual void init() = 0;
			virtual void update() = 0;
			virtual Json::Value serialize() = 0;
			virtual void deserialize(const Json::Value& data) = 0;
			virtual string getStatus() = 0;
		protected:
		};
	}
}