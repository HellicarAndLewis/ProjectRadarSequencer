#include "ofMain.h"
#include "MiniFont.h"
#include "ofxMidi.h"
#include "ofxTiming.h"

const float pxToMm = 1. / 10.;
float carWidth = 2600, tireWidth = 550, tireHeight = 180, tireSpacing = 845;
float directionSize = 15000, triangleSide = 50000;
float outerEllipseWidth = 4600, outerEllipseHeight = 3480;
float innerEllipseWidth = 3840, innerEllipseHeight = 2720;
float peopleRadius = 300 * pxToMm;
float offsetAmount = 600;
float debounceTime = .05;

ofVec2f spka(-1, +1), spkb(-1, -1), spkc(+1, +1), spkd(+1, -1);
float spkRadius = 3000, spkSize = 100;
bool isInside(float x, float left, float right) {
	return left < x && x < right;
}

float distanceWrap(float a, float b, float range) {
	a = ofWrap(a, 0, range);
	b = ofWrap(b, 0, range);
	if(a > b) {
		swap(a, b);
	}
	float d1 = b - a, d2 = a + (range - b);
	return MIN(d1, d2);
}

int isInsideWrap(float x, float left, float right, float range) {
	x = ofWrap(x, 0, range);
	left = ofWrap(left, 0, range);
	right = ofWrap(right, 0, range);
	bool nearLeft = distanceWrap(x, left, range) < distanceWrap(x, right, range);
	bool inside = left < right ?
	(isInside(x, left, right)) :
	(isInside(x, left, range) || isInside(x, 0, right));
	return inside ? 0 : (nearLeft ? -1 : +1);
}

class Midi : public ofxMidiOut {
public:
	void sendNote(int channel, int pitch, int velocity = 64, int length = 150) {
		sendNoteOn(channel, pitch, velocity);
		sendNoteOff(channel, pitch, 0);
		/*
		 // this wasn't causing too much bandwidth to be used
		vector<unsigned char> bytes;
		bytes.push_back(MIDI_NOTE_OFF+(channel-1));
		bytes.push_back(pitch);
		bytes.push_back(velocity);
		sendMidiBytes(bytes, length);*/
	}
};

Midi midi;

class DraggableCircle {
protected:
	bool selected;
	ofVec2f position;
	static int idCount;
	int id;
	float radius;
	ofVec2f offset;
public:
	DraggableCircle()
	:selected(false)
	,radius(8) {
	}
	virtual void setup() {
		id = idCount++;
		ofAddListener(ofEvents().mousePressed, this, &DraggableCircle::mousePressed);
		ofAddListener(ofEvents().mouseMoved, this, &DraggableCircle::mouseMoved);
		ofAddListener(ofEvents().mouseDragged, this, &DraggableCircle::mouseDragged);
		ofAddListener(ofEvents().draw, this, &DraggableCircle::draw);
	}
	bool getSelected() const {
		return selected;
	}
	ofVec2f& getPosition() {
		return position;
	}
	void mousePressed(ofMouseEventArgs& e) {
		offset = position - e;
		mouseMoved(e);
	}
	void mouseMoved(ofMouseEventArgs& e) {
		selected = position.distance(e) < radius;
	}
	void mouseDragged(ofMouseEventArgs& e) {
		if(selected) {
			position.set(e + offset);
		}
	}
	virtual void draw() {
	}
	virtual void draw(ofEventArgs& e) {
		ofPushMatrix();
		ofTranslate((int) position.x, (int) position.y);
		ofPushStyle();
		ofFill();
		ofSetColor(255, 128);
		ofCircle(0, 0, radius);
		if(selected) {
			ofSetColor(ofColor::red);
			ofNoFill();
			ofCircle(0, 0, radius + 4);				
		}
		draw();
		ofPopStyle();
		ofPopMatrix();
	}
};

int DraggableCircle::idCount = 0;

class HasAngle {
public:
	virtual float getAngle() const = 0;
	ofVec2f getNormal() const {
		return ofVec2f(1, 0).rotate(getAngle());
	}
	bool operator < (const HasAngle& other) const {
		return getAngle() < other.getAngle();
	}
};

class Person : public DraggableCircle, public HasAngle {
public:
	float leftAngle, rightAngle;
	DelayTimer timer;
	Person()
	:leftAngle(0)
	,rightAngle(0) {
	}
	virtual void setup(ofVec2f position) {
		DraggableCircle::setup();
		this->position = position;
		radius = peopleRadius;
		timer.setPeriod(1);
		timer.setUseBuffer(false);
		ofAddListener(ofEvents().update, this, &Person::update);
	}
	ofVec2f getWorldPosition() const {
		return position - ofGetWindowSize() / 2;
	}
	float getPresence() const {
		return getWorldPosition().length();
	}
	virtual float getAngle() const {
		return ofVec2f(1, 0).angle(getWorldPosition());
	}
	void update(ofEventArgs& e) {
		if(timer.tick()) {
			int channel = 1 + id;
			ofVec2f normal = getNormal();
			float pan = ofMap(normal.x, -1, 1, 0, 127);
			midi.sendControlChange(channel, 0, pan);
			midi.sendNote(channel, 64 + id * 6, 127, 1200);
		}
	}
	virtual void draw() {
		MiniFont::drawHighlight("Person " + ofToString(id), 0, 0);
		MiniFont::drawHighlight(ofToString((int) getPresence()) + "mm", 0, 10);
	}
};

