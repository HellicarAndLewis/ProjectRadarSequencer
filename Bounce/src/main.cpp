#include "ofMain.h"
#include "LedRing.h"
#include "ofxMidi.h"
ofxMidiOut midi;
ofVec2f makeRotatedVector(float angle) {
	return ofVec2f(1, 0).rotate(angle);
}
class MidiNote {
protected:
	float startTime;
	int noteLength;
public:
	int channel, pitch;
	MidiNote(int channel, int pitch, int noteLength)
	:channel(channel)
	,pitch(pitch)
	,noteLength(noteLength)
	,startTime(ofGetElapsedTimeMillis()) {
	}
	bool finished() {
		return (ofGetElapsedTimeMillis() - startTime) > noteLength;
	}
};
bool isFinished(MidiNote& note) {
	return note.finished();
}
class MidiWrapper : public ofThread {
public:
	MidiWrapper() {
		startThread();
	}
	void sendNote(int channel, int pitch, int velocity, int noteLength) {
		if(isThreadRunning()) {
			midi.sendNoteOn(channel, pitch, velocity);
			MidiNote note(channel, pitch, noteLength);
			lock();
			frontNotes.push_back(note);
			unlock();
		}
	}
protected:
	vector<MidiNote> backNotes, frontNotes;
	void swap() {
		if(frontNotes.size()) {
			backNotes.insert(backNotes.end(), frontNotes.begin(), frontNotes.end());
			frontNotes.clear();
		}		
	}
	void threadedFunction() {
		while(isThreadRunning()) {
			for(int i = 0; i < backNotes.size(); i++) {
				MidiNote& cur = backNotes[i];
				if(cur.finished()) {
					midi.sendNoteOff(cur.channel, cur.pitch, 0);
				}
			}
			ofRemove(backNotes, isFinished);
			lock();
			swap();
			unlock();
			ofSleepMillis(1);
		}
		// silence remaining notes
		swap();
		for(int i = 0; i < backNotes.size(); i++) {
			MidiNote& cur = backNotes[i];
			midi.sendNoteOff(cur.channel, cur.pitch, 0);
		}
	}
};
class Person {
public:
	ofVec2f position;
	Person(float x, float y) {
		position.set(x, y);
	}
	float getAngle() {
		return ofRadToDeg(atan2f(position.y, position.x));
	}
	float getRadius() {
		return position.length();
	}
};
class Pulse {
protected:
	float lastDiff, lastAngle;
	float speed, startAngle, startTime;
	bool clockwise;
	int index;
	void bounce() {
		midiWrapper->sendNote(1, 64 + index, 64, 200);
		startAngle = lastAngle;
		startTime = ofGetElapsedTimef();
		clockwise = !clockwise;
	}
public:
	static MidiWrapper* midiWrapper;
	Pulse(float startAngle, int index)
	:startAngle(startAngle)
	,index(index)
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
MidiWrapper* Pulse::midiWrapper;
int peopleRadius = 4;
class ofApp : public ofBaseApp {
public:
	LedRing ledRing;
	ofFbo fbo;
	ofPixels fboPixels;
	bool selected;
	int selectedIndex;
	vector<Person> people;
	vector<Pulse> pulses;
	MidiWrapper midiWrapper;
	void setup() {
		ledRing.setup();
		fbo.allocate(512, 512);
		fbo.begin();
		ofClear(0);
		fbo.end();
		ofSetCircleResolution(64);
		ofSetLineWidth(2);
		ofLog() << "MIDI Ports: " << ofToString(midi.getPortList());
		midi.openPort();
		Pulse::midiWrapper = &midiWrapper;
	}
	void update() {
		for(int i = 0; i < pulses.size(); i++) {
			pulses[i].update(people);
		}
		fbo.begin();
		ofPushStyle();
		ofSetLineWidth(30);
		ofSetColor(0, 10);
		ofFill();
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
		ofSetColor(255, 128);
		ofCircle(0, 0, 1 * ofGetWidth() / 4);
		ofCircle(0, 0, 2 * ofGetWidth() / 4);
		ofPopStyle();
		
		for(int i = 0; i < people.size(); i++) {
			ofSetColor(255);
			ofFill();
			ofCircle(people[i].position, peopleRadius);
			if(selected && selectedIndex == i) {
				ofSetColor(ofColor::red);
				ofNoFill();
				ofCircle(people[i].position, peopleRadius * 2);				
			}
			ofPushMatrix();
			ofRotate(people[i].getAngle());
			ofSetColor(255, 128);
			ofLine(1 * ofGetWidth() / 4, 0, people[i].getRadius(), 0);
			ofPopMatrix();
		}
		for(int i = 0; i < pulses.size(); i++) {
			ofPushMatrix();
			ofRotate(pulses[i].getAngle());
			ofSetColor(ofColor::red);
			ofLine(1 * ofGetWidth() / 4, 0, 2 * ofGetWidth() / 4, 0);
			ofPopMatrix();
		}
	}
	void mousePressed(int x, int y, int b) {
		if(!selected) {
			selected = true;
			selectedIndex = people.size();
			people.push_back(Person(x - ofGetWidth() / 2, y - ofGetHeight() / 2));
			pulses.push_back(Pulse(people.back().getAngle(), selectedIndex));
		}
	}
	void mouseDragged(int x, int y, int b) {
		people[selectedIndex].position.set(x - ofGetWidth() / 2, y - ofGetHeight() / 2);
	}
	void mouseMoved(int x, int y) {
		ofVec2f mouse(x - ofGetWidth() / 2, y - ofGetHeight() / 2);
		float nearest;
		int nearestIndex;
		for(int i = 0; i < people.size(); i++) {
			float distance = mouse.distance(people[i].position);
			if(i == 0 || distance < nearest) {
				nearest = distance;
				nearestIndex = i;
			}
		}
		if(people.size() && nearest < 2 * peopleRadius) {
			selectedIndex = nearestIndex;
			selected = true;
		} else {
			selected = false;
		}
	}
	void keyPressed(int key) {
		if(key == OF_KEY_BACKSPACE) {
			if(selected) {
				people.erase(people.begin() + selectedIndex);
			}
		}
	}
};
int main( ){
	ofSetupOpenGL(512, 512, OF_WINDOW);
	ofRunApp(new ofApp());
}