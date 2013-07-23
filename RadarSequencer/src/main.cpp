#include "ofMain.h"

#include "ofxCv.h"
#include "ofxDmx.h"
#include "ofxSick.h"
#include "ofxMidi.h"
#include "ofxUI.h"

ofPoint sickOffset;
float sickRotation = 0;
bool useRecording = false;

class ofApp : public ofBaseApp {
public:	
	ofxSick *sickA, *sickB;
	ofxSickPlayer sickPlayerA, sickPlayerB;
	ofxSickGrabber sickGrabberA, sickGrabberB;
	ofxUICanvas* gui;
	
	void setup() {
		sickA = &sickPlayerA;
		sickB = &sickPlayerB;
		setupGui();
	}
	void setupGui() {
		gui = new ofxUICanvas();
		gui->setTheme(OFX_UI_THEME_MINBLACK);
		gui->addLabel("Radar Sequencer");
		gui->addSpacer();
		gui->addFPS();
		gui->addSpacer();
		gui->addLabelToggle("Use recording", &useRecording);
		gui->addSpacer();
		gui->addMinimalSlider("Sick rotation", 0, 360, &sickRotation);
		gui->add2DPad("Sick offset", ofVec2f(-250, 250), ofVec2f(-250, 250), &sickOffset, 100, 100);
		gui->autoSizeToFitWidgets();
	}
	void update() {
		
	}
	void draw() {
		
	}
};

int main( ){
	ofSetupOpenGL(1280, 720, OF_WINDOW);
	ofRunApp(new ofApp());
}