#include "ofMain.h"
#include "LedRing.h"
ofVec2f makeRotatedVector(float angle) {
	return ofVec2f(1, 0).rotate(angle);
}
class Person {
public:
	ofVec2f position;
	Person(float x, float y) {
		position.set(x, y);
	}
	float getAngle() {
		return ofRadToDeg(atan2f(position.y, position.x));
	}
};
class Pulse {
protected:
	float lastDiff, lastAngle;
	float speed, startAngle, startTime;
	bool clockwise;
	void bounce() {
		startAngle = lastAngle;
		startTime = ofGetElapsedTimef();
		clockwise = !clockwise;
	}
public:
	Pulse(float startAngle)
	:startAngle(startAngle)
	,lastDiff(0)
	,lastAngle(0)
	,speed(60)
	,clockwise(true)
	,startTime(ofGetElapsedTimef()) {
	}
	float getAngle() {
		float t = ofGetElapsedTimef() - startTime;
		return startAngle + t * speed * (clockwise ? 1 : -1);
	}
	void setSpeed(float speed) {
		startAngle = getAngle();
		startTime = ofGetElapsedTimef();		
		this->speed = speed;
	}
	void update(vector<Person>& people) {
		float angle = getAngle();
		ofVec2f direction = makeRotatedVector(angle);
		float bestDiff;
		for(int i = 0; i < people.size(); i++) {
			float diff = direction.angle(people[i].position);
			if(i == 0 || abs(diff) < abs(bestDiff)) {
				bestDiff = diff;
			}
		}
		float bounceEpsilon = 15;
		if(abs(bestDiff) < bounceEpsilon &&
			 (bestDiff>0) != (lastDiff>0) &&
			 ((bestDiff < 0 && clockwise) || (bestDiff > 0 && !clockwise))) {
			bounce();
		} else {
			lastDiff = bestDiff;
			lastAngle = angle;
		}
	}
};
int getClockwise(vector<Person>& people, float angle) {
	for(int i = 0; i < people.size(); i++) {
		
	}
}
class ofApp : public ofBaseApp {
public:
	LedRing ledRing;
	ofFbo fbo;
	ofPixels fboPixels;
	vector<Person> people;
	vector<Pulse> pulses;
	void setup() {
		ledRing.setup();
		fbo.allocate(512, 512);
		fbo.begin();
		ofClear(0);
		fbo.end();
		ofSetCircleResolution(64);
		ofSetLineWidth(2);
	}
	void update() {
		for(int i = 0; i < pulses.size(); i++) {
			pulses[i].update(people);
		}
		fbo.begin();
		ofPushStyle();
		ofSetLineWidth(30);
		ofSetColor(0, 10);
		ofRect(0, 0, fbo.getWidth(), fbo.getHeight());
		ofSetColor(255);
		ofTranslate(ofGetWidth() / 2, ofGetHeight() / 2);
		for(int i = 0; i < pulses.size(); i++) {
			ofPushMatrix();
			ofRotate(pulses[i].getAngle());
			ofLine(0, 0, ofGetWidth() / 2, 0);
			ofPopMatrix();
		}
		ofPopStyle();
		fbo.end();
		fbo.readToPixels(fboPixels);
		ledRing.update(fboPixels);
	}
	void draw() {
		ofBackground(0);
		//fbo.draw(0, 0);
		ofTranslate(ofGetWidth() / 2, ofGetHeight() / 2);
		ledRing.draw();
		
		ofPushStyle();
		ofNoFill();
		ofCircle(0, 0, 1 * ofGetWidth() / 4);
		ofCircle(0, 0, 2 * ofGetWidth() / 4);
		ofPopStyle();
		
		for(int i = 0; i < people.size(); i++) {
			ofCircle(people[i].position, 4);
		}
		for(int i = 0; i < pulses.size(); i++) {
			ofPushMatrix();
			ofRotate(pulses[i].getAngle());
			ofLine(1 * ofGetWidth() / 4, 0, 2 * ofGetWidth() / 4, 0);
			ofPopMatrix();
		}
	}
	void mousePressed(int x, int y, int b) {
		people.push_back(Person(x - ofGetWidth() / 2, y - ofGetHeight() / 2));
		pulses.push_back(people.back().getAngle());
	}
	void mouseDragged(int x, int y, int b) {
		people.back().position.set(x - ofGetWidth() / 2, y - ofGetHeight() / 2);
	}
	void mouseMoved(int x, int y) {
		float speed = ofMap(x, 0, ofGetWidth(), 1, 180);
		for(int i = 0; i < pulses.size(); i++) {
			//pulses[i].setSpeed(speed);
		}
	}
};
int main( ){
	ofSetupOpenGL(512, 512, OF_WINDOW);
	ofRunApp(new ofApp());
}