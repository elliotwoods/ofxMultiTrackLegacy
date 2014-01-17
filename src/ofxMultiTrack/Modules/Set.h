#pragma once

#include "Base.h"

namespace ofxMultiTrack {
	namespace Modules {
		class Set : public vector<shared_ptr<Base> > {
		public:
			template<class T>
			void add(T* module) {
				this->push_back(shared_ptr<T>(module));
			}
		};
	}
}