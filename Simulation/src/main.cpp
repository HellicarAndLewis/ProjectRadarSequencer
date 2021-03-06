#include "ofMain.h"
#include "MiniFont.h"
#include "ofxMidi.h"
#include "ofxTiming.h"

// need to get rid of all screen units.

const float pxToMm = 1. / 10.;
float carWidth = 2600, tireWidth = 550, tireHeight = 180, tireSpacing = 845;
float directionSize = 15000, triangleSide = 50000;
float outerEllipseWidth = 4600, outerEllipseHeight = 3480;
float innerEllipseWidth = 3840, innerEllipseHeight = 2720;
float peopleRadius = 300 * pxToMm;
float offsetAmount = 600;
float debounceTime = .05;
float nearRange = 0, farRange = 3000;

ofVec2f spka(-1, -1), spkb(+1, -1), spkc(-1, +1), spkd(+1, +1);
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
	void sendNote(int channel, int pitch, int velocity = 64) {
		sendNoteOn(channel, pitch, velocity);
		sendNoteOff(channel, pitch, 0);
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
	float getDistance() const {
		ofVec2f curWorld = getWorldPosition();
		curWorld /= pxToMm;
		float theta = ofDegToRad(getAngle());
		ofVec2f curEllipse(cos(theta) * innerEllipseWidth, sin(theta) * innerEllipseHeight);
		curEllipse /= 2;
		return curWorld.length() - curEllipse.length(); 
	}
	virtual float getAngle() const {
		return ofVec2f(1, 0).angle(getWorldPosition());
	}
	int getPan() {
		return ofMap(getNormal().x, -1, 1, 0, 127);
	}
	int getPresence() {
		return ofMap(getDistance(), nearRange, farRange, 127, 0, true);
	}
	void update(ofEventArgs& e) {
		if(timer.tick()) {
			int channel = 1 + id;
			midi.sendControlChange(channel, 0, getPan());
			midi.sendControlChange(channel, 1, getPresence());
			midi.sendNote(channel, 64 + id * 6, 127);
		}
	}
	virtual void draw() {
		MiniFont::drawHighlight("Person " + ofToString(id), 0, 0);
		MiniFont::drawHighlight(ofToString((int) getDistance()) + "mm", 0, 10);
		MiniFont::drawHighlight("pan: " + ofToString(getPan()), 0, 20);
		MiniFont::drawHighlight("presence: " + ofToString(getPresence()), 0, 30);
	}
};

class Led {
private:
	float delay, state, lastTime;
public:
	float startAngle, stopAngle;
	Led()
	:startAngle(0)
	,stopAngle(0)
	,state(0)
	,delay(1) { // expose this on the gui, or smaller value for more users
	}
	void ping(float state = 1) {
		this->state = state;
		lastTime = ofGetElapsedTimef();
	}
	float getState() {
		float curState = state - ((ofGetElapsedTimef() - lastTime) / delay);
		return MAX(0, curState);
	}
};

float speed = 150;
class Pulse : public HasAngle {
private:
	bool bounced, passed;
	int lastPersonIndex;
	list<float> lastPersonDistance;
public:
	static int idCount;
	int id;
	float leftAngle, rightAngle, pulseCenter;
	bool direction;
	float startAngle, startTime;
	float lastBounceTime;
	float currentAngle;
	int personIndex;
	float personAngle;
	
