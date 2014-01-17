#include "Types.h"
#include "ofxCvGui2/src/ofxCvGui/Assets.h"

namespace ofxMultiTrack {
	//----------
	Json::Value Joint::serialise() const {
		Json::Value json;
		for(int i=0; i<3; i++) {
			json["position"][i] = this->position[i];
		}
		for(int i=0; i<4; i++) {
			json["rotation"][i] = this->rotation[i];
		}
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
	void User::customDraw() {
		ofSetDrawBitmapMode(OF_BITMAPMODE_MODEL);
		for(auto & joint : *this) {
			ofPushMatrix();
			ofTranslate(joint.second.position);
			float rotation[4];
			joint.second.rotation.getRotate(rotation[0], rotation[1], rotation[2], rotation[3]);
			ofRotate(rotation[0], rotation[1], rotation[2], rotation[3]);
			ofDrawAxis(0.05f);
			ofScale(0.001f, 0.001f, 0.001f);
			ofxCvGui::AssetRegister.drawText(joint.first, 0.05, 0, "", true, 20.0f);
			ofPopMatrix();
		}
		ofSetDrawBitmapMode(OF_BITMAPMODE_SIMPLE);
	}

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
	void UserSet::customDraw() {
		for(auto & user : *this) {
			user.draw();
		}
	}
}