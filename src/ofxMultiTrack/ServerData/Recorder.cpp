#include "Recorder.h"

namespace ofxMultiTrack {
	namespace ServerData {
		//----------
		Recorder::Recorder(const NodeSet & nodes) : nodes(nodes) {
			this->state = Recorder::Waiting;
			this->startTime = 0;
			this->endTime = 0;
			this->playHead = 0;
		}

		//----------
		void Recorder::update() {
			if (this->playHead < this->getStartTime()) {
				this->playHead = this->getStartTime();
			}
			if (this->playHead > this->getEndTime()) {
				this->playHead = this->getEndTime();
			}

			if (this->isRecording()) {
				for(auto node : this->nodes) {
					node->getRecording().recordIncoming();
				}
			} else {
				for(auto node : this->nodes) {
					node->getRecording().clearIncoming();
				}
			}

			auto currentTime = ofGetElapsedTimeMicros();

			switch(this->state) {
			case Recorder::Recording:
				if (currentTime > this->endTime) {
					this->endTime = currentTime;
					this->outPoint = currentTime;
					this->playHead = currentTime;
				}
				break;
			case Recorder::Playing:
				if (this->getDuration() > 0) {
					this->playHead += (Timestamp) (ofGetLastFrameTime() * 1000000.0);
					if (this->playHead > this->outPoint || this->playHead < this->inPoint) {
						this->playHead = this->inPoint;
					}
				}
				break;
			}
		}

		//----------
		void Recorder::record() {
			//This is to reset the recording start time
			//But we might want to consider not doing this later (e.g. to avoid losing user data)
			//That would instead suggest having a local timespan, rather than using the app's time
			this->clear();
			this->state = Recorder::Recording;
		}

		//----------
		void Recorder::play() {
			if (this->hasData()) {
				this->state = Recorder::Playing;
			}
		}

		//----------
		void Recorder::stop() {
			this->state = Recorder::Waiting;
		}

		//----------
		Json::Value Recorder::serialise() const {
			Json::Value json;
			int nodeIndex = 0;
			for(auto node : this->nodes) {
				auto & nodeJson = json[nodeIndex++];
				auto & frames = node->getRecording().getFrames();
				for(auto & frame : frames) {
					auto frameJson = frame.second.serialise();
					nodeJson[ofToString(frame.first)] = frameJson;
				}
			}
			return json;
		}

		//----------
		void Recorder::deserialise(const Json::Value & json) {
			if (json.size() != this->nodes.size()) {
				string errorMsg = "Mismatch on deserialise : number of nodes connected [" + ofToString(this->nodes.size()) + "] does not equal number of nodes in file to load [" + ofToString(json.size()) + "]";
				throw(Exception(errorMsg.c_str()));
			}

			this->clear();

			int nodeIndex = 0;
			Timestamp newStart = std::numeric_limits<Timestamp>::max();
			Timestamp newEnd = std::numeric_limits<Timestamp>::min();
			bool anyFramesFound = false;

			cout << "Loading";

			for(auto node : this->nodes) {
				auto & nodeJson = json[nodeIndex++];
				auto & recording = node->getRecording();
				auto & frameSet = recording.getFrames();
				auto timestamps = nodeJson.getMemberNames();
				for(auto timestamp : timestamps) {
					Timestamp rawTimestamp = stoll(timestamp);
					auto frame = UserSet();
					frame.deserialise(nodeJson[timestamp]);
					frameSet[rawTimestamp] = frame;

					if (rawTimestamp < newStart) {
						newStart = rawTimestamp;
					}
					if (rawTimestamp > newEnd) {
						newEnd = rawTimestamp;
					}

					cout << ".";
					anyFramesFound = true;
				}
			}

			cout << endl;

			if (anyFramesFound) {
				this->startTime = newStart;
				this->endTime = newEnd;
				this->clearInOutPoints();
			}
		}

		//----------
		void Recorder::save(string filename) const {
			if (filename=="") {
				auto response = ofSystemSaveDialog("recording.json", "Save recording");
				if (!response.bSuccess) {
					ofLogWarning("ofxMultiTrack") << "No file selected for save";
					return;
				}
				filename = response.filePath;
			}

			auto json = this->serialise();

			Json::FastWriter writer;
			ofFile output;
			output.open(filename, ofFile::WriteOnly, false);
			output << writer.write(json);
		}