	Pulse()
	:leftAngle(0)
	,rightAngle(0)
	,startAngle(0)
	,startTime(0)
	,lastBounceTime(0)
	,bounced(false)
	,passed(false)
	,direction(false)
	,personAngle(0)
	,lastPersonIndex(-1)
	,lastPersonDistance(0){
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

		if(personIndex != lastPersonIndex) {
			lastPersonDistance.clear();
		}
		float personDistance = distanceWrap(currentAngle, personAngle, 360);
		lastPersonDistance.push_back(personDistance);
		if(lastPersonDistance.size() > 3) {
			lastPersonDistance.pop_front();
			list<float>::iterator itr = lastPersonDistance.begin();
			float a = *(itr++), b = *(itr++), c = *(itr++);
			if(a > b && c > b) {
				pass();
			}
		}
		lastPersonIndex = personIndex;
	}
	virtual float getAngle() const {
		return pulseCenter;
	}
	ofVec2f getCurrentNormal() {
		return ofVec2f(1, 0).rotate(getCurrentAngle());
	}
	bool getPassed() {
		bool lastPassed = passed;
		passed = false;
		return lastPassed;
	}
	bool getBounced() {
		bool lastBounced = bounced;
		bounced = false;
		return lastBounced;
	}
	void pass() {
		int channel = 9 + id;
		midi.sendNote(channel, 64 + id * 4, 127);
		passed = true;
	}
	void bounce() {
		float curTime = ofGetElapsedTimef();
		float timeSinceLastBounce = curTime - lastBounceTime;
		if(timeSinceLastBounce > debounceTime) {
			int channel = 9 + id;
			ofVec2f normal = getCurrentNormal();
			float pan = ofMap(normal.x, -1, 1, 0, 127, true);
			midi.sendControlChange(channel, 0, pan);
			//midi.sendNote(channel, 64 + id * 4, 127);
			lastBounceTime = curTime;
			bounced = true;
		}
	}
	float getCurrentAngle() {
		return currentAngle;
	}
};

void drawArcGlow(float x, float y, float w, float h, float start, float stop, int steps = 4, int resolution = 32) {
	ofStyle style = ofGetStyle();
	ofPath path;
	path.setFilled(false);
	float theta  = ofDegToRad(start);
	path.moveTo(cos(theta) * w, sin(theta) * h);
	path.arc(ofVec2f(0, 0), w, h, start, stop, true);
	path.setCircleResolution(style.circleResolution);
	path.setColor(ofColor(style.color, 255 / steps));
	for(int i = 0; i < steps; i++) {
		path.setStrokeWidth(ofMap(i, 0, steps - 1, 2, style.lineWidth));
		path.draw();
	}
}

int Pulse::idCount = 0;

class ofApp : public ofBaseApp {
public:	
	vector<Person> people;
	vector<Pulse> pulses;
	vector<Led> leds;

	void setup() {
		ofSetCircleResolution(64);
		ofSetLineWidth(2);
		ofSetLogLevel(OF_LOG_VERBOSE);
		
		MiniFont::setup();
		
		midi.listPorts();
		midi.openPort();
		
		people.resize(2);
		pulses.resize(2);
		for(int i = 0; i < people.size(); i++) {
			people[i].setup(ofVec2f(ofRandomWidth(), ofRandomHeight()));
			pulses[i].setup(people[i].getAngle());
		}
		
		leds.resize(4);
		float ledAngle = 360 / leds.size();
		for(int i = 0; i < leds.size(); i++) {
			leds[i].startAngle = i * ledAngle;
			leds[i].stopAngle = (i + 1) * ledAngle;
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
			pulses[i].personIndex = personIndex;
			Person& person = people[personIndex];
			pulses[i].personAngle = person.getAngle();
			pulses[i].leftAngle = person.leftAngle;
			pulses[i].rightAngle = person.rightAngle;
			float a = pulses[i].leftAngle, b = pulses[i].rightAngle;
			if(b < a) {
				b += 360;
			}
			float avg = (a + b) / 2;
			avg = fmodf(avg, 360);
			pulses[i].pulseCenter = avg;
			
			if(pulses[i].getPassed()) {
				for(int j = 0; j < leds.size(); j++) {
					int inside = isInsideWrap(person.getAngle(), leds[j].startAngle, leds[j].stopAngle, 360);
					if(inside == 0) {
						leds[j].ping();
					}
				}
			}
		}
		
		// get average presence
		float presence = 0;
		for(int i = 0; i < n; i++) {
			presence += people[i].getPresence();
		}
		presence /= n;
		midi.sendControlChange(1, 2, (int) presence);
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
			//ofLine(190 * currentNormal, people[pulses[i].personIndex].getNormal() * 190);
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
		ofPushStyle();
		for(int j = 0; j < 2; j++) {
			int w, h;
			if(j == 0) {
				w = outerEllipseWidth / 2, h = outerEllipseHeight / 2;
			} else {
				w = innerEllipseWidth / 2, h = innerEllipseHeight / 2;
			}
			ofSetLineWidth(16);
			for(int i = 0; i < leds.size(); i++) {
				ofSetColor(ofColor(0, 0, 255 * leds[i].getState()));
				drawArcGlow(0, 0, w, h, leds[i].startAngle, leds[i].stopAngle);
			}
		}
		ofPopStyle();
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