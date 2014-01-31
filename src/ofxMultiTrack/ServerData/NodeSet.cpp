#include "NodeSet.h"

namespace ofxMultiTrack {
	namespace ServerData {
#pragma mark NodeUserIndex
		//----------
		NodeSet::NodeUserIndex::NodeUserIndex(int nodeIndex, int userIndex) :
			nodeIndex(nodeIndex), userIndex(userIndex) {

		}

		//----------
		bool NodeSet::NodeUserIndex::operator<(const NodeUserIndex & other) const {
			if (this->nodeIndex != other.nodeIndex) {
				return this->nodeIndex < other.nodeIndex;
			} else {
				return this->userIndex < other.userIndex;
			}
		}

#pragma mark NodeSet
		//----------
		NodeSet::NodeSet() {
		}

		//----------
		vector<UserSet> NodeSet::getUsersView() const {
			vector<UserSet> usersView;
			for(auto node : *this) {
				usersView.push_back(node->getLiveData());
			}
			return usersView;
		}

		//----------
		vector<UserSet> NodeSet::getUsersWorld() const {
			return this->getUsersWorld(this->getUsersView());
		}

		//----------
		vector<UserSet> NodeSet::getUsersWorld(const vector<UserSet> & usersView) const {
			auto usersWorld = usersView;
			int viewIndex = 0;
			for(auto & users : usersWorld) {
				this->at(viewIndex)->applyTransform(users);
				viewIndex++;
			}
			return usersWorld;
		}

		//----------
		CombinedUserSet NodeSet::getUsersCombined() const {
			return this->getUsersCombined(this->getUsersWorld());
		}

		//----------
		CombinedUserSet NodeSet::getUsersCombined(const vector<UserSet> & usersWorld) const {

			// HACK!!
			//combine all users into one
			vector<User> users;
			for(auto view : usersWorld) {
				for(auto user : view) {
					users.push_back(user);
				}
			}
			CombinedUserSet combined;
			combined.push_back(User(users));
			return combined;


			/*
			auto userMatches = this->getUserMatches(usersWorld);
			CombinedUserSet combinedUserSet;
			for(auto baseUser : userMatches) {
				vector<User> usersToCombine;
				map<int, int> sourceMapping;
				for(auto userToCombineIndex : baseUser.second) {
					usersToCombine.push_back(usersWorld[userToCombineIndex.nodeIndex][userToCombineIndex.userIndex]);
					sourceMapping.insert(pair<int, int>(userToCombineIndex.nodeIndex, userToCombineIndex.userIndex));
				}
				combinedUserSet.push_back(User(usersToCombine));
				combinedUserSet.addSourceMapping(sourceMapping);
			}
			return combinedUserSet;
			*/
		}

		//----------
		NodeSet::UserMatches NodeSet::getUserMatches(const vector<UserSet> & userWorld) const {
			//precache a list of users across all nodes
			set<NodeSet::NodeUserIndex> userIndices;
			int nodeIndex = 0;
			for(const auto & node : userWorld) {
				int userIndex = 0;
				for(const auto & user : node) {
					userIndices.insert(NodeUserIndex(nodeIndex, userIndex));
					userIndex++;
				}
				nodeIndex++;
			}

			//allocate solution
			NodeSet::UserMatches matches;

			//perform search
			User combinedUser;
			while(!userIndices.empty()) {
				auto & searchUserIndex = * userIndices.begin();
				const auto & searchUser = userWorld[searchUserIndex.nodeIndex][searchUserIndex.userIndex];

				//------do we need this?
				////make a list of nodes which won't be used for search
				//set<int> nodesUserSeenIn;
				//nodesUserSeenIn.insert(searchUserIndex.nodeIndex);

				//--
				//search other users for best matching user
				//
				float bestMatchDistance = std::numeric_limits<float>::max(); //lower is better
				NodeUserIndex * bestMatch = NULL;
				for(auto otherUserIndex : userIndices) {
					//don't look within the same node for matching users
					// (and therefore also don't look at self)
					if (otherUserIndex.nodeIndex == searchUserIndex.nodeIndex) {
						continue;
					}

					const auto & otherUser = userWorld[otherUserIndex.nodeIndex][otherUserIndex.userIndex];
					float matchDistance = searchUser.getDistanceTo(otherUser);
					
					if(matchDistance < bestMatchDistance) {
						bestMatchDistance = matchDistance;
						bestMatch = & otherUserIndex;
					}
				}
				//
				//--

				if(bestMatch != NULL && bestMatchDistance < OFXMULTITRACK_SERVER_COMBINE_DISTANCE_THRESHOLD) {
					if (matches.count(searchUserIndex) == 0) {
						//add this base user to the results map indices
						matches.insert(pair<NodeUserIndex, vector<NodeUserIndex> >(searchUserIndex, vector<NodeUserIndex>()));
						
						//add this base user to the results at this index
						matches.at(searchUserIndex).push_back(searchUserIndex);
					}
					matches.at(searchUserIndex).push_back(*bestMatch);
				} else {
					//no more matches for this user, remove it from the search
					userIndices.erase(searchUserIndex);
				}
			}

			return matches;
		}
	}
}