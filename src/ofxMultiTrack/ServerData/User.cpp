#include "User.h"
#include "ofxCvGui2/src/ofxCvGui/Assets.h"
#include <numeric>

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
			json["connectedTo"] = this->connectedTo;
			json["tracked"] = this->tracked;
			json["inferred"] = this->inferred;
		}

		//----------
		void Joint::deserialise(const Json::Value & json){
			for(int i=0; i<3; i++) {
				this->position[i] = json["position"][i].asFloat();
			}
			for(int i=0; i<4; i++) {
				this->rotation[i] = json["rotation"][i].asFloat();
			}
			this->connectedTo = json["connectedTo"].asString();
			if(json["tracked"].isNull()) {
				//if we haven't stored a value for tracked (e.g. old data)
				//then we must mark as not-inferred since we don't use
				//inferred for calibration
				this->tracked = true;
				this->inferred = false;
			} else {
				this->tracked = json["tracked"].asBool();
				this->inferred = json["inferred"].asBool();
			}
		}

#pragma mark User
		//----------
		User::User() {
			this->alive = true;
		}
		
		//----------
		struct AccumulationJoint {
			AccumulationJoint(string connectedTo) :
				connectedTo(connectedTo) {
			}

			vector<ofVec3f> trackedPositions;
			vector<ofVec3f> inferredPositions;
			vector<ofQuaternion> trackedRotations;
			vector<ofQuaternion> inferredRotations;

			const string connectedTo;
		};

		//----------
		User::User(const vector<User> & userSet) {
			map<string, AccumulationJoint> accumulation;

			for(auto & user : userSet) {
				if(!user.isAlive()) {
					continue;
				}
				for(auto & joint : user) {
					if (accumulation.find(joint.first) == accumulation.end()) {
						accumulation.insert(pair<string, AccumulationJoint>(joint.first, AccumulationJoint(joint.second.connectedTo)));
					}
					auto & accumulationJoint = accumulation.at(joint.first);
					if(joint.second.tracked) {
						if(joint.second.inferred) {
							accumulationJoint.inferredPositions.push_back(joint.second.position);
							accumulationJoint.inferredRotations.push_back(joint.second.rotation);
						} else {
							accumulationJoint.trackedPositions.push_back(joint.second.position);
							accumulationJoint.trackedRotations.push_back(joint.second.rotation);
						}
					}
				}
			}

			for(auto & joint : accumulation) {
				const auto & trackedPositions = joint.second.trackedPositions;
				const auto & inferredPositions = joint.second.inferredPositions;
				auto & localJoint = (*this)[joint.first];
				if (!trackedPositions.empty()) {
					localJoint.position = std::accumulate(trackedPositions.begin(), trackedPositions.end(), ofVec3f()) / (float) joint.second.trackedPositions.size();
					localJoint.rotation = joint.second.trackedRotations.front();
				} else if (!joint.second.inferredPositions.empty()) {
					localJoint.position = std::accumulate(inferredPositions.begin(), inferredPositions.end(), ofVec3f()) / (float) joint.second.inferredPositions.size();
					localJoint.rotation = joint.second.inferredRotations.front();
				} else {
					//we have no tracked data for this joint
					//we may want to look to the previous frame for data, or use the junk coming from the kinect
				}
				localJoint.connectedTo = joint.second.connectedTo;
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
		CombinedUserSet::NodeUserIndex::NodeUserIndex(int nodeIndex, int userIndex) :
			nodeIndex(nodeIndex), userIndex(userIndex) {

		}

		//----------
		bool CombinedUserSet::NodeUserIndex::operator<(const NodeUserIndex & other) const {
			if (this->nodeIndex != other.nodeIndex) {
				return this->nodeIndex < other.nodeIndex;
			} else {
				return this->userIndex < other.userIndex;
			}
		}

		//----------
		void CombinedUserSet::addSourceMapping(const SourceMapping & sourceMapping) { 
			this->sourceUserMappings.push_back(sourceMapping);
		}

		//----------
		const vector<CombinedUserSet::SourceMapping> & CombinedUserSet::getSourceMappings() const {
			return this->sourceUserMappings;
		}
	}
}