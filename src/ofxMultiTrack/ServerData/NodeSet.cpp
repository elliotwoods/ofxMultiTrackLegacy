#include "NodeSet.h"

namespace ofxMultiTrack {
	namespace ServerData {
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
			CombinedUserSet combinedUserSet;

			//go to first node with a user
			//check other nodes for users and add to search map
			//if closest per node is within threshold value:
			//	1. make a combined user for the combinedSet
			//	2. stop usimg users from other nodes in next search
			good morning elliot. implement this please :)
			
			return combinedUserSet;
		}
	}
}