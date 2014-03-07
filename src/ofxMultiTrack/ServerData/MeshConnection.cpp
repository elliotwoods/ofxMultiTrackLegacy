#include "MeshConnection.h"

namespace ofxMultiTrack {
	namespace ServerData {
		//----------
		MeshConnection::MeshConnection(string remoteAddress, int remoteIndex) {
			this->incomingBuffer.allocate(1024 * 32);
			this->startThread(true, false);
		}

		//----------
		void MeshConnection::threadedFunction() {
			while(this->isThreadRunning()) {
				auto size = this->client.receiveRawBytes(this->incomingBuffer.getBinaryBuffer(), this->incomingBuffer.size());
				if (size > 0) {
					cout << "receiving " << size << " bytes. " << endl;
				}
			}
		}
	}
}