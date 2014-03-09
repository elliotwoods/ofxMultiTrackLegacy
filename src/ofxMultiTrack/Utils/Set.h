#pragma once
#include "ofxJSON\libs\jsoncpp\include\json\json.h"
#include <memory>
#include <vector>
#include <string>

namespace ofxMultiTrack {
	namespace Utils {
		template<typename C>
		class Set : public std::vector<std::shared_ptr<C> > {
		public:
			template<class T>
			std::shared_ptr<T> get() {
				for(auto module : *this) {
					auto cast = std::dynamic_pointer_cast<T>(module);
					if(cast != NULL) {
						return cast;
					}
				}
				ofLogError("ofxMultiTrack") << "Module/Device of type [" << T().getType() << "] could not be found";
				return shared_ptr<T>();
			}

			std::shared_ptr<C> get(const std::string & type) {
				for(auto module : *this) {
					if(module->getType() == type) {
						return module;
					}
				}
				ofLogError("ofxMultiTrack") << "Module/Device of type [" << type << "] could not be found";
				return shared_ptr<C>();
			}

			void setConfig(const Json::Value & json) {
				const auto moduleNames = json.getMemberNames();
				for(auto moduleName : moduleNames) {
					auto module = this->get(moduleName);
					if (module) {
						module->setConfig(json[moduleName]);
					}
				}
			}
		};
	}
}