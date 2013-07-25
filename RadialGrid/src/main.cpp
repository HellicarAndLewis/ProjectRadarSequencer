#include "ofMain.h"
#include "RadialGrid.h"
class ofApp : public ofBaseApp {
public:
	RadialGrid grid;
	void setup() {
		grid.setup();
	}
	void draw() {
		ofBackground(0);
		ofTranslate(ofGetWidth() / 2, ofGetHeight() / 2);
		ofScale(ofGetWidth() / 2, ofGetHeight() / 2);
		ofSetColor(ofColor::white);
		grid.draw();
		int i, j;
		getGridCoordinates(mouseX, mouseY, i, j);
		ofSetColor(ofColor::red);
		grid.draw(i, j);
	}
};
int main( ){
	ofSetupOpenGL(512, 512, OF_WINDOW);
	ofRunApp(new ofApp());
}