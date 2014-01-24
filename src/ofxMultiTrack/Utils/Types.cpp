#include "Types.h"

namespace ofxMultiTrack {
#pragma mark Exception
	//----------
	Exception::Exception(string text) : text(text) {

	}

	//----------
	string Exception::what() {
		return this->text;
	}
}