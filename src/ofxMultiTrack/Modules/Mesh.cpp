#include "ofxTriangle.h"
#include "Mesh.h"
#include "../Utils/Constants.h"
#include "ofxCv.h"

namespace ofxMultiTrack {
	namespace Modules {
		//----------
		string Mesh::getType() const {
			return "Mesh";
		}
			
		//----------
		Mesh::Mesh(shared_ptr<Devices::KinectSDK> kinect, int globalIndex) {
			this->isOscTargetSet = false;
			this->kinect = kinect;
			this->sensor = & kinect->getDevice().getNuiSensor();
			this->globalIndex = globalIndex;
			this->sendBuffer.allocate(64 * 1024);
		}

		//----------
		void Mesh::init() {
		}

		//----------
		ofVec3f depthToWorld(int x, int y, const ofShortPixels & depthData, INuiCoordinateMapper * mapper) {
			NUI_DEPTH_IMAGE_POINT depthPoint;
			depthPoint.x = x;
			depthPoint.y = y;
			depthPoint.depth = depthData[x + y * 640] << 1;
			Vector4 skeletonPoint;
			mapper->MapDepthPointToSkeletonPoint(NUI_IMAGE_RESOLUTION_640x480, & depthPoint, & skeletonPoint);
			return ofVec3f(skeletonPoint.x, skeletonPoint.y, skeletonPoint.z) / skeletonPoint.w;
		}

