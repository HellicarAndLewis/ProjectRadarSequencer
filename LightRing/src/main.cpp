#include "ofMain.h"
class LED {
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
		ofPushStyle();
		ofNoFill();
		ofSetColor(color);
		ofTranslate(position);
		ofScale(scale, scale);
		glow.draw(0, 0);
		ofPopStyle();
		ofPopMatrix();
	}
};
ofImage LED::glow;
float LED::scale = .3;
class ofApp : public ofBaseApp {
public:
	vector<LED> leds;
	void setup() {
		int n = 64;
		float radius = 100;
		LED::setup();
		for(int i = 0; i < n; i++) {
			LED led;
			led.theta = ofMap(i, 0, n, 0, 360);
			led.position = ofVec2f(radius, 0).rotate(led.theta);
			led.color = ofColor::white;
			leds.push_back(led);
		}
	}
	void update() {
		for(int i = 0; i < leds.size(); i++) {
			leds[i].color = ofColor(ofMap(sin(ofGetElapsedTimef() * 20 + i), -1, 1, 0, 255));
		}
	}
	void draw() {
		ofBackground(0);
		ofTranslate(ofGetWidth() / 2, ofGetHeight() / 2);
		for(int i = 0; i < leds.size(); i++) {
			leds[i].draw();
		}
	}
};
int main( ){
	ofSetupOpenGL(1280, 720, OF_WINDOW);
	ofRunApp(new ofApp());
}