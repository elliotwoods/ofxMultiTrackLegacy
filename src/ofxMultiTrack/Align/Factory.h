#pragma once

#include "Base.h"
#include "ofMatrix4x4.h"

namespace ofxMultiTrack {
	namespace Align {
		class Factory {
		public:
			static Align::Ptr makeDefault();
			static Align::Ptr make(const string type);
			static Align::Ptr make(const ofMatrix4x4 &);
		};
	}
}