float speed = 150;
class Pulse : public HasAngle {
public:
	static int idCount;
	int id;
	float leftAngle, rightAngle, pulseCenter;
	bool direction;
	float startAngle, startTime;
	float lastBounceTime;
	float currentAngle;
	
	Pulse()
	:leftAngle(0)
	,rightAngle(0)
	,startAngle(0)
	,startTime(0)
	,lastBounceTime(0)
	,direction(false) {
	}
	void setup(float angle) {
		pulseCenter = angle;
		startAngle = angle;
		ofAddListener(ofEvents().update, this, &Pulse::update);
		id = idCount++;
	}
	void update(ofEventArgs& update) {
		float curTime = ofGetElapsedTimef();
		float dt = curTime - startTime;
		currentAngle = startAngle + (direction ? +speed : -speed) * dt;
		int insideStatus = isInsideWrap(currentAngle, leftAngle, rightAngle, 360);
		if(insideStatus) {
			currentAngle = insideStatus > 0 ? rightAngle : leftAngle;
			direction = insideStatus > 0 ? false : true;
			startAngle = currentAngle;
			startTime = curTime;
			bounce();
		}
	}
	virtual float getAngle() const {
		return pulseCenter;
	}
	ofVec2f getCurrentNormal() {
		return ofVec2f(1, 0).rotate(getCurrentAngle());
	}
	void bounce() {
		float curTime = ofGetElapsedTimef();
		float timeSinceLastBounce = curTime - lastBounceTime;
		if(timeSinceLastBounce > debounceTime) {
			int channel = 9 + id;
			ofVec2f normal = getCurrentNormal();
			float pan = ofMap(normal.x, -1, 1, 0, 127, true);
			midi.sendControlChange(channel, 0, pan);
			midi.sendNote(channel, 64 + id * 4, 127, 500);
			lastBounceTime = curTime;
		}
	}
	float getCurrentAngle() {
		return currentAngle;
	}
};

int Pulse::idCount = 0;
	
class ofApp : public ofBaseApp {
public:
	vector<Person> people;
	vector<Pulse> pulses;
	