		//----------
		void Recorder::load(string filename) {
			if (filename=="") {
				auto response = ofSystemLoadDialog("Load recording");
				if (!response.bSuccess) {
					ofLogWarning("ofxMultiTrack") << "No file selected for load";
					return;
				}
				filename = response.filePath;
			}

			ofFile input;
			input.open(filename, ofFile::ReadOnly, false);
			string jsonRaw = input.readToBuffer().getText();

			try {
				Json::Reader reader;
				Json::Value json;
				reader.parse(jsonRaw, json);
				this->deserialise(json);
			} catch (Exception e) {
				ofSystemAlertDialog(e.what());
			}
		}

		//----------
		void Recorder::clear() {
			for(auto node : this->nodes) {
				node->getRecording().clear();
			}
			this->startTime = ofGetElapsedTimeMicros();
			this->endTime = this->startTime;
			this->clearInOutPoints();
		}
	
		//----------
		bool Recorder::hasData() const {
			return this->getDuration() > 0;
		}

		//----------
		Timestamp Recorder::getPlayHead() const {
			return this->playHead;
		}

		//----------
		void Recorder::setPlayHead(Timestamp playHead) {
			if (playHead < this->startTime) {
				this->playHead = this->startTime;
			} else if (playHead > this->endTime) {
				this->playHead = this->endTime;
			} else {
				this->playHead = playHead;
			}
		}

		//----------
		float Recorder::getPlayHeadNormalised() const {
			float duration = (float) this->getDuration();
			if (duration == 0.0f) {
				return 0.0f;
			} else {
				return (float) (this->playHead - this->startTime) / duration;
			}
		}

		//----------
		void Recorder::setPlayHeadNormalised(float normalised) {
			auto playHead = this->normalisedToTimestamp(normalised);
			this->setPlayHead(playHead);
		}

		//----------
		Timestamp Recorder::getStartTime() const {
			return this->startTime;
		}

		//----------
		Timestamp Recorder::getEndTime() const {
			return this->endTime;
		}

		//----------
		Timestamp Recorder::getDuration() const {
			return this->endTime - this->startTime;
		}

		//----------
		void Recorder::clearInOutPoints() {
			this->inPoint = this->startTime;
			this->outPoint = this->endTime;
		}

		//----------
		Timestamp Recorder::getInPoint() const {
			return this->inPoint;
		}

		//----------
		Timestamp Recorder::getOutPoint() const {
			return this->outPoint;
		}

		//----------
		void Recorder::setInPoint(Timestamp inPoint) {
			this->inPoint = inPoint;
		}

		//----------
		void Recorder::setOutPoint(Timestamp outPoint) {
			this->outPoint = outPoint;
		}

		//----------
		void Recorder::trim() {
			this->startTime = this->inPoint;
			this->endTime = this->outPoint;

			//erase out of range frames from nodes
			for(auto node : this->nodes) {
				auto & frames = node->getRecording().getFrames();
				vector<Timestamp> framesToRemove;
				for(auto frame : frames) {
					if (frame.first < this->startTime || frame.first > this->endTime) {
						framesToRemove.push_back(frame.first);
					}
				}
				for(auto timestamp : framesToRemove) {
					frames.erase(frames.find(timestamp));
				}
			}
		}
		
		//----------
		Timestamp Recorder::appToTimeline(Timestamp timestamp) const {
			return timestamp - this->startTime;
		}

		//----------
		Timestamp Recorder::timelineToApp(Timestamp timestamp) const {
			return timestamp + this->startTime;
		}

		//----------
		float Recorder::timelineToNormalised(Timestamp timestamp) const {
			return float(timestamp - this->startTime) / (float) this->getDuration();
		}

		//----------
		Timestamp Recorder::normalisedToTimestamp(float normalised) const {
			auto duration = (float) this->getDuration();
			return duration * normalised + this->startTime;
		}

		//----------
		string Recorder::toString(Timestamp timestamp) {
			long long millis = timestamp / 1000LL;
			long long secs = millis / 1000;
			long long mins = secs / 60;

			string millisString = ofToString( (int) millis % 1000);
			string secsString = ofToString( ( (int) secs ) % 60);
			string minsString = ofToString( (int) mins );
			
			while(millisString.size() < 3) {
				millisString = "0" + millisString;
			}
			while(secsString.size() < 2) {
				secsString = "0" + secsString;
			}
			while(minsString.size() < 4) {
				minsString = " " + minsString;
			}

			return minsString + ":" + secsString + "." + millisString;
		}
	}
}