#include "Set.h"

namespace ofxMultiTrack {
	namespace Devices {
		//---------
		ofPtr<Base> Set::find(string deviceType) {
			for(auto device : *this) {
				if (device->getType() == deviceType) {
					return device;
				}
			}
			string error = "Device of type [" + deviceType + "] could not be found";
			throw(std::exception(error.c_str()));
		}
	}
}