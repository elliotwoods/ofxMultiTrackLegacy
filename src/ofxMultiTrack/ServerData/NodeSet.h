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

			struct NodeUserIndex {
				NodeUserIndex(int nodeIndex, int userIndex);
				bool operator<(const NodeUserIndex & other) const;
				int nodeIndex;
				int userIndex;
			};

			typedef map<NodeUserIndex, vector<NodeUserIndex> > UserMatches;

			NodeSet();
			bool isCalibrated() const;
			TransformSet & getTransforms();
			const TransformSet & getTransforms() const;
			void setTransform(int nodeIndex, int sourceNodeIndex, Align::Ptr transform);
			void applyTransform(UserSet & users, int nodeIndex) const;
			ofVec3f applyTransform(const ofVec3f &, int nodeIndex) const;

			vector<UserSet> getUsersView() const;
			vector<UserSet> getUsersWorld() const;
			vector<UserSet> getUsersWorld(const vector<UserSet> & usersView) const;
			CombinedUserSet getUsersCombined() const;
			CombinedUserSet getUsersCombined(const vector<UserSet> & usersWorld) const;
			UserMatches getUserMatches(const vector<UserSet> & userWorld) const;
		protected:
			TransformSet transforms;
			bool enableCalibration;
		};
	}
}
