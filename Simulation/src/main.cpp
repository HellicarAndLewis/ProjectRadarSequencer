#include "ofMain.h"
#include "MiniFont.h"

float pxToMm = 1. / 10.;
float carWidth = 2600, tireWidth = 550, tireHeight = 180, tireSpacing = 845;
float directionSize = 10000, triangleSide = 50000;
float outerEllipseWidth = 4600, outerEllipseHeight = 3480;
float innerEllipseWidth = 3840, innerEllipseHeight = 2720;
float peopleRadius = 10;
float offsetAmount = 600;

class DraggableCircle {
protected:
	bool selected;
	ofVec2f position;
	static int idCount;
	int id;
public:
	DraggableCircle()
	:selected(false) {
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
		mouseMoved(e);
	}
	void mouseMoved(ofMouseEventArgs& e) {
		selected = position.distance(e) < peopleRadius;
	}
	void mouseDragged(ofMouseEventArgs& e) {
		if(selected) {
			position.set(e);
		}
	}
	virtual void draw(ofEventArgs& e) {
		ofPushStyle();
		ofFill();
		ofSetColor(255);
		ofCircle(position, peopleRadius);
		if(selected) {
			ofSetColor(ofColor::red);
			ofNoFill();
			ofCircle(position, peopleRadius * 2);				
		}
		MiniFont::drawHighlight(ofToString(id), position);
		ofPopStyle();
	}
};

int DraggableCircle::idCount = 0;

class Person : public DraggableCircle {
public:
	float leftAngle, rightAngle;
	float startAngle, startTime;
	float pulseAngle;
	bool direction;
	Person()
	:leftAngle(0)
	,rightAngle(0)
	,startAngle(0)
	,startTime(0),
	direction(false) {
	}
	virtual void setup(ofVec2f position) {
		DraggableCircle::setup();
		this->position = position;
		ofAddListener(ofEvents().update, this, &Person::update);
	}
	void update(ofEventArgs& update) {
		float a = leftAngle, b = rightAngle;
		if(b < a) {
			b += 360;
		}
		pulseAngle = MAX(pulseAngle, leftAngle);
		pulseAngle = MIN(pulseAngle, rightAngle);
	}
	float getPulseAngle() {
		return pulseAngle;
	}
	ofVec2f getWorldPosition() const {
		return position - ofGetWindowSize() / 2;
	}
	float getAngle() const {
		return getWorldPosition().angle(ofVec2f(1, 0));
	}
};

bool operator < (const Person& a, const Person& b) {
	return a.getAngle() < b.getAngle();
}

class ofApp : public ofBaseApp {
public:
	vector<Person> people;
	
	void setup() {
		ofSetCircleResolution(64);
		ofSetLineWidth(2);
		
		MiniFont::setup();
		
		people.resize(5);
		for(int i = 0; i < people.size(); i++) {
			people[i].setup(ofVec2f(ofRandomWidth(), ofRandomHeight()));
		}
	}
	void update() {
		ofSort(people);
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
		ofSetColor(ofColor::cyan);
		ofTranslate(ofGetWidth() / 2, ofGetHeight() / 2);
		int n = people.size();
		for(int i = 0; i < n; i++) {
			int inext = (i + 1) % n;
			float a = people[i].getAngle();
			float b = people[inext].getAngle();
			if(b < a) {
				b += 360;
			}
			float avg = (a + b) / 2;
			avg = fmodf(avg, 360);
			people[i].rightAngle = avg;
			people[inext].leftAngle = avg;
			ofLine(ofVec2f(100, 0).rotate(-avg), ofVec2f(300, 0).rotate(-avg));
		}
		ofSetColor(ofColor::magenta);
		for(int i = 0; i < n; i++) {
			float theta = people[i].getPulseAngle();
			ofLine(ofVec2f(150, 0).rotate(-theta), ofVec2f(250, 0).rotate(-theta));
		}
		ofPopStyle();
		ofPopMatrix();
		
		MiniFont::drawHighlight(ofToString(mouseX) + " " + ofToString(mouseY), 10, 10);
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
		ofRectRounded(0, 0, 4100, 1690, 300);
		ofRect(-carWidth / 2, +tireSpacing - tireHeight / 2, tireWidth, tireHeight);
		ofRect(+carWidth / 2, +tireSpacing - tireHeight / 2, tireWidth, tireHeight);
		ofRect(-carWidth / 2, -tireSpacing + tireHeight / 2, tireWidth, tireHeight);
		ofRect(+carWidth / 2, -tireSpacing + tireHeight / 2, tireWidth, tireHeight);
		ofPopStyle();
	}
};

int main( ){
	ofSetupOpenGL(1280, 720, OF_WINDOW);
	ofRunApp(new ofApp());
}