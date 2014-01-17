#include "Types.h"
#include "ofxCvGui2/src/ofxCvGui/Assets.h"

namespace ofxMultiTrack {
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
	void UserSet::customDraw() {
		for(auto & user : *this) {
			user.draw();
		}
	}
}