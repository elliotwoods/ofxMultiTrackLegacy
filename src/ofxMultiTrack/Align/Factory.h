#pragma once

#include "Base.h"

namespace ofxMultiTrack {
	namespace Align {
		class Factory {
		public:
			static Align::Ptr makeDefault();
			static Align::Ptr make(const string type);
		};
	}
}