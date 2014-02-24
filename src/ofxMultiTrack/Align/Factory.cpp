#include "Factory.h"

#include "RigidBodyFit.h"
#include "PolyFit.h"

namespace ofxMultiTrack {
	namespace Align {
		//----------
		Align::Ptr Factory::makeDefault() {
			return Align::Ptr(new Align::RigidBodyFit());
		}

		//----------
		template<class T>
		bool checkAndMake(string type, Align::Ptr & result) {
			//check if the type name matches the type class
			if (T().getType() == type) {
				//if so, instantiate this class
				result = Align::Ptr(new T());
				return true;
			}
			//if not, then say it wasn't me
			return false;
		}

		//----------
		Align::Ptr Factory::make(const string type) {
			Align::Ptr align;
			if (checkAndMake<RigidBodyFit>(type, align)) {
				return align;
			}
			if (checkAndMake<PolyFit>(type, align)) {
				return align;
			}
			ofLogError("ofxMultiTrack::Align::Factory") << "Align type [" << type << "] is not available, making a default one instead";
			return makeDefault();
		}
	}
}