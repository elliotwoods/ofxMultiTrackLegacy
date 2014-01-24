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
			this->enableCalibration = false;
		}

		//----------
		bool NodeSet::isCalibrated() const {
			return this->enableCalibration;
		}

		//----------
		NodeSet::TransformSet & NodeSet::getTransforms() {
			return this->transforms;
		}

		//----------
		const NodeSet::TransformSet & NodeSet::getTransforms() const {
			return this->transforms;
		}

		//----------
		void NodeSet::setTransform(int nodeIndex, int sourceNodeIndex, Align::Ptr transform) {
			Transform newTransform;
			newTransform.source = sourceNodeIndex;
			newTransform.transform = transform;
			this->transforms[nodeIndex] = newTransform;
			this->enableCalibration = true;
		}

		//----------
		void NodeSet::applyTransform(UserSet & users, int nodeIndex) const {
			auto transform = this->transforms.find(nodeIndex);
			if(transform == this->transforms.end()) {
				// do nothing, there is no transform for this node
			} else {
				// first apply the upstream transform
				this->applyTransform(users, transform->second.source);

				// apply our transform
				auto transformFunction = transform->second.transform;
				for(auto & user : users) {
					for(auto & joint : user) {
						joint.second.position = transformFunction->transform(joint.second.position);
					}
				}
			}
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
				this->applyTransform(users, viewIndex);
			}
			return usersWorld;
		}

		//----------
		CombinedUserSet NodeSet::getUsersCombined() const {
			return this->getUsersCombined(this->getUsersWorld());
		}

		//----------
		CombinedUserSet NodeSet::getUsersCombined(const vector<UserSet> & usersWorld) const {
			auto userMatches = this->getUserMatches(usersWorld);
			CombinedUserSet combinedUserSet;
			for(auto baseUser : userMatches) {
				vector<User> usersToCombine;
				for(auto userToCombineIndex : baseUser.second) {
					usersToCombine.push_back(usersWorld[userToCombineIndex.nodeIndex][userToCombineIndex.userIndex]);
				}
				combinedUserSet.push_back(User(usersToCombine));
			}
			return combinedUserSet;
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
				}
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