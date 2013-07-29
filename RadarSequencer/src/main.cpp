#include "ofMain.h"

#include "ofxCv.h"
#include "ofxDmx.h"
#include "ofxSick.h"
#include "ofxMidi.h"
#include "ofxUI.h"
#include "LedRing.h"
#include "RadialGrid.h"
#include "Sequencer.h"

ofPoint sickOffset;
float sickRotation = 0;
bool useRecording = false;

class ofApp : public ofBaseApp {
public:	
	ofxSick *sickA, *sickB;
	ofxSickPlayer sickPlayerA, sickPlayerB;
	ofxSickGrabber sickGrabberA, sickGrabberB;
	ofxUICanvas* gui;
	Sequencer sequencer;
	LedRing ring;
	RadialGrid grid;
	ofFbo fbo;
	ofPixels fboPixels;
	
	vector<ofVec2f> people;
	
	void setup() {
		sickA = &sickPlayerA;
		sickB = &sickPlayerB;
		setupGui();
		
		ofXml xml;
		xml.load("settings.xml");
		xml.setTo("sequencer");
		string notes = xml.getValue("notes");
		int bpm = xml.getValue<int>("bpm");
		int columns = xml.getValue<int>("columns");
		int channel = xml.getValue<int>("channel");
		vector<unsigned char> noteList;
		vector<string> noteSplit = ofSplitString(notes, ",", true, true);
		for(int i = 0; i < noteSplit.size(); i++) {
			noteList.push_back((unsigned char) ofToInt(noteSplit[i]));
		}
		sequencer.setup(noteList, columns, bpm, channel);
		
		ring.setup();
		grid.setup(noteList.size(), columns);
	}
	void setupGui() {
		gui = new ofxUICanvas();
		gui->addLabel("Radar Sequencer");
		gui->addSpacer();
		gui->addFPS();
		gui->addSpacer();
		gui->addLabelToggle("Use recording", &useRecording);
		gui->addSpacer();
		gui->addMinimalSlider("Sick rotation", 0, 360, &sickRotation);
		gui->add2DPad("Sick offset", ofVec2f(-250, 250), ofVec2f(-250, 250), &sickOffset, 200, 200);
		gui->autoSizeToFitWidgets();
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
		ring.update(fboPixels);		
	}
	ofVec2f mapToGrid(ofVec2f point) {
		return ofVec2f((point.x - (ofGetWidth() / 2)) / 256., (point.y - (ofGetHeight() / 2)) / 256.);
	}
	void draw() {
		ofPushStyle();
		ofPushMatrix();
		
		ofBackground(0);
		int gridRow, gridCol;
		vector<vector<bool> > states;
		sequencer.clearStates();
		for(int i = 0; i < people.size(); i++) {
			ofCircle(people[i], 10);
			ofVec2f position = mapToGrid(people[i]);
			grid.getGridCoordinates(position.x, position.y, gridRow, gridCol);
			sequencer.setState(gridRow, gridCol, true);
		}
		
		ofTranslate(ofGetWidth() / 2, ofGetHeight() / 2);
		
		ofSetColor(ofColor::white);
		ring.draw();
		
		ofPushMatrix();
		ofScale(256, 256);
		grid.draw();
		ofPopMatrix();
		
		ofScale(40, 40);
		ofSetColor(ofColor::red);
		for(int row = 0; row < sequencer.getRows(); row++) {
			for(int col = 0; col < sequencer.getCols(); col++) {
				if(sequencer.getState(row, col)) {
					grid.draw(row, col);
					ofFill();
				} else {
					ofNoFill();
				}
				ofCircle(col, row, .5, .5);
			}
		}

		ofPopMatrix();
		ofPopStyle();
	}
	void mouseDragged(int x, int y, int b) {
		people.push_back(ofVec2f(x, y));
	}
	void exit() {
		sequencer.waitForThread();
	}
};

int main() {
	ofSetupOpenGL(1280, 720, OF_WINDOW);
	ofRunApp(new ofApp());
}