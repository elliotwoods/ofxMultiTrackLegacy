#pragma once

#include "ofTypes.h"
#include "ofQuaternion.h"

#include <map>
#include "ofxJSON/libs/jsoncpp/include/json/json.h"
#include "../Utils//Types.h"

namespace ofxMultiTrack {
	namespace ServerData {
		class Joint : public ISerialisable {
		public:
			void serialise(Json::Value &) const override;
			void deserialise(const Json::Value &) override;
			ofVec3f position;
			ofQuaternion rotation;
			bool inferred;
			bool tracked;
			string connectedTo;
		};

		class User : public std::map<string, Joint>, public ISerialisable {
		public:
			typedef int GlobalIndex;

			User();
			User(const vector<User> &); ///<create an average user out of a set
			void serialise(Json::Value &) const override;
			void deserialise(const Json::Value &) override;
			void setAlive(bool);
			bool isAlive() const;
			bool hasTrackedJoints() const;
			float getDistanceTo(const User &) const;
			void draw(bool enableColors = true) const;
			void setGlobalIndex(GlobalIndex);
			void assignForEmptyGlobalIndex();
			GlobalIndex getGlobalIndex() const;
		protected:
			bool alive;
			GlobalIndex globalIndex;

			static GlobalIndex nextGlobalIndex;
		};
	
		class UserSet : public vector<User>, public ISerialisable {
		public:
			void serialise(Json::Value &) const override;
			void deserialise(const Json::Value &) override;
			void draw(bool enableColors = true) const;
		};

		class CombinedUserSet : public UserSet {
		public:
			struct NodeUserIndex {
				NodeUserIndex();
				NodeUserIndex(int nodeIndex, int userIndex);
				bool operator<(const NodeUserIndex &) const;
				bool operator==(const NodeUserIndex &) const;
				friend ostream& operator<<(ostream&, const NodeUserIndex&);

				int nodeIndex;
				int userIndex;
				bool valid;
			};

			typedef float Distance;
			typedef map<NodeUserIndex, Distance> SourceMapping;

			void addSourceMapping(const SourceMapping &);
			const vector<SourceMapping> & getSourceMappings() const;

			void matchFromPreviousFrame(const CombinedUserSet &);
			void assignForEmptyGlobalIndices();
		protected:
			vector<SourceMapping> sourceUserMappings; /// < mapping of user in this list -> matches used in source set
		};
	}
}