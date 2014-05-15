#include "Utils.h"

#include <sys/timeb.h>

namespace ofxMultiTrack {
	namespace Utils {
		//----------
		unsigned long long getAbsoluteTime() {
			timeb timeOfDay;
			ftime(&timeOfDay);
			unsigned long long absoluteTime = timeOfDay.time;
			absoluteTime *= 1000;
			absoluteTime += timeOfDay.millitm;
			return absoluteTime;
		}
	}
}