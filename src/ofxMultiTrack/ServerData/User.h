#pragma once

#include "ofTypes.h"
#include "ofQuaternion.h"

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
			bool isAlive() const;
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
			typedef vector<map<int, int> > SourceMapping;

			///add map of origin users (used in tandem with .push_back(user) )
			void addSourceMapping(const map<int, int> &);
			const SourceMapping & getSourceMapping() const;
		protected:
			SourceMapping sourceUserMapping; /// < mapping of user in this list -> mapping of node -> this user index in node view
		};
	}
}