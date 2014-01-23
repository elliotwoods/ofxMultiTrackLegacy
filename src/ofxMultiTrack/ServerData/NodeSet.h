#pragma once
#include "NodeConnection.h"
#include "../Align/Base.h"
#include "User.h"

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
			const TransformSet & getTransforms() const;
			void setTransform(int nodeIndex, int sourceNodeIndex, Align::Ptr transform);
			void applyTransform(UserSet &, int nodeIndex) const;
		protected:
			TransformSet transforms;
		};
	}
}
