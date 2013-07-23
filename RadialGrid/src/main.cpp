#include "ofMain.h"
ofVec2f polarToCartesian(float theta, float radius) {
	return ofVec2f(cos(theta) * radius, sin(theta) * radius);
}
class ofApp : public ofBaseApp {
public:
	ofMesh mesh;
	void setup() {
		int thetaDivisions = 16, radialDivisions = 4;
		float startRadius = .5, endRadius = 1;
		mesh.setMode(OF_PRIMITIVE_LINES);
		for(int i = 0; i < thetaDivisions; i++) {
			for(int j = 0; j < radialDivisions; j++) {
				float left = ofMap(i, 0, thetaDivisions, 0, TWO_PI);
				float right = ofMap(i + 1, 0, thetaDivisions, 0, TWO_PI);
				float top = ofMap(j, 0, radialDivisions, startRadius, endRadius);
				float bottom = ofMap(j + 1, 0, radialDivisions, startRadius, endRadius);
				ofVec2f nw = polarToCartesian(left, top);
				ofVec2f ne = polarToCartesian(right, top);
				ofVec2f sw = polarToCartesian(left, bottom);
				ofVec2f se = polarToCartesian(right, bottom);
				mesh.addVertex(nw);
				mesh.addVertex(ne);
				mesh.addVertex(nw);
				mesh.addVertex(sw);
				if(j + 1 == radialDivisions) {
					mesh.addVertex(sw);
					mesh.addVertex(se);
				}
			}
		}
	}
	void draw() {
		ofBackground(0);
		ofTranslate(ofGetWidth() / 2, ofGetHeight() / 2);
		ofScale(ofGetWidth() / 2, ofGetHeight() / 2);
		mesh.draw();
	}
};
int main( ){
	ofSetupOpenGL(512, 512, OF_WINDOW);
	ofRunApp(new ofApp());
}