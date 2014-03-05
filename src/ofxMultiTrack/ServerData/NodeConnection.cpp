#include "NodeConnection.h"
#include "../Align/Factory.h"

namespace ofxMultiTrack {
	namespace ServerData {
#pragma mark Transform
		//----------
		NodeConnection::Transform::Transform() : parent(-1) {
		}

		//----------
		NodeConnection::Transform::Transform(unsigned int parent, Align::Ptr transform) :
		parent(parent), transform(transform) {
		}

		//----------
		int NodeConnection::Transform::getParent() const {
			return this->parent;
		}

		//----------
		Align::Ptr NodeConnection::Transform::getTransform() const {
			return this->transform;
		}

#pragma mark NodeConnection
		//----------
		NodeConnection::NodeConnection(string address, int remoteIndex, NodeConnection::Collection & otherNodes) :
		otherNodes(otherNodes) {
			this->address = address;
			this->remoteIndex = remoteIndex;
			this->running = true;
			this->threadEnded = false;
			this->cachedConnected = false;
			this->startThread(true, false);
			this->enabled = true;
		}

		//----------
		NodeConnection::~NodeConnection() {
			this->running = false;
			this->stopThread();
			while(!this->threadEnded) {
				ofSleepMillis(1);
			}
		}
	
		//----------
		bool NodeConnection::isConnected() {
			return this->cachedConnected;
		}

		//----------
		int NodeConnection::getUserCount() {
			this->lockUsers.lock();
			int count = this->users.size();
			this->lockUsers.unlock();
			return count;
		}

		//----------
		void NodeConnection::clearUsers() {
			this->lockUsers.lock();
			this->users.clear();
			this->lockUsers.unlock();
		}

		//----------
		UserSet NodeConnection::getLiveData() {
			this->lockUsers.lock();
			auto users = this->users;
			this->lockUsers.unlock();

			//remove dead users from set to send
			UserSet::size_type i = 0;
			while (i < users.size()) {
				if(!users[i].isAlive() || !users[i].hasTrackedJoints()) {
					users.erase(users.begin() + i);
				} else {
					i++;
				}
			}
			return users;
		}

		//----------
		Recording & NodeConnection::getRecording() {
			return this->recording;
		}

		//----------
		Json::Value NodeConnection::getStatus() {
			Json::Value status;
			status["address"] = this->address;
			status["index"] = this->remoteIndex;
			status["connected"] = this->isConnected();
			status["users"] = this->getUserCount();

			this->remoteStatusLock.lock();
			status["remoteStatus"] = this->remoteStatus;
			this->remoteStatusLock.unlock();

			return status;
		}

		//----------
		int NodeConnection::getIndex() const {
			int i=0; 
			for(auto node : this->otherNodes) {
				if (node.get() == this) {
					return i;
				}
				i++;
			}
			return -1;
		}

		//----------
		const NodeConnection::Transform & NodeConnection::getTransform() const {
			return this->transform;
		}

		//----------
		void NodeConnection::setTransform(const Transform & transform) {
			this->transform = transform;
		}

		//----------
		void NodeConnection::clearTransform() {
			this->transform = Transform();
		}

		//----------
		void NodeConnection::applyTransform(UserSet & users) const {
			const auto ourTransform = this->transform.getTransform();
			const auto ourParent = this->transform.getParent();

			//--
			//apply our transform
			//
			//check it exists
			if (ourTransform) {
				for(auto & user : users) {
					for(auto & joint : user) {
						joint.second.position = ourTransform->applyTransform(joint.second.position);
					}
				}
			}
			//
			//--

			//--
			//apply upstream transforms
			//
			//check we're not a root branch
			if(ourParent == -1) {
				return;
			}
			//check our upstream transform exists in the set
			if(ourParent >= otherNodes.size()) {
				ofLogError("ofxMultiTrack") << "Cannot apply transform for node, as parent node does not exist";
				return;
			}
			//get the upstream node
			auto upstreamNode = this->otherNodes[ourParent];
			upstreamNode->applyTransform(users);
			//
			//--
		}

		//----------
		ofVec3f NodeConnection::applyTransform(const ofVec3f & xyz) const {
			const auto ourTransform = this->transform.getTransform();
			const auto ourParent = this->transform.getParent();

			//--
			//apply our transform
			//
			auto xyzDash = xyz;
			if (ourTransform) { //check if we have one
				xyzDash = ourTransform->applyTransform(xyzDash);
			}
			//
			//--


			//--
			//apply upstream transforms
			//
			//check we're not a root branch
			if(ourParent == -1) {
				return xyzDash;
			}
			//check our upstream transform exists in the set
			if(ourParent >= otherNodes.size()) {
				ofLogError("ofxMultiTrack") << "Cannot apply upstream transforms for node, as parent node does not exist";
				return xyzDash;
			}
			//get the upstream node
			auto upstreamNode = this->otherNodes[ourParent];
			xyzDash = upstreamNode->applyTransform(xyzDash);
			//
			//--

			return xyzDash;
		}

