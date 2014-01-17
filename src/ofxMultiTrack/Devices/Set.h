#pragma once

#include "Base.h"

namespace ofxMultiTrack {
	namespace Devices {
		class Set : public vector<shared_ptr<Base> > {
		public:
			template<class T>
			void add(T* device) {
				this->push_back(shared_ptr<T>(device));
			}

			template<class T>
			shared_ptr<T> get() {
				for(auto device : *this) {
					auto cast = dynamic_pointer_cast<T>(device);
					if(cast != NULL) {
						return cast;
					}
				}
				string error = "Device of type [" + T().getType() + "] could not be found";
				throw(std::exception(error.c_str()));
			}
		};
	}
}