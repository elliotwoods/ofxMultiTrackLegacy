#pragma once

#include "ofParameter.h"
#include "ofxJSON/libs/jsoncpp/include/json/json.h"

#include <map>
#include <string>
#include <typeinfo>

namespace ofxMultiTrack {
	namespace Utils {
		class Config {
		public:
			Config();
			~Config();

			template<typename T>
			void setParameter(const string name, T value) {
				this->lock.lock();
				auto findParameter = this->parameters.find(name);
				if (findParameter == this->parameters.end()) {
					this->parameters[name] = shared_ptr<ofParameter<T> >(new ofParameter<T>(name, value));
				} else {
					auto parameter = this->getParameter<T>(name);
					if (parameter) {
						parameter->set(value);
					} else {
						ofLogError("ofxMultiTrack") << "Parameter " << name << " is of wrong type, doesn't match " << typeid(T).name();
					}
				}
				this->lock.unlock();
			}

			template<typename T>
			void setParameter(const string name, T value, T minimum, T maximum) {
				this->lock.lock();
				this->parameters[name] = shared_ptr<ofParameter<T> >(new ofParameter<T>(name, value, minimum, maximum));
				this->lock.unlock();
			}

			template<typename T>
			shared_ptr<ofParameter<T> > getParameter(const string name) {
				this->lock.lock();
				auto findParameter = this->parameters.find(name);
				if (findParameter == this->parameters.end()) {
					ofLogError("ofxMultiTrack") << "Config parameter " << name << " not found!";
					this->lock.unlock();
					return shared_ptr<ofParameter<T> >();
				} else {
					this->lock.unlock();
					return dynamic_pointer_cast<ofParameter<T> >(findParameter->second);
				}
			}

			template<typename T>
			T getValue(const string name) {
				auto parameter = this->getParameter<T>(name);
				if (!parameter) {
					ofLogError("ofxMultiTrack") << "Config parameter " << name << " is not of type " << typeid(T).name();
					return T();
				}

				return parameter->get();
			}

			void load(const Json::Value &);
		protected:
			ofMutex lock;
			map<string, shared_ptr<ofAbstractParameter> > parameters;
		};

		extern shared_ptr<Config> config;
	}
}
