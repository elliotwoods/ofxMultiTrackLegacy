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
			string connectedTo;
		};

		class User : public std::map<string, Joint>, public ISerialisable {
		public:
			User();
			User(const vector<User> &); ///<create an average user out of a set
			void serialise(Json::Value &) const override;
			void deserialise(const Json::Value &) override;
			void setAlive(bool);
			bool isAlive() const;
			float getDistanceTo(const User &) const;
			void draw();
		protected:
			bool alive;
		};
	
		class UserSet : public vector<User>, public ISerialisable {
		public:
			void serialise(Json::Value &) const override;
			void deserialise(const Json::Value &) override;
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