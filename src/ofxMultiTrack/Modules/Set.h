#pragma once

#include "Base.h"

namespace ofxMultiTrack {
	namespace Modules {
		class Set : public vector<shared_ptr<Base> > {
		public:
			template<class T>
			shared_ptr<T> get() {
				for(auto module : *this) {
					auto cast = dynamic_pointer_cast<T>(module);
					if(cast != NULL) {
						return cast;
					}
				}
				ofLogError("ofxMultiTrack") << "Module of type [" << T().getType() << "] could not be found";
				return shared_ptr<T>();
			}
		};
	}
}