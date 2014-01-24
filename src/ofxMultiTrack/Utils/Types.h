#pragma once

#include "ofTypes.h"
#include "ofNode.h"
#include <map>
#include "ofxJSON/libs/jsoncpp/include/json/json.h"

namespace ofxMultiTrack {
	class Exception {
	public:
		Exception(string text);
		string what();
	protected:
		string text;
	};
	
	struct NodeSettings {
		int localIndex;
	};

	struct ServerToNodeHeader {

	};

	typedef long long Timestamp;
}