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
int ccs = 3;

int lastControl = -1;

class ofApp : public ofBaseApp {
public:	
	void setup() {
		ofDisableAntiAliasing();
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
		int curControl = 0;
		for(int v = 0; v < h; v++) {
			for(int channel = 0; channel < w; channel++) {
				int x = channel * rw;
				int y = v * rh;
				ofSetColor(128);
				ofNoFill();
				ofRectangle rect(x, y, rw, rh);
				ofRect(rect);
				int amt = 0;
				if(rect.inside(mouseX, mouseY)) {
					if(ofGetKeyPressed(OF_KEY_SHIFT)) {
						amt = 64;
					} else {
						amt = ofMap(mouseY, rect.getTop(), rect.getBottom(), 0, 127, true);
					}
					ofFill();
					ofSetColor(64);
					if(ofGetMousePressed()) {
						ofSetColor(128);
						if(v < notes) {
							if(curControl != lastControl) {
								midi.sendNote(1 + channel, amt);
							}
							lastControl = curControl;
						} else {
							midi.sendControlChange(1 + channel, v - notes, amt);
						}
					}
					rect.setY(ofMap(amt, 0, 127, rect.getTop(), rect.getBottom()));
					rect.setHeight(ofMap(amt, 0, 127, rect.getHeight(), 0));
				}
				ofRect(rect);
				ofSetColor(255);
				ofFill();
				if(y < notes) {
					MiniFont::draw("note", x + 2, y + 10);
					MiniFont::draw(ofToString(1 + channel) + "/" + ofToString(64 + v), x + 2, y + 20);
					MiniFont::draw(ofToString(amt), x + 2, y + 30);
				} else {
					MiniFont::draw("cc", x + 2, y + 10);
					MiniFont::draw(ofToString(1 + channel) + "/" + ofToString(v - notes), x + 2, y + 20);
					MiniFont::draw(ofToString(amt), x + 2, y + 30);			
				}
				curControl++;
			}
		}
	}
};

int main( ){
	ofSetupOpenGL(420, 240, OF_WINDOW);
	ofRunApp(new ofApp());
}