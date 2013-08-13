#include "ofMain.h"
#include "ofxDmx.h"

class ofApp : public ofBaseApp {
public:
	
	vector<ofxDmx> dmx;
	vector< vector<int> > channels;
	
	void setup() {
		ofSetFrameRate(30);
		ofSetLogLevel(OF_LOG_VERBOSE);
		ofXml xml("settings.xml");
		int n = xml.getNumChildren("dmx");
		dmx.resize(n);
		channels.resize(n);
		xml.setTo("dmx");
		for(int i = 0; i < n; i++) {
			dmx[i].connect(xml.getAttribute("device"), ofToInt(xml.getAttribute("channels")));
			xml.setTo("range");
			do {
				int start= ofToInt(xml.getAttribute("start"));
				int stop = ofToInt(xml.getAttribute("stop"));
				for(int j = start; j <= stop; j++) {
					channels[i].push_back(j);
				}
			} while(xml.setToSibling());
			xml.setToParent();
			xml.setToSibling();
		}
	}
	void update() {
		for(int i = 0; i < channels.size(); i++) {
			int n = channels[i].size();
			int curFrame = ofGetFrameNum() % n;
			for(int j = 0; j < n; j++) {
				int channel = channels[i][j];
				dmx[i].setLevel(channel, curFrame == j ? 255 : 0);
			}
		}
		for(int i = 0; i < dmx.size(); i++) {
			dmx[i].update();
		}
	}
	void draw() {
		ofBackground(0);
		int h = dmx.size();
		float rh = ofGetHeight() / h;
		for(int i = 0; i < h; i++) {
			int w = dmx[i].getChannels();
			float rw = ofGetWidth() / w;
			for(int j = 1; j <= w; j++) {
				float x = ofMap(j, 0, w, 0, ofGetWidth());
				float y = ofMap(i, 0, h, 0, ofGetHeight());
				int level = dmx[i].getLevel(j);
				ofSetColor(level);
				ofFill();
				ofRect(x, y, rw, rh);
				ofNoFill();
				ofSetColor(128);
				ofRect(x, y, rw, rh);
			}
		}
	}
};

int main() {
	ofSetupOpenGL(1280, 720, OF_WINDOW);
	ofRunApp(new ofApp());
}