	void setup() {
		ofSetCircleResolution(64);
		ofSetLineWidth(2);
		ofSetLogLevel(OF_LOG_VERBOSE);
		
		MiniFont::setup();
		
		midi.listPorts();
		midi.openPort();
		
		people.resize(3);
		pulses.resize(3);
		for(int i = 0; i < people.size(); i++) {
			people[i].setup(ofVec2f(ofRandomWidth(), ofRandomHeight()));
			pulses[i].setup(people[i].getAngle());
		}
	}
	void update() {
		ofSort(people);
		ofSort(pulses);
		
		int n = people.size();
		
		for(int i = 0; i < n; i++) {
			int inext = (i + 1) % n;
			float a = people[i].getAngle(), b = people[inext].getAngle();
			if(b < a) {
				b += 360;
			}
			float avg = (a + b) / 2;
			avg = fmodf(avg, 360);
			people[i].rightAngle = avg;
			people[inext].leftAngle = avg;
		}
		
		// try to determine the relationship between the pulses and people
		int bestOffset = 0;
		float bestDistance = 0;
		ofVec2f curPulse = pulses[0].getNormal();
		for(int j = 0; j < n; j++) {
			ofVec2f curPerson = people[j].getNormal();
			float distance = curPerson.distance(curPulse);
			if(j == 0 || distance < bestDistance) {
				bestDistance = distance;
				bestOffset = j;
			}
		}
		int personOffset = bestOffset;
		
		for(int i = 0; i < n; i++) {
			int personIndex = (i + personOffset) % n;
			pulses[i].leftAngle = people[personIndex].leftAngle;
			pulses[i].rightAngle = people[personIndex].rightAngle;
			float a = pulses[i].leftAngle, b = pulses[i].rightAngle;
			if(b < a) {
				b += 360;
			}
			float avg = (a + b) / 2;
			avg = fmodf(avg, 360);
			pulses[i].pulseCenter = avg;
		}
	}
	void drawTriangle(ofVec2f center, ofVec2f p1, ofVec2f p2, float side) {
		p1 = center + (p1 - center).normalize() * side;
		p2 = center + (p2 - center).normalize() * side;
		ofTriangle(center, p1, p2);
	}
	void draw() {
		ofBackground(0);
		
		ofPushMatrix();
		ofTranslate(ofGetWidth() / 2, ofGetHeight() / 2);
		ofScale(pxToMm, pxToMm);
		ofRotate(45);
		drawCar();
		ofPopMatrix();
		
		ofPushMatrix();
		ofPushStyle();
		ofTranslate(ofGetWidth() / 2, ofGetHeight() / 2);
		int n = people.size();
		for(int i = 0; i < n; i++) {
			ofVec2f leftNormal(1, 0);
			leftNormal.rotate(people[i].leftAngle);
			ofSetColor(ofColor::cyan);
			ofLine(190 * leftNormal, 300 * leftNormal);
			ofVec2f normal = people[i].getNormal();
			ofSetColor(ofColor::yellow);
			ofLine(150 * normal, 190 * normal);
		}
		
		ofSetColor(ofColor::magenta);
		for(int i = 0; i < n; i++) {
			ofVec2f normal = pulses[i].getNormal();
			ofLine(190 * normal, 240 * normal);
			MiniFont::drawHighlight("Pulse " + ofToString(pulses[i].id), 240 * normal);
			ofVec2f currentNormal = pulses[i].getCurrentNormal();
			ofLine(150 * currentNormal, 190 * currentNormal);
		}
		ofPopStyle();
		ofPopMatrix();
	}
	void drawCar() {
		ofVec2f flt(-carWidth / 2 - tireWidth / 2, +tireSpacing - tireHeight), brt(carWidth / 2 - tireWidth / 2, -tireSpacing);
		ofVec2f topCenter = (flt + brt) / 2, topOffsetDirection = (brt - flt).normalize();
		ofVec2f topSensorCenter = topCenter + topOffsetDirection * offsetAmount;
		ofVec2f topSensorDirection = topOffsetDirection.rotate(-90);
		ofVec2f flb(-carWidth / 2 + tireWidth / 2, +tireSpacing), brb(carWidth / 2 + tireWidth / 2, -tireSpacing + tireHeight);
		ofVec2f bottomCenter = (flb + brb) / 2, bottomOffsetDirection = (flb - brb).normalize();
		ofVec2f bottomSensorCenter = bottomCenter + bottomOffsetDirection * offsetAmount;
		ofVec2f bottomSensorDirection = bottomOffsetDirection.rotate(-90);
		ofVec2f bl(+carWidth / 2 - tireWidth / 2, +tireSpacing);
		ofVec2f br(+carWidth / 2 + tireWidth / 2, +tireSpacing - tireHeight);
		ofVec2f tl(-carWidth / 2 - tireWidth / 2, -tireSpacing + tireHeight);
		ofVec2f tr(-carWidth / 2 + tireWidth / 2, -tireSpacing);
		
		ofPushStyle();
		ofSetColor(ofColor::yellow);
		ofLine(bottomSensorCenter, bottomSensorCenter + bottomSensorDirection * directionSize);
		ofLine(topSensorCenter, topSensorCenter + topSensorDirection * directionSize);
		ofPopStyle();
		
		ofPushStyle();
		ofFill();
		ofSetColor(ofColor::red, 64);
		drawTriangle(bottomSensorCenter, flb, bl, triangleSide);
		drawTriangle(bottomSensorCenter, brb, br, triangleSide);
		drawTriangle(topSensorCenter, flt, tl, triangleSide);
		drawTriangle(topSensorCenter, brt, tr, triangleSide);
		ofPopStyle();
		
		ofPushStyle();
		ofFill();
		ofSetColor(255);
		ofCircle(topSensorCenter, 50);
		ofCircle(bottomSensorCenter, 50);
		ofPopStyle();
		
		ofPushStyle();
		ofSetColor(255, 128);
		ofNoFill();
		ofEllipse(0, 0, outerEllipseWidth, outerEllipseHeight); // outer
		ofEllipse(0, 0, innerEllipseWidth, innerEllipseHeight); // inner
		ofSetRectMode(OF_RECTMODE_CENTER);
		ofRectRounded(0, 0, 4150, 1690, 300);
		ofRect(-carWidth / 2, +tireSpacing - tireHeight / 2, tireWidth, tireHeight);
		ofRect(+carWidth / 2, +tireSpacing - tireHeight / 2, tireWidth, tireHeight);
		ofRect(-carWidth / 2, -tireSpacing + tireHeight / 2, tireWidth, tireHeight);
		ofRect(+carWidth / 2, -tireSpacing + tireHeight / 2, tireWidth, tireHeight);
		ofPopStyle();
		
		ofPushMatrix();
		ofPushStyle();
		ofNoFill();
		ofSetColor(255, 128);
		ofCircle(spkRadius * spka, spkSize);
		ofCircle(spkRadius * spkb, spkSize);
		ofCircle(spkRadius * spkc, spkSize);
		ofCircle(spkRadius * spkd, spkSize);
		ofSetColor(255);
		ofDrawBitmapString("Speaker A", spkRadius * spka);
		ofDrawBitmapString("Speaker B", spkRadius * spkb);
		ofDrawBitmapString("Speaker C", spkRadius * spkc);
		ofDrawBitmapString("Speaker D", spkRadius * spkd);
		ofPopStyle();
		ofPopMatrix();
	}
	void keyPressed(int key) {
		if(key == 'f') {
			ofToggleFullscreen();
		}
	}
};

int main( ){
	ofSetupOpenGL(1024, 1024, OF_FULLSCREEN);
	ofRunApp(new ofApp());
}