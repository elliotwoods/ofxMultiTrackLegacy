#include "RecordingControl.h"
#include "ofxMultiTrack/src/ofxMultiTrack/Align/RigidBodyFit.h"

using namespace ofxMultiTrack;
using namespace ofxCvGui;

//---------
RecordingControl::RecordingControl(ServerData::Recorder & recorder, ServerData::Recording & recording, ServerData::NodeConnection::Ptr node, const ServerData::NodeSet & nodes)
	: recorder(recorder), recording(recording), node(node), nodes(nodes) {
	this->setBounds(ofRectangle(0,0,100,30));

	this->onDraw += [this] (DrawArguments & args) { this->draw(args); };
	this->onUpdate += [this] (UpdateArguments & args) { this->update(args); };
	this->onBoundsChange += [this] (BoundsChangeArguments & args) { this->boundsChange(args); };
	this->onMouseReleased += [this] (MouseArguments &) { 
		ofxCvGui::Panels::Inspector::select(*this);
	};
	this->trackDirty = false;
	this->cachedCount = 0;
	this->rebuildInspector = false;

	this->enabledParameter.set("Enabled", true);
}

//---------
ofxMultiTrack::ServerData::NodeConnection::Ptr RecordingControl::getNode() {
	return this->node;
}

//---------
void RecordingControl::update(UpdateArguments &) {
	this->enabledParameter.set(this->node->isEnabled());

	this->status = this->node->getStatus();

	if(this->cachedCount != this->recording.getFrames().size() || this->recorder.isRecording()) {
		this->trackDirty = true;
	}

	if (trackDirty) {
		auto startTime = (float) this->recorder.getStartTime();
		auto endTime = (float) this->recorder.getEndTime();
		auto timeScale = endTime - startTime;
	
		auto & frames = this->recording.getFrames();
		ofMesh markers;
		for(auto & frame : frames) {
			for(int i=0; i<frame.second.size(); i++) {
				if (frame.second[i].size() > 0) {
					markers.addVertex(ofVec3f(frame.first, i, 0));
				}
			}
		}

		this->fbo.begin();
		ofPushMatrix();
		ofClear(0, 0);
		ofScale(fbo.getWidth() / (float) timeScale, fbo.getHeight() / 6.0f, 1.0f);
		ofTranslate(-startTime, 0, 0);
		markers.drawVertices();
		ofPopMatrix();
		this->fbo.end();

		this->cachedCount = this->recording.getFrames().size();
		this->trackDirty = false;
	}

	if (this->rebuildInspector) {
		ofxCvGui::Panels::Inspector::refresh();
		this->rebuildInspector = false;
	}
}

//---------
void RecordingControl::draw(DrawArguments & args) {
	this->fbo.draw(args.localBounds);

	ofPushStyle();
	auto index = this->node->getIndex();
	auto notes = "[" + ofToString(this->node->getIndex()) + "] " + this->node->getName() + " :";
	
	auto influences = this->node->getInfluenceList();
	for(auto influence : influences) {
		notes += " <-- " + this->nodes[influence]->getName();
	}

	ofSetColor(100);
	ofDrawBitmapString(notes, 5, this->getHeight() - 5);

	ofSetLineWidth(1);
	ofLine(0,this->getHeight(), this->getWidth(), this->getHeight()); 

	if (ofxCvGui::Panels::Inspector::isSelected(*this)) {
		ofEnableAlphaBlending();
		ofSetColor(255,255,255,40);
		ofRect(args.localBounds);
	}

	if (this->node->isEnabled()) {
		ofFill();
	} else {
		ofNoFill();
	}

	ofCircle(9, 8, 4);

	//draw connection indicator
	ofFill();
	ofSetColor(this->node->isConnected() ? ofColor(100, 255, 100) : ofColor(255, 100, 100));
	ofCircle(23, 8, 4);

	ofPopStyle();
}

//---------
void RecordingControl::boundsChange(BoundsChangeArguments & args) {
	this->fbo.allocate(this->getWidth(), this->getHeight(), GL_RGBA);
	this->trackDirty = true;
}

using namespace ofxCvGui::Widgets;

