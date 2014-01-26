#include "User.h"
#include "ofxCvGui2/src/ofxCvGui/Assets.h"

namespace ofxMultiTrack {
	namespace ServerData {
#pragma mark Joint
		//----------
		Json::Value Joint::serialise() const {
			Json::Value json;
			for(int i=0; i<3; i++) {
				json["position"][i] = this->position[i];
			}
			for(int i=0; i<4; i++) {
				json["rotation"][i] = this->rotation[i];
			}
			json["inferred"] = this->inferred;
			return json;
		}

		//----------
		void Joint::deserialise(const Json::Value & json){
			for(int i=0; i<3; i++) {
				this->position[i] = json["position"][i].asFloat();
			}
			for(int i=0; i<4; i++) {
				this->rotation[i] = json["rotation"][i].asFloat();
			}
			this->inferred = json["inferred"].asBool();
		}

#pragma mark User
		//----------
		User::User() {
			this->alive = true;
		}

		//----------
		User::User(const vector<User> & userSet) {
			int foundUserCount = 0;
			for(auto & user : userSet) {
				if (user.size() == 0) {
					//blank user, carry on
					continue;
				}

				foundUserCount++;
				for(auto & joint : user) {
					(*this)[joint.first].position += joint.second.position;

					//currently we cheat and use last found rotation
					(*this)[joint.first].rotation = joint.second.rotation;
				}
			}

			for(auto & joint : *this) {
				joint.second.position /= (float) foundUserCount;
			}
		}

		//----------
		Json::Value User::serialise() const {
			Json::Value json;
			for(auto & joint : *this) {
				json[joint.first] = joint.second.serialise();
			}
			return json;
		}

		//----------
		void User::deserialise(const Json::Value & json) {
			this->clear();
			auto jointNames = json.getMemberNames();
			for(auto jointName : jointNames) {
				auto joint = Joint();
				joint.deserialise(json[jointName]);
				(*this)[jointName] = joint;
			}
		}
	
		//----------
		void User::setAlive(bool alive) {
			this->alive = alive;
		}

		//----------
		bool User::isAlive() const {
			return this->alive;
		}
		
		//----------
		float User::getDistanceTo(const User & other) const {
			if (other.size() == 0 || !other.isAlive()) {
				return std::numeric_limits<float>::max();
			}

			//find mean of joint distances
			//N.B. didn't use RMS as it is more sensetive to a few large differences in the set
			float totalDistance = 0.0f;
			for(auto & joint : *this) {
				totalDistance += (joint.second.position - other.at(joint.first).position).length();
			}
			auto meanDistance = sqrt(totalDistance) / (float) this->size();

			////find rms of bone length difference
			//float totalDifference = 0.0f;
			////need a way of enumerating bones before we can use this for scoring
			//float meanDifference = totalDifference;

			return meanDistance;
		}

		//----------
		void User::draw() {
			ofMesh points;
			if (this->alive) {
				ofSetDrawBitmapMode(OF_BITMAPMODE_MODEL);
				for(auto & joint : *this) {
					points.addVertex(joint.second.position);
				}
				ofSetDrawBitmapMode(OF_BITMAPMODE_SIMPLE);
			}
			points.drawVertices();
		}

#pragma mark UserSet
		//----------
		Json::Value UserSet::serialise() const {
			Json::Value json;
			int userIndex = 0;
			for(auto & user : *this) {
				json[userIndex++] = user.serialise();
			}
			return json;
		}

		//----------
		void UserSet::deserialise(const Json::Value & json) {
			this->clear();
			for(int i=0; i<json.size(); i++) {
				User user;
				user.deserialise(json[i]);
				this->push_back(user);
			}
		}

		//----------
		void UserSet::draw() {
			for(auto & user : *this) {
				user.draw();
			}
		}

#pragma mark CombinedUserSet
		//----------
		void CombinedUserSet::addSourceMapping(const map<int, int> & sourceMapping) { 
			this->sourceUserMapping.push_back(sourceMapping);
		}

		//----------
		const CombinedUserSet::SourceMapping & CombinedUserSet::getSourceMapping() const {
			return this->sourceUserMapping;
		}
	}
}