#include "NodeSet.h"

namespace ofxMultiTrack {
	namespace ServerData {
		//----------
		NodeSet::NodeSet() {
		}

		//----------
		vector<UserSet> NodeSet::getUsersView() const {
			vector<UserSet> usersView;
			for(auto node : *this) {
				if(node->isConnected()) {
					usersView.push_back(node->getLiveData());
				} else {
					usersView.push_back(UserSet());
				}
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
			auto userMappings = this->makeUserMappings(usersWorld);
			CombinedUserSet combinedUserSet;
			for(auto & sourceMapping : userMappings) {
				vector<User> sourceUserData;
				for(auto & sourceUser : sourceMapping.second) {
					sourceUserData.push_back(usersWorld[sourceUser.first.nodeIndex][sourceUser.first.userIndex]);
				}
				combinedUserSet.push_back(User(sourceUserData));
				combinedUserSet.addSourceMapping(sourceMapping.second);
			}
			return combinedUserSet;
		}

		//----------
		NodeSet::UserMatches NodeSet::makeUserMappings(const vector<UserSet> & userWorld) const {
			//pull out some types
			typedef CombinedUserSet::NodeUserIndex NodeUserIndex;
			typedef CombinedUserSet::SourceMapping SourceMapping;

			//precache a list of users across all nodes
			set<NodeUserIndex> userIndices;
			int nodeIndex = 0;
			for(const auto & node : userWorld) {
				int userIndex = 0;
				for(const auto & user : node) {
					userIndices.insert(NodeUserIndex(nodeIndex, userIndex));
					userIndex++;
				}
				nodeIndex++;
			}

			//strip out blank users from search
			for(auto it = userIndices.begin(); it != userIndices.end(); ) {
				const auto & user = userWorld[it->nodeIndex][it->userIndex];
				if (user.empty() || !user.isAlive()) {
					userIndices.erase(it++);
				} else {
					++it;
				}
			}

			//allocate solution
			NodeSet::UserMatches matches;

			//perform search
			while(!userIndices.empty()) {
				//start with first user in our set which hasn't been output yet
				auto & searchUserIndex = * userIndices.begin();
				const auto & searchUser = userWorld[searchUserIndex.nodeIndex][searchUserIndex.userIndex];
				auto & searchMapping = matches[searchUserIndex];

				//check if it includes itself yet
				if(searchMapping.count(searchUserIndex) == 0) {
					//if not add it, and it's got zero distance of course
					searchMapping[searchUserIndex] = 0.0f;
				}
				
				//--
				//search other users for best matching user
				//
				float bestMatchDistance = std::numeric_limits<float>::max(); //lower is better
				NodeUserIndex * bestMatch = NULL;
				for(auto otherUserIndex : userIndices) {
					//check if we've already got a user from this node
					bool alreadyHasThisNode = false;
					for(auto matches : searchMapping) {
						if(matches.first.nodeIndex == otherUserIndex.nodeIndex) {
							alreadyHasThisNode = true;
							break;
						}
					}
					if (alreadyHasThisNode) {
						//if so, then skip and continue
						continue;
					}

					//compare distance score against this other user
					const auto & otherUser = userWorld[otherUserIndex.nodeIndex][otherUserIndex.userIndex];
					float matchDistance = searchUser.getDistanceTo(otherUser);
					
					//if it's best score so far out of current search, let's keep it
					if(matchDistance < bestMatchDistance) {
						bestMatchDistance = matchDistance;
						bestMatch = & otherUserIndex;
					}
				}
				//
				//--

				//if we found anything (useful) at all
				if(bestMatch != NULL && bestMatchDistance < OFXMULTITRACK_SERVER_COMBINE_DISTANCE_THRESHOLD) {
					//then add it as a source mapping
					searchMapping[*bestMatch] = bestMatchDistance;

					//and remove it from search
					userIndices.erase(*bestMatch);
				} else {
					//no more matches for this user, remove it from the search
					userIndices.erase(searchUserIndex);
				}
			}

			return matches;
		}
	}
}