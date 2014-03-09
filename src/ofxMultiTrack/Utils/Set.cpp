#pragma once
#include "Set.h"

using namespace std;

namespace ofxMultiTrack {
	namespace Utils {
		////----------
		//template<typename C>
		//std::shared_ptr<C> Set::get(const string & type) {
		//	for(auto module : *this) {
		//		if(module->getType() == type) {
		//			return module;
		//		}
		//	}
		//	ofLogError("ofxMultiTrack") << "Module/Device of type [" << type << "] could not be found";
		//	return shared_ptr<C>();
		//}

		////----------
		//template<typename C>
		//void Set::setConfig(string moduleName, const Json::Value & json) {
		//	/*for(auto moduleNames = json.getMemberNames()) {
		//		auto module = this->get(moduleName);
		//		if (module) {
		//			module->setConfig(json[moduleName]);
		//		}
		//	}*/
		//}
	}
}