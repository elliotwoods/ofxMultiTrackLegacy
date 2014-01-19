#pragma once
#include "NodeConnection.h"
#include "../Align/Base.h"

namespace ofxMultiTrack {
	namespace ServerData {
		class NodeSet : public vector<shared_ptr<NodeConnection>> {
		public:
			struct Transform {
				int source;
				Align::Ptr transform;
			};
			typedef map<int, Transform> TransformSet;
			TransformSet & getTransforms();
			void setTransform(int nodeIndex, int sourceNodeIndex, Align::Ptr transform);
		protected:
			TransformSet transforms;
		};
	}
}
