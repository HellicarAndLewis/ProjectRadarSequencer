#pragma once

#include "ofMain.h"
#include "ofxMidi.h"

class Sequencer : public ofThread {
protected:
	ofxMidiOut midiOut;
	
	vector<unsigned char> notes;
	unsigned long startTime;
	unsigned int channel;
	unsigned int rows, cols;
	float measureLength, colLength;
	vector<bool> notesOn;
	vector<vector<bool> > threadedBuffer, middleBuffer, updateBuffer;
	
	void trigger(unsigned int col) {
		for(int i = 0; i < rows; i++) {
			if(notesOn[i]) {
				midiOut.sendNoteOff(channel, notes[i], 0);
				notesOn[i] = false;
			}
			if(threadedBuffer[i][col]) {
				midiOut.sendNoteOn(channel, notes[i], 100);
				notesOn[i] = true;
			}
		}
	}
	void threadedFunction() {
		midiOut.listPorts();
		midiOut.openPort();
		unsigned int lastCol = 0;
		while(isThreadRunning()) {
			unsigned int curCol = ((unsigned int) getPosition()) % cols;
			if(curCol != lastCol) {
				trigger(curCol);
			}
			lastCol = curCol;
			lock();
			threadedBuffer = middleBuffer;
			unlock();
			ofSleepMillis(1); // could use an approximation of how much time is left
		}
	}
public:
	void setup(vector<unsigned char>& notes, unsigned int cols, float bpm = 120, unsigned int channel = 1) {				
		this->notes = notes;
		this->rows = notes.size();
		this->cols = cols;
		this->channel = channel;
		middleBuffer.resize(rows, vector<bool>(cols, false));
		updateBuffer = middleBuffer;
		threadedBuffer = middleBuffer;
		notesOn.resize(rows, false);
		setBpm(bpm);
		startTime = ofGetSystemTime();
		startThread(true, false);
	}
	void setBpm(float bpm) {
		measureLength = 1000 * 60 * 4 / bpm;
		colLength = measureLength / cols;
	}
	void setState(int row, int col, bool state) {
		if(row >= 0 && row < rows && col >= 0 && col < cols) {
			updateBuffer[row][col] = state;
		}
	}
	bool getState(int row, int col) const {
		if(row >= 0 && row < rows && col >= 0 && col < cols) {
			return updateBuffer[row][col];
		} else {
			return false;
		}
	}
	void clearStates() {
		for(int i = 0; i < rows; i++) {
			for(int j = 0; j < cols; j++) {
				updateBuffer[i][j] = false;
			}
		}
	}
	void update() {
		lock();
		middleBuffer = updateBuffer;
		unlock();
	}
	float getPosition() const {
		unsigned long curTime = ofGetSystemTime();
		unsigned long timeDiff = curTime - startTime;
		return fmod(timeDiff, measureLength) * (cols / measureLength);
	}
	unsigned int getRows() const {
		return rows;
	}
	unsigned int getCols() const {
		return cols;
	}
};