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
		};

		class User : public std::map<string, Joint>, public ofNode {
		public:
			User();
			User(const vector<User> &); ///<creata an average user out of a set
			Json::Value serialise() const;
			void deserialise(const Json::Value &);
			void setAlive(bool);
			bool getAlive() const;
		protected:
			void customDraw();
			bool alive;
		};
	
		class UserSet : public vector<User>, public ofNode {
		public:
			Json::Value serialise() const;
			void deserialise(const Json::Value &);
		protected:
			void customDraw();
		};
	}
}