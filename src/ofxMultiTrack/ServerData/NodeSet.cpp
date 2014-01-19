#include "NodeSet.h"

namespace ofxMultiTrack {
	namespace ServerData {
		//----------
		NodeSet::TransformSet & NodeSet::getTransforms() {
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
	}
}