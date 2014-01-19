#include "NodeSet.h"

namespace ofxMultiTrack {
	namespace ServerData {
		//----------
		void NodeSet::setTransform(int nodeIndex, int sourceNodeIndex, Align::Ptr transform) {
			Transform newTransform;
			newTransform.source = sourceNodeIndex;
			newTransform.transform = transform;
			this->transforms[nodeIndex] = newTransform;
		}
	}
}