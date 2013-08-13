#include "ofMain.h"
#include "MiniFont.h"
#include "ofxMidi.h"

class Midi : public ofxMidiOut {
public:
	void sendNote(int channel, int pitch, int velocity = 64) {
		sendNoteOn(channel, pitch, velocity);
		sendNoteOff(channel, pitch, 0);
	}
};

Midi midi;
int channels = 16;
int notes = 1;
int ccs = 6;

bool pressed = false;

class ofApp : public ofBaseApp {
public:	
	void setup() {
		MiniFont::setup();
		midi.listPorts();
		midi.openPort();
	}
	void update() {
	}
	void draw() {
		int w = channels, h = notes + ccs;
		int rw = ofGetWidth() / w, rh = ofGetHeight() / h;
		ofBackground(0);
		for(int v = 0; v < h; v++) {
			for(int channel = 0; channel < w; channel++) {
				float x = ofMap(channel, 0, channels, 0, ofGetWidth());
				float y = ofMap(v, 0, h, 0, ofGetHeight());
				ofSetColor(128);
				ofNoFill();
				ofRectangle rect(x, y, rw, rh);
				if(rect.inside(mouseX, mouseY)) {
					ofFill();
					ofSetColor(64);
					if(pressed) {
						ofSetColor(255);
						if(v < notes) {
							midi.sendNote(1 + channel, 64);
						} else {
							midi.sendControlChange(1 + channel, v - notes, 64);
						}
						pressed = false;
					}
				}
				ofRect(rect);
				ofSetColor(255);
				ofFill();
				if(y < notes) {
					MiniFont::draw("note", x + 2, y + 10);
					MiniFont::draw(ofToString(1 + channel) + "/" + ofToString(64 + v), x + 2, y + 20);
				} else {
					MiniFont::draw("cc", x + 2, y + 10);
					MiniFont::draw(ofToString(1 + channel) + "/" + ofToString(v - notes), x + 2, y + 20);					
				}
			}
		}
	}
	void mousePressed(int x, int y, int b) {
		pressed = true;
	}
};

int main( ){
	ofSetupOpenGL(420, 240, OF_WINDOW);
	ofRunApp(new ofApp());
}