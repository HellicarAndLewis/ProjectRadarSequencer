#include "ofMain.h"
#include "LedRing.h"
class ofApp : public ofBaseApp {
public:
	LedRing ledRing;
	ofFbo fbo;
	ofPixels fboPixels;
	void setup() {
		ledRing.setup();
		fbo.allocate(512, 512);
	}
	void update() {
		fbo.begin();
		ofClear(0);
		ofTranslate(256, 256);
		ofRotate(mouseX);
		ofScale(200, 200);
		ofRect(-.5, -.5, 1, 1);
		fbo.end();
		fbo.readToPixels(fboPixels);
		ledRing.update(fboPixels);
	}
	void draw() {
		ofBackground(0);
		//fbo.draw(0, 0);
		ofTranslate(ofGetWidth() / 2, ofGetHeight() / 2);
		ledRing.draw();
	}
};
int main( ){
	ofSetupOpenGL(512, 512, OF_WINDOW);
	ofRunApp(new ofApp());
}