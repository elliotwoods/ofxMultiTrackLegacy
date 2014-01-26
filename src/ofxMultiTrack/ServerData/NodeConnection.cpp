#include "NodeConnection.h"

namespace ofxMultiTrack {
	namespace ServerData {
		//----------
		NodeConnection::NodeConnection(string address, int index) {
			this->address = address;
			this->index = index;
			this->running = true;
			this->threadEnded = false;
			this->cachedConnected = false;
			this->startThread(true, false);
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
				if(!users[i].isAlive()) {
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
			status["index"] = this->index;
			status["connected"] = this->isConnected();
			status["users"] = this->getUserCount();

			this->remoteStatusLock.lock();
			status["remoteStatus"] = this->remoteStatus;
			this->remoteStatusLock.unlock();

			return status;
		}

		//----------
		void NodeConnection::threadedFunction() {
			this->client.setVerbose(false);
			while(this->running) {
				if (!this->client.isConnected()) {
					this->client.setup(this->address, OFXMULTITRACK_NODE_LISTEN_PORT + index, false);
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
									userLocal.setAlive(false);
								} else {
									userLocal.setAlive(true);
									auto jointNames = jsonUser.getMemberNames();
									for(auto & jointName : jointNames) {
										auto & joint = jsonUser[jointName];
										auto & jointLocal = userLocal[jointName];
										jointLocal.deserialise(joint);
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
					catch(Exception e)
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