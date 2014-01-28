#pragma once

#include "Base.h"

namespace ofxMultiTrack {
	namespace Devices {
		class Set : public vector<shared_ptr<Base> > {
		public:
			template<class T>
			shared_ptr<T> get() {
				for(auto device : *this) {
					auto cast = dynamic_pointer_cast<T>(device);
					if(cast != NULL) {
						return cast;
					}
				}
				ofLogError("ofxMultiTrack") << "Device of type [" << T().getType() << "] could not be found";
				return shared_ptr<T>();
			}
		};
	}
}