//---------
void RecordingControl::populate(ofxCvGui::ElementGroupPtr inspector) {
	auto header = shared_ptr<Title>(new Title("Node #" + ofToString(this->node->getIndex()) + ": " + this->node->getName(), Title::Level::H2));
	
	auto addressValue = shared_ptr<LiveValue<string> >(new LiveValue<string>("Address", [this] () {
		return this->status["address"].asString();
	}));

	auto deviceIndexValue = shared_ptr<LiveValue<int> >(new LiveValue<int>("Device Index", [this] () {
		return this->status["remoteStatus"]["localIndex"].asInt();
	}));
	
	auto timestampValue = shared_ptr<LiveValue<unsigned long long> >(new LiveValue<unsigned long long>("Timestamp", [this] () {
		return this->status["remoteStatus"]["timestamp"].asLargestUInt();
	}));

	auto absoluteTimeOffsetValue = shared_ptr<LiveValueHistory>(new LiveValueHistory("Absolute time offset [ms]", [this] () {
		return this->node->getAbsoluteTimeOffset();
	}));

	auto connectedValue = shared_ptr<LiveValue<string> >(new LiveValue<string>("Connected", [this] () {
		return this->status["connected"].asBool() ? "true" : "false";
	}));

	auto enabledToggle = shared_ptr<Toggle>(new Toggle(this->enabledParameter));
	enabledToggle->onValueChange += [this] (const ofParameter<bool> & value) {
		this->node->setEnabled(value.get());
	};

	auto fpsValue = shared_ptr<LiveValueHistory>(new LiveValueHistory("Remote fps", [this] () {
		return this->status["remoteStatus"]["fps"].asFloat();
	}, true));

	auto rxpsValue = shared_ptr<LiveValueHistory>(new LiveValueHistory("Receives per second [Hz]", [this] () {
		auto interval = (float) this->node->getLastRxInterval() / (float) 1e3;
		if (interval < ofGetLastFrameTime() * 5.0f) {
			return 1.0f / interval;
		} else {
			return 0.0f;
		}
	}, true));

	auto tiltSlider = shared_ptr<Slider>(new Slider(this->node->getTiltParameter()));

	inspector->add(shared_ptr<Spacer>(new Spacer()));
	inspector->add(header);
	inspector->add(addressValue);
	inspector->add(deviceIndexValue);
	inspector->add(enabledToggle);
	inspector->add(timestampValue);
	inspector->add(absoluteTimeOffsetValue);
	inspector->add(connectedValue);
	inspector->add(fpsValue);
	inspector->add(rxpsValue);
	inspector->add(tiltSlider);

	inspector->add(shared_ptr<Spacer>(new Spacer()));
	inspector->add(shared_ptr<Title>(new Title("Transform", Title::Level::H3)));
	
	auto transformParentValue = shared_ptr<LiveValue<string>>(new LiveValue<string>("Parent", [this] () {
		auto parent = this->node->getTransform().getParent();
		if (parent == -1) {
			return string("none");
		} else {
			return ofToString(parent);
		}
	}));
	inspector->add(transformParentValue);

	auto transformTypeValue = shared_ptr<LiveValue<string>>(new LiveValue<string>("Type", [this] () {
		auto align = this->node->getTransform().getTransform();
		if (align) {
			return align->getType();
		} else {
			return string("none");
		}
	}));
	inspector->add(transformTypeValue);
	
	auto nodeAlignBase = this->node->getTransform().getTransform();
	auto nodeAlignRigidTransform = dynamic_pointer_cast<Align::RigidBodyFit>(nodeAlignBase);
	if (nodeAlignRigidTransform) {
		const auto transformParameters = nodeAlignRigidTransform->getParameters();
		for(auto parameter : transformParameters) {
			auto parameterSlider = shared_ptr<Slider>(new Slider(* parameter));
			inspector->add(parameterSlider);
		}
	}

	inspector->add(shared_ptr<Widgets::Button>(new Widgets::Button("Clear transform", [this] () {
		this->node->clearTransform();
		this->rebuildInspector = true;
	})));

	inspector->add(shared_ptr<Widgets::Button>(new Widgets::Button("Make blank transform", [this] () {
		this->node->setTransform(ServerData::NodeConnection::Transform(-1, Align::Factory::makeDefault()));
		this->rebuildInspector = true;
	})));
}