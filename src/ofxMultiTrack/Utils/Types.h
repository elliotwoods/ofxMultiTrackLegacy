#pragma once

#include "ofTypes.h"
#include "ofNode.h"
#include <map>
#include "ofxJSON/libs/jsoncpp/include/json/json.h"

namespace ofxMultiTrack {
	class Exception : public std::exception {
	public:
		Exception(string text);
		string what();
	protected:
		string text;
	};

	class ISerialisable {
	public:
		virtual void serialise(Json::Value &) const = 0;
		virtual void deserialise(const Json::Value &) = 0;
	};
	
	struct NodeSettings {
		int localIndex;
	};

	struct ServerToNodeHeader {

	};

	typedef long long Timestamp;
}