#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
	this->enableUpdate = true;
	this->enableTestPoint = false;

	this->gui.init();
	this->gui.add(this->kinect.getDepthTexture(), "Depth");
	auto output = this->gui.add(this->previewImage, "Preview Image");
	auto scroll = this->gui.addScroll();
	auto world = this->gui.addWorld();

	this->cannyThreshold.set("Canny threshold", 46, 0, 255);
	this->ksize.set("ksize", 3, 1, 32);
	this->dilateIterations.set("dilate iterations", 3, 0, 10);
	this->erodeIterations.set("erode iterations", 3, 0, 10);
	this->erodeDepth.set("erode depth", 1, 0, 32);
	this->areaThreshold.set("areaThreshold [normalised]", 0.005f, 0.0f, 1.0f);
	this->decimateContour.set("decimate contour", 2, 1, 64);

	auto intValidator = [] (float & value) {
		value = ceil(value + 0.5f);
	};

	auto addIntSlider = [=] (ofParameter<float> & parameter) {
		auto slider = shared_ptr<Widgets::Slider>(new Widgets::Slider(parameter));
		slider->setValidator(intValidator);
		scroll->add(slider);
	};

	addIntSlider(this->cannyThreshold);
	addIntSlider(this->ksize);
	addIntSlider(this->dilateIterations);
	addIntSlider(this->erodeIterations);
	scroll->add(shared_ptr<Widgets::Spacer>(new Widgets::Spacer()));
	addIntSlider(this->erodeDepth);
	scroll->add(shared_ptr<Widgets::Slider>(new Widgets::Slider(this->areaThreshold)));
	addIntSlider(this->decimateContour);

	output->onDrawCropped += [this] (Panels::BaseImage::DrawCroppedArguments & args) {
		ofPushStyle();
		ofSetLineWidth(4.0f);
		for(auto & contour : this->contours) {
			const auto area = cv::contourArea(contour);
			ofColor color(255, 0, 0, 100);
			color.setHue(((int) (area * 0.001f)) % 360);
			ofSetColor(color);
			ofBeginShape();
			for(auto & point : contour) {
				ofVertex(point.x, point.y);
			}
			ofEndShape();
		}
		ofPopStyle();
	};

	world->onDrawWorld += [this] (ofCamera &) {
		int contourIndex = 0;
		ofPushStyle();
		ofSetLineWidth(3.0f);
		for(auto & worldContour : this->worldContours) {
			ofColor color(255, 0, 0);
			const auto area = cv::contourArea(this->contours[contourIndex]);
			color.setHue(((int) (area * 0.001f)) % 360);
			ofSetColor(color);
			worldContour.drawWireframe();
			contourIndex++;
		}

		if (this->enableTestPoint) {
			ofMesh preview;
			for(auto point : testPoints) {
				preview.addVertex(point);
			}
			preview.drawVertices();
		}

		ofPopStyle();
	};

	this->kinect.initSensor();
	this->kinect.initDepthStream(640, 480);
	this->kinect.start();
	this->sensor = & this->kinect.getNuiSensor();
}

ofVec3f depthToWorld(int x, int y, const ofShortPixels & depthData, INuiCoordinateMapper * mapper) {
	NUI_DEPTH_IMAGE_POINT depthPoint;
	depthPoint.x = x;
	depthPoint.y = y;
	depthPoint.depth = depthData[x + y * 640] << 1;
	Vector4 skeletonPoint;
	mapper->MapDepthPointToSkeletonPoint(NUI_IMAGE_RESOLUTION_640x480, & depthPoint, & skeletonPoint);
	return ofVec3f(skeletonPoint.x, skeletonPoint.y, skeletonPoint.z) / skeletonPoint.w;
}

//--------------------------------------------------------------
void ofApp::update(){
	if (this->enableUpdate) {
		this->kinect.update();
	}

	auto depthImage = ofxCv::toCv(this->kinect.getRawDepthPixelsRef());
	try {
		cv::Mat result;
		cv::Mat depthImage8;
		depthImage.convertTo(depthImage8, CV_8UC1, 0.25);
		cv::Canny(depthImage8, result, this->cannyThreshold, this->cannyThreshold * 3.0f, this->ksize);
		cv::dilate(result, result, cv::Mat(), cvPoint(-1,-1), this->dilateIterations);
		cv::erode(result, result, cv::Mat(), cvPoint(-1,-1), this->erodeIterations);
		result = depthImage8 - result;
		result = result > 0;
		cv::erode(result, result, cv::Mat(), cvPoint(-1,-1), this->erodeDepth);
		ofxCv::copy(result, previewImage);

		cv::findContours(result, this->contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE, cv::Point(0, 0) );
		
		//strip based on min area
		const auto toRemove = std::remove_if(this->contours.begin(), this->contours.end(), [this] (vector<cv::Point> & contour) {
			bool remove = cv::contourArea(contour) < (this->areaThreshold * 640.0f * 480.0f);
			return remove;
		});
		this->contours.erase(toRemove, this->contours.end());

		const auto depthData = this->kinect.getRawDepthPixelsRef();
		this->worldContours.clear();
		INuiCoordinateMapper * mapper;
		this->sensor->NuiGetCoordinateMapper(&mapper);

		if (this->enableTestPoint) {
			this->testPoints.clear();
			for(int i=0; i<640; i+=2) {
				for(int j=0; j<480; j+=2) {
					NUI_DEPTH_IMAGE_POINT depthPoint;
					depthPoint.x = i;
					depthPoint.y = j;
					depthPoint.depth = depthData.getPixels()[i + j * 640] << 1;
					Vector4 skeletonPoint;
					mapper->MapDepthPointToSkeletonPoint(NUI_IMAGE_RESOLUTION_640x480, & depthPoint, & skeletonPoint);
					const auto point = ofVec3f(skeletonPoint.x, skeletonPoint.y, skeletonPoint.z);
					if (point.lengthSquared() > 0.0f) {
						this->testPoints.push_back(point);
					}
				}
			}
		}

		for(auto contour : this->contours) {
			//decimate and reformat
			int iPoint = 0;
			vector<ofVec3f> depthPoints;
			for(auto & point : contour) {
				if (this->decimateContour > 1 ? (iPoint++ % (int) this->decimateContour == 0) : true) {
					depthPoints.push_back(ofVec3f(point.x, point.y, 0));
				}
			}

			if (depthPoints.size() < 3)	 {
				continue;
			}

			//triangulate
			ofxTriangle triangulate;
			triangulate.triangulate(depthPoints);

			//transform to skeleton space
			ofMesh meshInWorld;
			for(auto triangle : triangulate.triangles) {
				meshInWorld.addVertex(depthToWorld(triangle.a.x, triangle.a.y, depthData, mapper));
				meshInWorld.addVertex(depthToWorld(triangle.b.x, triangle.b.y, depthData, mapper));
				meshInWorld.addVertex(depthToWorld(triangle.c.x, triangle.c.y, depthData, mapper));
			}
			
			this->worldContours.push_back(meshInWorld);
		}
	} catch (cv::Exception e) {
		cout << e.what();
	}
	previewImage.update();
}

//--------------------------------------------------------------
void ofApp::draw(){

}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
	if (key == ' ') {
		this->enableUpdate = ! this->enableUpdate;
	}
	if (key == 't') {
		this->enableTestPoint = ! this->enableTestPoint;
	}
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
