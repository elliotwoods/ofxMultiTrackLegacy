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

	this->ddepth.set("depth format", -1, -1, 3);
	this->ddx.set("ddx", 1, 1, 32);
	this->ksize.set("ksize", 3, 1, 32);
	this->threshold.set("threshold", 46, 0, 255);
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

	addIntSlider(this->ddx);
	addIntSlider(this->ksize);
	addIntSlider(this->threshold);
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
			color.setHue(area * 0.01f);
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
			color.setHue(cv::contourArea(this->contours[contourIndex]) * 0.01f);
			ofSetColor(color);
			worldContour.draw();
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

//--------------------------------------------------------------
void ofApp::update(){
	if (this->enableUpdate) {
		this->kinect.update();
	}

	auto depthImage = ofxCv::toCv(this->kinect.getDepthPixelsRef());

	try {
		cv::Mat result;
		cv::Sobel(depthImage, result, (int) this->ddepth, (int) this->ddx, (int) this->ddx, (int) this->ksize);
		result = result > this->threshold;
		cv::dilate(result, result, cv::Mat(), cvPoint(-1,-1), this->dilateIterations);
		cv::erode(result, result, cv::Mat(), cvPoint(-1,-1), this->erodeIterations);
		result = depthImage - result;
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
				if (iPoint++ % (int) this->decimateContour == 0) {
					depthPoints.push_back(ofVec3f(point.x, point.y, 0));
				}
			}

			//triangulate
			ofxDelaunay triangulate;
			triangulate.addPoints(depthPoints);
			triangulate.triangulate();
			const auto & meshInDepth = triangulate.triangleMesh;
			auto meshInWorld = meshInDepth;

			//transform to skeleton space
			auto & vertices = meshInWorld.getVertices();
			for(auto & vertex : vertices) {
				NUI_DEPTH_IMAGE_POINT depthPoint;
				depthPoint.x = vertex.x;
				depthPoint.y = vertex.y;
				depthPoint.depth = depthData[vertex.x + vertex.y * 640] << 1;
				Vector4 skeletonPoint;
				mapper->MapDepthPointToSkeletonPoint(NUI_IMAGE_RESOLUTION_640x480, & depthPoint, & skeletonPoint);
				vertex = ofVec3f(skeletonPoint.x, skeletonPoint.y, skeletonPoint.z) / skeletonPoint.w;
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
