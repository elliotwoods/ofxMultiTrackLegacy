#include "User.h"
#include "ofxCvGui2/src/ofxCvGui/Assets.h"

namespace ofxMultiTrack {
	namespace ServerData {
#pragma mark Joint
		//----------
		void Joint::serialise(Json::Value & json) const {
			for(int i=0; i<3; i++) {
				json["position"][i] = this->position[i];
			}
			for(int i=0; i<4; i++) {
				json["rotation"][i] = this->rotation[i];
			}
			json["inferred"] = this->inferred;
			json["connectedTo"] = this->connectedTo;
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
			this->tracked = json["tracked"].asBool();
			this->connectedTo = json["connectedTo"].asString();
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
		void User::serialise(Json::Value & json) const {
			for(auto & joint : *this) {
				joint.second.serialise(json[joint.first]);
			}
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
			//N.B. didn't use RMS as it is more sensitive to a few large differences in the set
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
		void User::draw(bool enableColors) {
			ofMesh points;
			ofMesh lines;
			if (this->alive) {
				for(auto & joint : *this) {
					points.addVertex(joint.second.position);
					if (enableColors) {
						auto color = joint.second.tracked ? (joint.second.inferred ? ofColor::blue : ofColor::white) : ofColor::red;
						points.addColor(color);
					}

					auto findConnected = this->find(joint.second.connectedTo);
					if (findConnected != this->end()) {
						lines.addVertex(joint.second.position);
						lines.addVertex(findConnected->second.position);
					}
				}
			}
			points.drawVertices();
			lines.setMode(OF_PRIMITIVE_LINES);
			glDisable(GL_TEXTURE_2D);
			lines.draw();
			glEnable(GL_TEXTURE_2D);
		}

#pragma mark UserSet
		//----------
		void UserSet::serialise(Json::Value & json) const {
			int userIndex = 0;
			for(auto & user : *this) {
				user.serialise(json[userIndex++]);
			}
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
		void UserSet::draw(bool enableColors) {
			for(auto & user : *this) {
				user.draw(enableColors);
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