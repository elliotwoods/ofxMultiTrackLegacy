#pragma once

#include "User.h"

namespace ofxMultiTrack {
	namespace ServerData {
		struct OutputFrame {
			vector<ServerData::UserSet> views;
			vector<ServerData::UserSet> world;
			ServerData::CombinedUserSet combined;
		};
	}
}