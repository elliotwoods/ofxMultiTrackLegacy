#pragma once

#include "ofTypes.h"
#include "ofNode.h"
#include <map>
#include "ofxJSON/libs/jsoncpp/include/json/json.h"

namespace ofxMultiTrack {
	namespace ServerData {
		struct Joint {
			Json::Value serialise() const;
			void deserialise(const Json::Value &);
			ofVec3f position;
			ofQuaternion rotation;
			bool inferred;
		};

		class User : public std::map<string, Joint> {
		public:
			User();
			User(const vector<User> &); ///<create an average user out of a set
			Json::Value serialise() const;
			void deserialise(const Json::Value &);
			void setAlive(bool);
			bool getAlive() const;
			float getDistanceTo(const User &) const;
			void draw();
		protected:
			bool alive;
		};
	
		class UserSet : public vector<User>{
		public:
			Json::Value serialise() const;
			void deserialise(const Json::Value &);
			void draw();
		};

		class CombinedUserSet : public UserSet {
		public:
			typedef map<int, map<int, int> > SourceMapping;
			const SourceMapping & getSourceMapping() const;
		protected:
			SourceMapping sourceUserMapping; /// < mapping of user in this list -> mapping of node -> this user index in node view
		};
	}
}