#pragma once

#include <vector>
#include "ofTypes.h"

namespace ofxMultiTrack {
	namespace Devices {
		class Base {
		public:
			virtual string getType() = 0;
			virtual void init() = 0;
			virtual void update() = 0;
		protected:
		};
	}
}