		//----------
		void NodeConnection::applyOriginPose(const User & user) {
			const auto & head = user.at(OFXMULTITRACK_SERVER_ORIGIN_REFERENCE_JOINT_UP).position;
			const auto & rightHand = user.at(OFXMULTITRACK_SERVER_ORIGIN_REFERENCE_JOINT_RIGHT).position;
			const auto & leftFoot = user.at(OFXMULTITRACK_SERVER_ORIGIN_REFERENCE_JOINT_FLOOR_LEFT).position;
			const auto & rightFoot = user.at(OFXMULTITRACK_SERVER_ORIGIN_REFERENCE_JOINT_FLOOR_RIGHT).position;

			ofMatrix4x4 transform;

			//0. start with left foot as origin
			transform.postMultTranslate(-leftFoot);

			//1. rotate right foot to be (+x, 0, 0)
			// i.e. set ground plane
			ofQuaternion groundPlaneRotate;
			groundPlaneRotate.makeRotate((rightFoot * transform).getNormalized(), ofVec3f(1.0f, 0.0f, 0.0f));
			transform.postMultRotate(groundPlaneRotate);

			//2. rotate around x to make head (x, +y, 0);
			ofVec3f headDash = (head * transform);
			float upVectorRotateTheta = atan2f(headDash.z, headDash.y);
			ofQuaternion upVectorRotate;
			upVectorRotate.makeRotate((-upVectorRotateTheta) * RAD_TO_DEG, 1.0f, 0.0f, 0.0f);
			transform.postMultRotate(upVectorRotate);

			//3. translate head to be (0, +y, 0)
			headDash = head * transform;
			transform.postMultTranslate((-headDash) * ofVec3f(1.0f, 0.0f, 1.0f));

			//4. rotate right hand to be (+, y, 0)
			const ofVec3f rightHandDash = (rightHand * transform);
			float rightVectorRotateTheta = atan2(rightHandDash.z, rightHandDash.x);
			ofQuaternion rightVectorRotate;
			rightVectorRotate.makeRotate(rightVectorRotateTheta * RAD_TO_DEG, 0.0f, 1.0f, 0.0f);
			transform.postMultRotate(rightVectorRotate);

			//set our transform to be a calibrated as a root
			this->transform = Transform(-1, Align::Factory::make(transform));
		}

		//----------
		list<int> NodeConnection::getInfluenceList() const {
			list<int> influence;
			const auto ourParent = this->transform.getParent();
			if (ourParent != -1) {
				//if we're not root, add our parent
				influence.push_back(ourParent);
				//and ask our parent
				auto upstreamInfluences = otherNodes[this->transform.getParent()]->getInfluenceList();
				influence.insert(influence.end(), upstreamInfluences.begin(), upstreamInfluences.end());
			}
			return influence;
		}

		//----------
		bool NodeConnection::isRoot() const {
			return this->transform.getParent() == -1;
		}

		//----------
		bool NodeConnection::isEnabled() const {
			return this->enabled;
		}

		//----------
		void NodeConnection::setEnabled(bool enabled) {
			this->enabled = enabled;
		}

		//----------
		void NodeConnection::toggleEnabled() {
			this->enabled = ! this->enabled;
		}

		//----------
		void NodeConnection::threadedFunction() {
			ofSetLogLevel("ofxNetwork", OF_LOG_SILENT);
			ofSetLogLevel("ofxTCPClient", OF_LOG_SILENT);
			while(this->running) {
				if (!this->client.isConnected()) {
					this->client.setup(this->address, OFXMULTITRACK_NODE_LISTEN_PORT + this->remoteIndex, false);
				}
				auto response = this->client.receive();
				if (response.size() > 0) {
					try
					{
						Json::Value json;
						jsonReader.parse(response, json);
						auto & jsonSkeletons = json["modules"][0]["data"];
						bool newSkeleton = jsonSkeletons["isNewSkeleton"].asBool();
						auto & jsonUsers = jsonSkeletons["users"];
						this->lockUsers.lock();
						int userIndex = 0;
						if (this->users.size() < jsonUsers.size()) {
							this->users.resize(jsonUsers.size());
						}
						for(auto & userLocal : this->users) {
							if (userIndex >= jsonUsers.size() || jsonUsers.isNull()) {
								userLocal.setAlive(false);
							} else {
								auto & jsonUser = jsonUsers[userIndex];
								if (jsonUser.size() == 0) {
									userLocal.clear();
									userLocal.setAlive(false);
								} else {
									userLocal.setAlive(true);
									auto jointNames = jsonUser.getMemberNames();
									bool foundANonZero = false;
									for(auto & jointName : jointNames) {
										auto & joint = jsonUser[jointName];
										auto & jointLocal = userLocal[jointName];
										jointLocal.deserialise(joint);
										foundANonZero |= jointLocal.position.lengthSquared() > 0.0f;
									}
									if (!foundANonZero) {
										userLocal.setAlive(false);
									}
								}
							}
							userIndex++;
						}
						if (newSkeleton) {
							this->recording.addIncoming(this->users);
						}

						this->remoteStatusLock.lock();
						this->remoteStatus = json["status"];
						this->remoteStatusLock.unlock();

					}
					catch(std::exception e)
					{
						ofLogError("ofxMultiTrack") << e.what();
					}
					lockUsers.unlock();
				}

				this->cachedConnected = this->client.isConnected();

				while(actionQueue.size() > 0) {
					lockActionQueue.lock();
					auto function = actionQueue.front();
					function();
					actionQueue.pop_front();
					lockActionQueue.unlock();
				}
			}
			this->threadEnded = true;
		}

		//----------
		void NodeConnection::performBlocking(function<void()> action) {
			lockActionQueue.lock();
			actionQueue.push_back(action);
			lockActionQueue.unlock();

			bool waiting = true;
			while(waiting) {
				lockActionQueue.lock();
				waiting = !actionQueue.empty();
				lockActionQueue.unlock();	
			}
		}
	}
}