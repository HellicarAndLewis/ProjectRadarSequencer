#include "ofMain.h"
class Led {
public:
	static ofImage glow;
	static float scale;
	ofVec2f position;
	float theta;
	ofColor color;
	static void setup() {
		glow.loadImage("glow.png");
		glow.setAnchorPercent(.5, .5);
	}
	void draw() {
		ofPushMatrix();
		ofSetColor(color);
		ofTranslate(position);
		ofScale(scale, scale);
		glow.draw(0, 0);
		ofPopMatrix();
	}
};
ofImage Led::glow;
float Led::scale = .3;
class LedRing : public vector<Led> {
public:
	void setup(int n = 64, float radius = 128) {
		Led::setup();
		for(int i = 0; i < n; i++) {
			Led led;
			led.theta = ofMap(i, 0, n, 0, 360);
			led.position = ofVec2f(radius, 0).rotate(led.theta);
			led.color = ofColor::white;
			push_back(led);
		}
	}
	void draw() {
		for(int i = 0; i < size(); i++) {
			at(i).draw();
		}
	}
};
float shortestPath(float a, float b, float range) {
	a = fmodf(a, range), b = fmodf(b, range);
	float c = fabsf(a - b), d = fabsf((a - range) - b);
	return MIN(c, d);
}
class ofApp : public ofBaseApp {
public:
	LedRing ledRing;
	void setup() {
		ledRing.setup();
	}
	void update() {
		float mouseTheta = atan2f(mouseY - ofGetHeight() / 2, mouseX - ofGetWidth() / 2);
		mouseTheta = fmodf(ofRadToDeg(mouseTheta) + 180, 360);
		float rate = 180;
		float arcLength = 90;
		for(int i = 0; i < ledRing.size(); i++) {
			float distance = shortestPath(mouseTheta, ledRing[i].theta, 360);
			distance = fabsf(distance - 180);
			ledRing[i].color = ofColor(ofMap(distance, 0, arcLength, 255, 0, true));
		}
	}
	void draw() {
		ofBackground(0);
		ofTranslate(ofGetWidth() / 2, ofGetHeight() / 2);
		ledRing.draw();
	}
};
int main( ){
	ofSetupOpenGL(512, 512, OF_WINDOW);
	ofRunApp(new ofApp());
}