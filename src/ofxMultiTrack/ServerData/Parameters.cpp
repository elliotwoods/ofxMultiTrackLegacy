#include "Parameters.h"

namespace ofxMultiTrack {
	namespace ServerData {
		//----------
		ParameterRegister Parameters = ParameterRegister();

		//----------
		ParameterRegister::ParameterRegister() {
			this->setDefaults();
		}
		
		//----------
		void ParameterRegister::setDefaults() {
			this->combineDistanceThreshold = 0.2f;
		}
	}
}