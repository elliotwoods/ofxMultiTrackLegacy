#pragma once

#include "ofTypes.h"
#include "ofNode.h"
#include <map>
#include "ofxJSON/libs/jsoncpp/include/json/json.h"

namespace ofxMultiTrack {
	struct NodeSettings {
		int localIndex;
	};

	struct ServerToNodeHeader {

	};

	struct Joint {
		Json::Value serialise() const;
		void deserialise(const Json::Value &);
		ofVec3f position;
		ofQuaternion rotation;
	};
	
	typedef long long Timestamp;

	class User : public std::map<string, Joint>, public ofNode {
	public:
		Json::Value serialise() const;
		void deserialise(const Json::Value &);
	protected:
		void customDraw();
	};
	
	class UserSet : public vector<User>, public ofNode {
	public:
		Json::Value serialise() const;
		void deserialise(const Json::Value &);
	protected:
		void customDraw();
	};
}