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
	void setup(int n = 128, float radius = 128) {
		Led::setup();
		for(int i = 0; i < n; i++) {
			Led led;
			led.theta = ofMap(i, 0, n, 0, 360);
			led.position = ofVec2f(radius, 0).rotate(led.theta);
			led.color = ofColor::white;
			push_back(led);
		}
	}
	void update(ofBaseHasPixels& image) {
		update(image.getPixelsRef());
	}
	void update(ofPixels& pixels) {
		ofVec2f center(pixels.getWidth() / 2, pixels.getHeight() / 2);
		for(int i = 0; i < size(); i++) {
			ofVec2f position = center + at(i).position;
			at(i).color = pixels.getColor(position.x, position.y);
		}		
	}
	void draw() {
		ofPushStyle();
		for(int i = 0; i < size(); i++) {
			at(i).draw();
		}
		ofPopStyle();
	}
};
float shortestPath(float a, float b, float range) {
	a = fmodf(a, range), b = fmodf(b, range);
	float c = fabsf(a - b), d = fabsf((a - range) - b);
	return MIN(c, d);
}