#pragma once

#include "../Utils/Constants.h"
#include "ofxNetwork/src/ofxNetwork.h"

namespace ofxMultiTrack {
	namespace ServerData {
		class MeshConnection : ofThread { 
		public:
			MeshConnection(string remoteAddress, int remoteIndex);
		protected:
			void threadedFunction() override;
			ofxTCPClient client;
			ofBuffer incomingBuffer;
		};
	}
}