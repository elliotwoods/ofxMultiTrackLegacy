#pragma once
#include "NodeConnection.h"
#include "../Align/Base.h"
#include "User.h"

namespace ofxMultiTrack {
	namespace ServerData {
		class NodeSet : public vector<shared_ptr<NodeConnection>> {
		public:
			typedef map<CombinedUserSet::NodeUserIndex, CombinedUserSet::SourceMapping> UserMatches;

			NodeSet();

			vector<UserSet> getUsersView() const;
			vector<UserSet> getUsersWorld() const;
			vector<UserSet> getUsersWorld(const vector<UserSet> & usersView) const;
			CombinedUserSet getUsersCombined() const;
			CombinedUserSet getUsersCombined(const vector<UserSet> & usersWorld) const;
			UserMatches makeUserMappings(const vector<UserSet> & userWorld) const;
		};
	}
}
