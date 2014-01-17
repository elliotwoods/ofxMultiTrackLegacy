#pragma once

#include "ofTypes.h"
#include "ofNode.h"
#include <map>

namespace ofxMultiTrack {
	struct NodeSettings {
		int localIndex;
	};

	struct ServerToNodeHeader {

	};

	struct Joint {
		ofVec3f position;
		ofQuaternion rotation;
	};
	
	typedef long long Timestamp;

	class User : public std::map<string, Joint>, public ofNode {
	protected:
		void customDraw();
	};
	
	class UserSet : public vector<User>, public ofNode {
	protected:
		void customDraw();
	};
}