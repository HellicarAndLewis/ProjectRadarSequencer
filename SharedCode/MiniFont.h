#pragma once

#include "ofMain.h"

namespace MiniFont {
	void updateProjectionState();
	ofVec3f ofWorldToScreen(ofVec3f world);
	void setup();
	void draw(string str, int x, int y);
	void draw(string str, const ofVec2f& position);
	void drawHighlight(string str, const ofVec2f& position, ofColor bg = ofColor(0), ofColor fg = ofColor(255));
	void drawHighlight(string str, int x, int y, ofColor bg = ofColor(0), ofColor fg = ofColor(255));
	void drawScreen(string str, ofVec3f position);
}