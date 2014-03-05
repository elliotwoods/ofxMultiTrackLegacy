#include "User.h"
#include "Parameters.h"
#include "../Utils/Constants.h"
#include "../../ofxAssets/src/ofxAssets.h"
#include <numeric>
#include <set>

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
		User::GlobalIndex User::nextGlobalIndex = 0;

		//----------
		User::User() {
			this->alive = true;
			this->globalIndex = -1;
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

			bool allDead = true;

			for(auto & joint : accumulation) {
				const auto & trackedPositions = joint.second.trackedPositions;
				const auto & inferredPositions = joint.second.inferredPositions;
				auto & localJoint = (*this)[joint.first];
				bool noJoints = false;
				if (!trackedPositions.empty()) {
					localJoint.position = std::accumulate(trackedPositions.begin(), trackedPositions.end(), ofVec3f()) / (float) joint.second.trackedPositions.size();
					localJoint.rotation = joint.second.trackedRotations.front();
				} else if (!joint.second.inferredPositions.empty()) {
					localJoint.position = std::accumulate(inferredPositions.begin(), inferredPositions.end(), ofVec3f()) / (float) joint.second.inferredPositions.size();
					localJoint.rotation = joint.second.inferredRotations.front();
				} else {
					//we have no tracked data for this joint
					//we may want to look to the previous frame for data, or use the junk coming from the kinect
					noJoints = true;
				}
				localJoint.connectedTo = joint.second.connectedTo;
				localJoint.tracked = ! noJoints;
				allDead &= noJoints;
			}

			this->alive = ! allDead;
			this->globalIndex = -1;
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
		bool User::hasTrackedJoints() const {
			for(auto joint : *this) {
				if(joint.second.tracked && joint.second.position.lengthSquared() > 0.0f) {
					return true;
				}
			}
			return false;
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
		void User::draw(bool enableColors) const {
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

			if (this->globalIndex != -1) {
				//--
				//draw globalIndex label
				//
				ofPushMatrix();

				bool onFoot = true;
				const auto foot = this->find(OFXMULTITRACK_SERVER_ORIGIN_REFERENCE_JOINT_FLOOR_LEFT);
				if (foot != this->end()) {
					const auto footPosition = foot->second.position;
					//if we're near the floor
					if  (abs(footPosition.y) < 1.0f) {
						//then put it on the floor under the head
						const auto head = this->find(OFXMULTITRACK_SERVER_ORIGIN_REFERENCE_JOINT_UP);
						const auto userPosition = (head != this->end()) ? head->second.position : footPosition;
						
						ofTranslate(userPosition * ofVec3f(1.0f, 0.0f, 1.0f));
						ofRotate(90, 1.0f, 0.0f, 0.0f);
						ofScale(2.0f, 2.0f, 2.0f);
						onFoot = false;
					} else {
						//otherwise attach to foot
						ofTranslate(footPosition);
					}
				} else if (!this->empty()) {
					//if there is no foot, then let's use first available joint
					ofTranslate(this->begin()->second.position);
				}

				float scaleFactor = 2.0f / ofGetCurrentViewport().height;
				ofScale(-scaleFactor, scaleFactor, scaleFactor);

				auto & font = ofxAssets::AssetRegister.getFont("ofxMultiTrack::swisop3", 36);
				auto message = ofToString(this->globalIndex);
				auto textBounds = font.getStringBoundingBox(message, 0, 0);
				auto rectBounds = textBounds;

				//fix for getStringBoundingBox not matching oF hacks
				rectBounds.y = 0.0f;

				//buffer the height by 10%
				float bufferHeight = rectBounds.height * 0.2f;
				rectBounds.y -= bufferHeight;
				rectBounds.x -= bufferHeight;
				rectBounds.height += bufferHeight * 2.0f;
				rectBounds.width += bufferHeight * 2.0f;

				if (!onFoot) {
					ofTranslate(-rectBounds.getCenter());
				}

				//draw background
				ofPath background;
				background.setFillColor(ofColor(0));
				background.setStrokeWidth(1.0f);
				background.setStrokeColor(ofGetStyle().color);
				background.rectRounded(rectBounds, bufferHeight);
				background.draw();

				//draw text
				ofTranslate(0.0f, 0.0f, -1.0f / 100.0f);
				font.drawString(message, 0, 0);

				//draw text on flip side
				if (onFoot) {
					ofTranslate(rectBounds.width - bufferHeight, 0.0f, +2.0f / 100.0f);
					ofScale(-1.0f, 1.0f, 1.0f);
				} else {
					ofTranslate(0.0f, textBounds.height, +2.0f / 100.0f);
					ofScale(1.0f, -1.0f, 1.0f);
				}
				font.drawString(message, 0, 0);

				ofPopMatrix();
				//--
			}
		}

		//----------
		void User::setGlobalIndex(GlobalIndex globalIndex) {
			this->globalIndex = globalIndex;
		}

		//----------
		void User::assignForEmptyGlobalIndex() {
			if (this->globalIndex == -1) {
				this->globalIndex = User::nextGlobalIndex++;
			}
		}

		//----------
		User::GlobalIndex User::getGlobalIndex() const {
			return this->globalIndex;
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
		void UserSet::draw(bool enableColors) const {
			for(auto & user : *this) {
				user.draw(enableColors);
			}
		}

#pragma mark CombinedUserSet::NodeUserIndex
		//----------
		CombinedUserSet::NodeUserIndex::NodeUserIndex() :
			nodeIndex(0), userIndex(0), valid(false) {

		}

		//----------
		CombinedUserSet::NodeUserIndex::NodeUserIndex(int nodeIndex, int userIndex) :
			nodeIndex(nodeIndex), userIndex(userIndex), valid(true) {

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
		bool CombinedUserSet::NodeUserIndex::operator==(const NodeUserIndex & other) const {
			return this->nodeIndex == other.nodeIndex && this->userIndex == other.userIndex;
		}

		//----------
		ostream& operator<<(ostream& os, const CombinedUserSet::NodeUserIndex& nodeUserIndex) {
			os << "[" << nodeUserIndex.nodeIndex << ":" << nodeUserIndex.userIndex << "]";
			return os;
		}

#pragma mark CombinedUserSet
		//----------
		void CombinedUserSet::addSourceMapping(const SourceMapping & sourceMapping) { 
			this->sourceUserMappings.push_back(sourceMapping);
		}

		//----------
		const vector<CombinedUserSet::SourceMapping> & CombinedUserSet::getSourceMappings() const {
			return this->sourceUserMappings;
		}

		//----------
		void CombinedUserSet::matchFromPreviousFrame(const CombinedUserSet & previousFrame) {
			//the intention of this function is to pick up any ID's from the previous frame

			map<int, int> availablePreviousUserIndices;
			{
				int index = 0;
				for(auto & user : previousFrame) {
					availablePreviousUserIndices.insert(pair<int, int>(user.getGlobalIndex(), index++));
				}
			}

//			{
//				cout << "Current : " << endl;
//				int i = 0;
//				for(auto user : *this) {
//					cout << "\t" << i << ": ";
//					const auto & sourceMappings = this->sourceUserMappings[i];
//					for(auto mapping : sourceMappings) {
//						cout << mapping.first << " ";
//					}
//					cout << endl;
//					i++;
//				}
//				cout << endl;
//			}
//			{
//				cout << "Previous : " << endl;
//				int i = 0;
//				for(auto user : *this) {
//					cout << "\t" << i << ": ";
//					const auto & sourceMappings = this->sourceUserMappings[i];
//					for(auto mapping : sourceMappings) {
//						cout << mapping.first << " ";
//					}
//					cout << endl;
//					i++;
//				}
//				cout << endl;
//			}

			//check whether a user was seen in a previous frame
			// by checking to see if we share a NodeUserIndex with a user from
			// previous frame.
			int index = 0;
			for(auto & user : * this) {
				const auto & sourceMapping = this->sourceUserMappings[index];
				set<int> matchingPreviousUser;
				for(auto previousIndex : availablePreviousUserIndices) {
					const auto & previousUser = previousFrame[previousIndex.second];
					const auto & previousSourceMapping = previousFrame.getSourceMappings()[previousIndex.second];
					//now search for any matches
					for(auto nodeUserIndex : sourceMapping) {
						for(auto previousNodeUserIndex : previousSourceMapping) {
							if (nodeUserIndex.first == previousNodeUserIndex.first) {
								//we've found a user in the previous frame which maatches the user in this frame
								
								//if this global user index is unset, or is newer than the one we found
								if (user.getGlobalIndex() == -1 || user.getGlobalIndex() > previousUser.getGlobalIndex()) {
									user.setGlobalIndex(previousUser.getGlobalIndex());

									//don't allow any other of the current users to be able to match with this
									//previous user. also update our iterator
									matchingPreviousUser.insert(previousIndex.second);
//									cout << "Current " << index << ": " << nodeUserIndex.first << " <-> Previous " << previousIndex << ": " << previousNodeUserIndex.first << endl;
									goto weFoundAMatch; //break the double loop and skip the iterator increment
								}
							}
						}
					}
				}

				weFoundAMatch:
				for(auto previousIndexToRemove : matchingPreviousUser) {
					availablePreviousUserIndices.erase(previousIndexToRemove);
				}
				index++;
			}
//			cout << endl << endl;

			//Check if any global ID's are duplicated
			// This will be the case where 2 users are considered as combined in the previous frame
			// but in this frame they are considered as 2 separate users. We decide to work this way
			// to avoid accidental indefinite 'welding' of users.
			// The play off is of course that sometimes a user may split into 2 (e.g. bad calibration
			// or bad tracking). However, we can assume that means we would have 1 bad skeleton rather
			// than 1 good and 1 bad, so benefits don't seem worth the downsides.
			for(auto A = this->begin(); A != this->end(); A++) {
				auto B = A;
				B++;
				for(; B != this->end(); B++) {
					if (A->getGlobalIndex() == B->getGlobalIndex()) {
						B->setGlobalIndex(-1);
						B->assignForEmptyGlobalIndex();
					}
				}
			}
		}

		//----------
		void CombinedUserSet::assignForEmptyGlobalIndices() {
			for(auto & user : * this) {
				//internally, ask User to check if it already has a global index
				//and if not, assign itself a new one
				user.assignForEmptyGlobalIndex();
			}
		}
	}
}