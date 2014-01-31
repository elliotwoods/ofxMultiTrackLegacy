#pragma once
#include "NodeConnection.h"
#include "../Align/Base.h"
#include "User.h"

namespace ofxMultiTrack {
	namespace ServerData {
		class NodeSet : public vector<shared_ptr<NodeConnection>> {
		public:
			struct NodeUserIndex {
				NodeUserIndex(int nodeIndex, int userIndex);
				bool operator<(const NodeUserIndex & other) const;
				int nodeIndex;
				int userIndex;
			};

			typedef map<NodeUserIndex, vector<NodeUserIndex> > UserMatches;

			NodeSet();

			vector<UserSet> getUsersView() const;
			vector<UserSet> getUsersWorld() const;
			vector<UserSet> getUsersWorld(const vector<UserSet> & usersView) const;
			CombinedUserSet getUsersCombined() const;
			CombinedUserSet getUsersCombined(const vector<UserSet> & usersWorld) const;
			UserMatches getUserMatches(const vector<UserSet> & userWorld) const;
		};
	}
}
