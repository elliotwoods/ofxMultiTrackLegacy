#pragma once

#include <vector>
#include "ofTypes.h"
#include "ofxJSON/libs/jsoncpp/include/json/json.h"

namespace ofxMultiTrack {
	namespace Devices {
		class Base {
		public:
			virtual string getType() const = 0;
			virtual void init() = 0;
			virtual void update() = 0;
			virtual Json::Value getStatus() = 0;
		protected:
		};
	}
}