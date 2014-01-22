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
					this->playHead = currentTime;
				}
				break;
			case Recorder::Playing:
				if (this->getDuration() > 0) {
					this->playHead += (Timestamp) (ofGetLastFrameTime() * 1000000.0);
					if (this->playHead > this->endTime) {
						this->playHead = this->startTime;
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
				string errorMsg = "Mismatch on deserialise : number of nodes connected [" + ofToString(this->nodes.size()) + "] does not equal number of nodes in json [" + ofToString(json.size()) + "]";
				throw(std::exception(errorMsg.c_str()));
			}
			int nodeIndex = 0;
			Timestamp newStart = std::numeric_limits<Timestamp>::max();
			Timestamp newEnd = std::numeric_limits<Timestamp>::min();
			bool anyFramesFound = false;

			for(auto node : this->nodes) {
				auto & nodeJson = json[nodeIndex++];
				auto & recording = node->getRecording();
				recording.clear();
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

					anyFramesFound = true;
				}
			}

			if (anyFramesFound) {
				this->startTime = newStart;
				this->endTime = newEnd;
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
			} catch (std::exception e) {
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
			auto duration = (float) this->getDuration();
			this->setPlayHead(duration * normalised + this->startTime);
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
	}
}