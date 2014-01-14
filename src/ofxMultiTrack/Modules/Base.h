#pragma once

#include "ofFileUtils.h"
#include "ofTypes.h"

namespace ofxMultiTrack {
	namespace Modules {
		class Base {
		public:
			virtual string getType() = 0;
			virtual void init() = 0;
			virtual void update() = 0;
			virtual void serialize(ofBuffer& data) = 0;
			virtual void deserialize(const ofBuffer& data) = 0;
		protected:
		};
	}
}