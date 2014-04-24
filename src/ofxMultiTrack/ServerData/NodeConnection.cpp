#include "NodeConnection.h"
#include "../Align/Factory.h"
#include "../Utils/Constants.h"
#include "../Utils/Config.h"

#include <chrono>
#include <thread>

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
			this->disableSaving = false;

			this->tiltParameter.set("Tilt [degrees]", 0, NUI_CAMERA_ELEVATION_MINIMUM, NUI_CAMERA_ELEVATION_MAXIMUM);
			this->tiltParameter.addListener(this, & NodeConnection::onTiltParameterChange);
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

			//check for ciruclar influence referencing
			try {
				this->getInfluenceList();
			} catch (...) {
				this->clearTransform();
			}

			const auto align = this->transform.getTransform();
			ofMatrix4x4 matrixTransform;
			if (align) {
				matrixTransform = align->getMatrixTransform();
			}
			Json::Value json;
			auto & jsonMesh = json["modules"]["Mesh"]["transform"];
			for(int i=0; i<16; i++) {
				jsonMesh[i] = matrixTransform.getPtr()[i];
			}
			this->addNodeConfig(json);
		}

		//----------
		void NodeConnection::clearTransform() {
			this->setTransform(Transform());
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
		list<int> NodeConnection::getInfluenceList(int circularInfluenceCheck) const {
			list<int> influence;
			const auto ourParent = this->transform.getParent();
			if (ourParent == circularInfluenceCheck) {
				throw(ofxMultiTrack::Exception("Circular influence list found"));
			}
			if (ourParent != -1) {
				//if we're not root, add our parent
				influence.push_back(ourParent);
				//and ask our parent
				auto upstreamInfluences = otherNodes[this->transform.getParent()]->getInfluenceList(this->getIndex());
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
		//helper
		void merge(Json::Value & base, const Json::Value & addition) {
			if (addition.isObject()) {
				const auto memberNames = addition.getMemberNames();
				for(auto memberName : memberNames) {
					merge(base[memberName], addition[memberName]);
				}
			} else {
				base = addition;
			}
		}

		//----------
		void NodeConnection::addNodeConfig(const Json::Value & message) {
			this->send(message);
			merge(this->remoteConfig, message);
			this->saveConfig();
		}

		//----------
		void NodeConnection::send(const Json::Value & value) {
			this->toSendMutex.lock();
			this->toSend[this->toSend.size()] = value;
			this->toSendMutex.unlock();
		}

		//----------
		void NodeConnection::saveConfig(string filename) const {
			if (this->disableSaving) {
				return;
			}

			if (filename == "") {
				filename = this->getDefaultConfigFilename();
			}
			
			Json::StyledWriter writer;
			ofFile output;
			output.open(filename, ofFile::WriteOnly, false);
			output << writer.write(this->remoteConfig);
		}

		//----------
		void NodeConnection::loadConfig(string filename) {
			if (filename == "") {
				filename = this->getDefaultConfigFilename();
			}
			
			this->disableSaving = true;

			try {
				ofFile input;
				input.open(ofToDataPath(filename, true), ofFile::ReadOnly, false);
				string jsonRaw = input.readToBuffer().getText();

				Json::Reader reader;
				Json::Value json;
				reader.parse(jsonRaw, json);
				this->remoteConfig = json;
				this->send(json);
			} catch (std::exception e) {
				ofSystemAlertDialog(e.what());
			}

			auto newConfig = this->remoteConfig;
			const auto tiltConfig = newConfig["devices"]["KinectSDK"]["tilt"];
			if (!tiltConfig.empty()) {
				this->tiltParameter.set(tiltConfig.asFloat());
			}

			this->disableSaving = false;
		}

		//----------
		string NodeConnection::getDefaultConfigFilename() const {
			return "nodeRemoteConfig" + ofToString(this->getIndex()) + ".json";
		}

		//----------
		string NodeConnection::getName() const {
			return this->name;
		}

		//----------
		void NodeConnection::setName(string name) {
			this->name = name;
		}

		//----------
		ofParameter<float> & NodeConnection::getTiltParameter() {
			return this->tiltParameter;
		}

		//----------
		void NodeConnection::threadedFunction() {
			ofSetLogLevel("ofxNetwork", OF_LOG_SILENT);
			ofSetLogLevel("ofxTCPClient", OF_LOG_SILENT);
			while(this->running) {
				if (!this->client.isConnected()) {
					this->client.setup(this->address, OFXMULTITRACK_NODE_LISTEN_PORT + this->remoteIndex, false);
					this->send(this->remoteConfig);
				}
				
				this->receiveMessages();
				this->sendMessages();

				this->cachedConnected = this->client.isConnected();

				while(actionQueue.size() > 0) {
					lockActionQueue.lock();
					auto function = actionQueue.front();
					function();
					actionQueue.pop_front();
					lockActionQueue.unlock();
				}

				auto sleepTimeMicros = (long long) Utils::config->getValue<int>("Node thread sleep time [us]");
				if (sleepTimeMicros > 0) {
					std::this_thread::sleep_for(std::chrono::microseconds(sleepTimeMicros));
				}
			}
			this->threadEnded = true;
		}

		//----------
		void NodeConnection::receiveMessages() {
			auto response = this->client.receive();
			while (response.size() > 0) {
				try
				{
					Json::Value json;
					jsonReader.parse(response, json);
					
					if (!json["modules"]) {
						cout << "bsdobh";
						throw(Exception("No modules found"));
					}
					if (!json["modules"]["Skeleton"]) {
						throw(Exception("No modules::Skeleton found"));
					}
					if (!json["modules"]["Skeleton"]["data"]) {
						throw(Exception("No modules::Skeleton::data found"));
					}
					
					auto & jsonSkeletons = json["modules"]["Skeleton"]["data"];
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
					ofLogError("ofxMultiTrack") << "NodeConnection::receiveMessages : " << e.what();
				}
				lockUsers.unlock();
				response = client.receive();
			}
		}

		//----------
		void NodeConnection::sendMessages() {
			this->toSendMutex.lock();
			if (!this->toSend.empty()) {
				Json::Value send;
				Json::FastWriter writer;
				const auto toSendString = writer.write(this->toSend);
				this->toSend.clear();
				client.send(toSendString);
			}
			this->toSendMutex.unlock();
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

		//----------
		void NodeConnection::onTiltParameterChange(float & value) {
			Json::Value json;
			auto & jsonMessage = json["devices"]["KinectSDK"]["tilt"];
			jsonMessage = value;
			this->addNodeConfig(json);
		}
	}
}