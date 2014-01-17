#pragma once

#include <vector>
#include "ofTypes.h"

namespace ofxMultiTrack {
	namespace Devices {
		class Base {
		public:
			virtual string getType() const = 0;
			virtual void init() = 0;
			virtual void update() = 0;
			virtual string getStatus() = 0;
		protected:
		};
	}
}