		//----------
		void Mesh::update() {
			if (!this->udpSender) {
				return;
			}

			try {
				//allocate images
				auto rawDepthPixels = this->kinect->getDevice().getRawDepthPixelsRef();
				auto depthImage16 = ofxCv::toCv(rawDepthPixels);
				cv::Mat result;
				cv::Mat depthImage8;
				depthImage16.convertTo(depthImage8, CV_8UC1, 0.25);

				//treat depth image
				cv::Canny(depthImage8, result, 30, 30.0 * 3.0f, 3);
				cv::dilate(result, result, cv::Mat(), cvPoint(-1,-1), 3);
				cv::erode(result, result, cv::Mat(), cvPoint(-1,-1), 3);
				result = depthImage8 - result;
				result = result > 0;
				cv::erode(result, result, cv::Mat(), cvPoint(-1,-1), 1);

				//make preview image
				ofxCv::copy(result, preview);
				preview.update();

				//find contours
				vector<vector<cv::Point> > depthContours;
				cv::findContours(result, depthContours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE, cv::Point(0, 0) );
		
				//strip based on min area
				const auto toRemove = std::remove_if(depthContours.begin(), depthContours.end(), [this] (vector<cv::Point> & contour) {
					bool remove = cv::contourArea(contour) < (0.005 * 640.0f * 480.0f);
					return remove;
				});
				depthContours.erase(toRemove, depthContours.end());

				//fill the area storage
				this->contourAreas.clear();
				for(auto depthContour : depthContours) {
					this->contourAreas.push_back(cv::contourArea(depthContour));
				}

				INuiCoordinateMapper * mapper;
				this->sensor->NuiGetCoordinateMapper(&mapper);

				const int decimateContour = 3;

				this->worldContours.clear();
				for(auto depthContour : depthContours) {
					//decimate and reformat
					int iPoint = 0;
					vector<ofVec3f> depthPoints;
					for(auto & point : depthContour) {
						if (decimateContour > 1 ? (iPoint++ % (int) decimateContour == 0) : true) {
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
						meshInWorld.addVertex(depthToWorld(triangle.a.x, triangle.a.y, rawDepthPixels, mapper));
						meshInWorld.addVertex(depthToWorld(triangle.b.x, triangle.b.y, rawDepthPixels, mapper));
						meshInWorld.addVertex(depthToWorld(triangle.c.x, triangle.c.y, rawDepthPixels, mapper));
					}
					auto numIndices = meshInWorld.getNumVertices();
					vector<ofIndexType> newIndices(numIndices);
					for(int iIndex = 0; iIndex < numIndices; iIndex++) {
						newIndices[iIndex] = iIndex;
					}
					meshInWorld.addIndices(newIndices);

					meshInWorld.mergeDuplicateVertices();
					this->worldContours.push_back(meshInWorld);
				}
			} catch(std::exception e) {
				cout << e.what() << endl;
			}

			this->sendBinary();
		}

		//----------
		Json::Value Mesh::serialize() {
			Json::Value json;
			json["meshCount"] = this->worldContours.size();
			return json;
		}

		//----------
		void Mesh::deserialize(const Json::Value & data) {

		}
		
		//----------
		void Mesh::setConfig(const Json::Value & json) {
			const auto parameterNames = json.getMemberNames();
			for(auto parameterName : parameterNames) {
				const auto jsonParameter = json[parameterName];
				if (parameterName == "client") {
					this->setTarget(jsonParameter["address"].asString(), jsonParameter["port"].asInt());
				} else if (parameterName == "transform") {
					if (jsonParameter.size() == 16) {
						for(int i=0; i<16; i++) {
							this->transform.getPtr()[i] = jsonParameter[i].asFloat();
						}
					}
				}
			}
		}

		//----------
		Json::Value Mesh::getStatus() {
			return this->status;
		}

		//----------
		void Mesh::setTarget(const string address, int port) {
			this->sender.setup(address, port);
			this->udpSender = shared_ptr<UdpTransmitSocket>(new UdpTransmitSocket(IpEndpointName(address.c_str(), 4445)));
			this->isOscTargetSet = true;
			this->status["clientAddress"] = address;
			this->status["clientPort"] = port;
		}

		//----------
		void Mesh::drawWorld() {
			int contourIndex = 0;
			ofPushStyle();
			ofSetLineWidth(3.0f);
			for(auto & worldContour : this->worldContours) {
				ofColor color(255, 0, 0);
				const auto area = this->contourAreas[contourIndex];
				color.setHue(((int) (area * 0.001f)) % 360);
				ofSetColor(color);
				worldContour.drawWireframe();
				contourIndex++;
			}

			ofPopStyle();
		}
			
		//----------
		void Mesh::sendBinary() {
			if (this->udpSender) {
				auto writeLocation = sendBuffer.getBinaryBuffer();
				int totalSize = 0;

				memcpy(writeLocation, & this->globalIndex, sizeof(int));
				totalSize += sizeof(int);
				writeLocation += sizeof(int);

				memcpy(writeLocation, this->transform.getPtr(), sizeof(ofMatrix4x4));
				totalSize += sizeof(ofMatrix4x4);
				writeLocation += sizeof(ofMatrix4x4);

				ofMesh combinedMesh;
				for(auto mesh : this->worldContours) {
					combinedMesh.append(mesh);
				}

				const int numIndices = combinedMesh.getNumIndices();
				memcpy(writeLocation, & numIndices, sizeof(int));
				totalSize += sizeof(int);
				writeLocation += sizeof(int);

				memcpy(writeLocation, combinedMesh.getIndexPointer(), combinedMesh.getNumIndices() * sizeof(ofIndexType));
				totalSize += combinedMesh.getNumIndices() * sizeof(ofIndexType);
				writeLocation += combinedMesh.getNumIndices() * sizeof(ofIndexType);

				const int numVertices = combinedMesh.getNumVertices();
				memcpy(writeLocation, & numVertices, sizeof(int));
				totalSize += sizeof(int);
				writeLocation += sizeof(int);

				const auto vertices = combinedMesh.getVertices();
				auto size = vertices.size() * sizeof(ofVec3f);
				memcpy(writeLocation, combinedMesh.getVerticesPointer(), size);
				writeLocation += size;
				totalSize += size;

				this->udpSender->Send(sendBuffer.getBinaryBuffer(), totalSize);
			}

			/*
			if (!this->isOscTargetSet) {
				return;
			}

			ofxOscBundle bundle;
			const string baseBaseAddress = "/node/" + ofToString(this->globalIndex);

			for(int index = 0; index < OFXMULTITRACK_NODE_MAX_MESH_COUNT; index++) {
				const auto baseAddress = baseBaseAddress + "/contour/" + ofToString(index);
				ofxOscMessage dataMessage;
				dataMessage.setAddress(baseAddress + "/data");
				int size = 0;
				if (this->worldContours.size() > index) {
					const auto worldContour = this->worldContours[index];
					const auto vertices = worldContour.getVertices();
					for(const auto vertex : vertices) {
						dataMessage.addFloatArg(vertex.x);
						dataMessage.addFloatArg(vertex.y);
						dataMessage.addFloatArg(vertex.z);
					}
					size++;
				}
				bundle.addMessage(dataMessage);

				ofxOscMessage countMessage;
				countMessage.setAddress(baseAddress + "/count");
				countMessage.addIntArg(size);
				bundle.addMessage(countMessage);
			}

			ofxOscMessage totalCountMessage;
			totalCountMessage.setAddress(baseBaseAddress + "/contourCount");
			totalCountMessage.addIntArg(this->worldContours.size());
			bundle.addMessage(totalCountMessage);

			this->sender.sendBundle(bundle);*/
		}
	}
}