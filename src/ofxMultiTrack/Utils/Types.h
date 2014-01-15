#pragma once

#include "ofTypes.h"

namespace ofxMultiTrack {
	struct NodeSettings {
		int localIndex;
	};

	struct ServerToNodeHeader {

	};

	namespace Data {
		struct Joint {
			ofVec3f position;
			ofQuaternion rotation;
		};

		typedef map<string, Joint> User;
	}
}