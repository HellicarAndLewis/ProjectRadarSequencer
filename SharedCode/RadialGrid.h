#include "ofMain.h"
int thetaDivisions = 16, radialDivisions = 4;
float startRadius = .4, endRadius = 1;
ofVec2f polarToCartesian(float theta, float radius) {
	return ofVec2f(cos(theta) * radius, sin(theta) * radius);
}
ofVec2f cartesianToPolar(float x, float y) {
	float theta = fmodf(atan2f(y, x) + TWO_PI, TWO_PI);
	return ofVec2f(theta, sqrtf((x * x) + (y * y)));
}
void getGridCoordinates(float x, float y, int& i, int& j) {
	float nx = ofMap(x, 0, ofGetWidth(), -1, 1);
	float ny = ofMap(y, 0, ofGetHeight(), -1, 1);
	ofVec2f cur = cartesianToPolar(nx, ny);
	i = ofMap(cur.x, 0, TWO_PI, 0, thetaDivisions, true);
	j = ofMap(cur.y, startRadius, endRadius, 0, radialDivisions, true);
	j = MIN(j, radialDivisions - 1);
}
class RadialGrid {
public:
	ofMesh mesh;
	vector< vector<ofMesh> > quads;
	void setup() {
		mesh.setMode(OF_PRIMITIVE_LINES);
		quads.resize(thetaDivisions, vector<ofMesh>(radialDivisions));
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
				ofMesh& quad = quads[i][j];
				quad.setMode(OF_PRIMITIVE_LINE_LOOP);
				quad.addVertex(nw);
				quad.addVertex(ne);
				quad.addVertex(se);
				quad.addVertex(sw);
			}
		}
	}
	void draw() {
		mesh.draw();
	}
	void draw(int i, int j) {
		quads[i][j].draw();
	}
};