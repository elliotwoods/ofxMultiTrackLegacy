#pragma once

#include "Base.h"

namespace ofxMultiTrack {
	namespace Devices {
		class Set : public vector<ofPtr<Base> > {
		public:
			template<class T>
			void add(T* device) {
				this->push_back(ofPtr<T>(device));
			}

			ofPtr<Base> find(string deviceType);
		};
	}
}