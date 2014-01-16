#pragma once

#include "ofTypes.h"

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

	typedef map<string, Joint> User;
	typedef vector<User> UserSet;
	typedef long long Timestamp;
}