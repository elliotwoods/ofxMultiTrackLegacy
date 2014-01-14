#pragma once

#include "Base.h"

namespace ofxMultiTrack {
	namespace Modules {
		class Set : public vector<ofPtr<Base> > {
		public:
			template<class T>
			void add(T* module) {
				this->push_back(ofPtr<T>(module));
			}
		};
	}
}