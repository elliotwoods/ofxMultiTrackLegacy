#pragma once

#include "ofParameter.h"

namespace ofxMultiTrack {
	namespace ServerData {
		class ParameterRegister {
		public:
			ParameterRegister();
			void setDefaults();

			ofParameter<float> combineDistanceThreshold;
		};
		extern ParameterRegister Parameters;
	}
}