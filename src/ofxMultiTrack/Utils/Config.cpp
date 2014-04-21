#include "Config.h"

namespace ofxMultiTrack {
	namespace Utils {
		//---------
		Config config = Config();

		//---------
		Config::Config() {
			this->setParameter<int>("nodeSleepTimeMicros", 200, 0, 2000);
			this->setParameter<string>("calibrationJoint", string("HAND_RIGHT"));
			this->setParameter<float>("combineDistanceThreshold", 0.2f);
		}

		//---------
		Config::~Config() {
		}

		//---------
		void Config::load(const Json::Value & json) {
			for(auto & parameterName : json.getMemberNames()) {
				const auto parameter = json[parameterName];
				if (parameter.isBool()) {
					this->setParameter<bool>(parameterName, parameter.asBool());
				} else if (parameter.isInt()) {
					this->setParameter<int>(parameterName, parameter.asInt());
				} else if (parameter.isConvertibleTo(Json::ValueType::realValue)) {
					this->setParameter<float>(parameterName, parameter.asFloat());
				} else if (parameter.isString()) {
					this->setParameter<string>(parameterName, parameter.asString());
				} 
			}
		}
	}
}