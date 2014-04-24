#include "Config.h"

namespace ofxMultiTrack {
	namespace Utils {
		//---------
		shared_ptr<Config> config = shared_ptr<Config>();

		//---------
		Config::Config() {
			auto nodeSleepParameter = this->setParameter<int>("Node thread sleep time [us]", 200);
			nodeSleepParameter->setMin(0);
			nodeSleepParameter->setMax(2000);
			
			this->setParameter<string>("calibrationJoint", string("HAND_RIGHT"));

			auto combineParameter = this->setParameter<float>("Combine distance threshold [m]", 0.2f);
			combineParameter->setMin(0.0f);
			combineParameter->setMax(1.0f);

			auto inferredWeightRampParameter = this->setParameter<float>("Inferred weight ramp time [s]", 5.0);
			inferredWeightRampParameter->setMin(0.0f);
			inferredWeightRampParameter->setMax(10.0f);

			this->setParameter<bool>("Draw source skeletons", true);
			this->setParameter<bool>("Draw combined skeletons", true);
		}

		//---------
		Config::~Config() {
			this->lock.lock();
			this->lock.unlock();
		}

		//---------
		template<typename T>
		static bool isParameterTypeName(string parameterTypeName) {
			return parameterTypeName == typeid(ofParameter<T>).name();
		}

		//---------
		void Config::load(const Json::Value & json) {
			for(auto & parameterName : json.getMemberNames()) {
				const auto jsonParameter = json[parameterName];
				
				const auto localParameter = this->parameters.find(parameterName);
				enum {
					TUndefined,
					TBool,
					TInt,
					TFloat,
					TString
				} parameterType = TUndefined;

				if (localParameter != this->parameters.end()) {
					//first we check the type against any already loaded parameters
					const auto parameterTypeName = localParameter->second->type();
					if (isParameterTypeName<bool>(parameterTypeName)) {
						parameterType = TBool;
					} else if (isParameterTypeName<int>(parameterTypeName)) {
						parameterType = TInt;
					} else if (isParameterTypeName<float>(parameterTypeName)) {
						parameterType = TFloat;
					} else if (isParameterTypeName<string>(parameterTypeName)) {
						parameterType = TString;
					} 
				} else {
					//otherwise we infer the type from the json
					// e.g. can cause int/float mismatches depending on how writting in json
					//	so always best initialise the param first in Config's constructor
					if (jsonParameter.isBool()) {
						parameterType = TBool;
					} else if (jsonParameter.isInt()) {
						parameterType = TInt;
					} else if (jsonParameter.isConvertibleTo(Json::ValueType::realValue)) {
						parameterType = TFloat;
					} else if (jsonParameter.isString()) {
						parameterType = TString;
					} 
				}

				if (parameterType == TBool) {
					this->setParameter<bool>(parameterName, jsonParameter.asBool());
				} else if (parameterType == TInt) {
					this->setParameter<int>(parameterName, jsonParameter.asInt());
				} else if (parameterType == TFloat) {
					this->setParameter<float>(parameterName, jsonParameter.asFloat());
				} else if (parameterType == TString) {
					this->setParameter<string>(parameterName, jsonParameter.asString());
				} 
			}
		}
	}
}