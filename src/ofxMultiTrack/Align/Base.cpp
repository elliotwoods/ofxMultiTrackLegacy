#include "Base.h"
#include "ofLog.h"

namespace ofxMultiTrack {
	namespace Align {
		//---------
		const ofMatrix4x4 Base::getMatrixTransform() const {
			ofLogError("ofxMultiTrack") << "Can't call getTransform as transform type [" << this->getType() << "] does not support this";
			return ofMatrix4x4();
		}
	}
}