#include "ofMain.h"
float startRadius = .4, endRadius = 1;

class RadialGrid {
public:
	int thetaDivisions, radialDivisions;
	ofMesh mesh;
	vector< vector<ofMesh> > quads;
	void setup(int radialDivisions, int thetaDivisions) {
		this->radialDivisions = radialDivisions;
		this->thetaDivisions = thetaDivisions;
		mesh.setMode(OF_PRIMITIVE_LINES);
		quads.resize(radialDivisions, vector<ofMesh>(thetaDivisions));
		for(int i = 0; i < radialDivisions; i++) {
			for(int j = 0; j < thetaDivisions; j++) {
				float top = ofMap(i, 0, radialDivisions, startRadius, endRadius);
				float bottom = ofMap(i + 1, 0, radialDivisions, startRadius, endRadius);
				float left = ofMap(j, 0, thetaDivisions, 0, TWO_PI);
				float right = ofMap(j + 1, 0, thetaDivisions, 0, TWO_PI);
				ofVec2f nw = polarToCartesian(top, left);
				ofVec2f ne = polarToCartesian(top, right);
				ofVec2f sw = polarToCartesian(bottom, left);
				ofVec2f se = polarToCartesian(bottom, right);
				mesh.addVertex(nw);
				mesh.addVertex(ne);
				mesh.addVertex(nw);
				mesh.addVertex(sw);
				if(i + 1 == radialDivisions) {
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
	ofVec2f polarToCartesian(float radius, float theta) {
		return ofVec2f(cos(theta) * radius, sin(theta) * radius);
	}
	ofVec2f cartesianToPolar(float x, float y) {
		float radius = sqrtf((x * x) + (y * y));
		float theta = fmodf(atan2f(y, x) + TWO_PI, TWO_PI);
		return ofVec2f(radius, theta);
	}
	void getGridCoordinates(float x, float y, int& i, int& j) {
		ofVec2f cur = cartesianToPolar(x, y);
		i = ofMap(cur.x, startRadius, endRadius, 0, radialDivisions, true);
		i = MIN(j, radialDivisions - 1);
		j = ofMap(cur.y, 0, TWO_PI, 0, thetaDivisions, true);
	}
};