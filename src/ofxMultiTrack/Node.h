#include "ofMain.h"
#include "ofxNetwork/src/ofxNetwork.h"
#include "Constants.h"

#include "Types.h"

#include "Devices/Set.h"
#include "Devices/KinectSDK.h"

#include "Modules/Set.h"
#include "Modules/Skeleton.h"

namespace ofxMultiTrack {
	class Node {
	public:
		Node();
		bool init();
		void update();
	protected:
		ofxTCPServer server;
		NodeSettings settings;

		Devices::Set devices;
		Modules::Set modules;
	};
}