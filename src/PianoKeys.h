//
//  PianoKeys.h
//  lexus_plates
//
//  Created by Will Gallia on 08/06/2015.
//
//

#pragma once

#include "ofMain.h"

//static string names[] = { "C0", "C1", "C2", "C3", "C4", "C5", "C6", "C7", "C8", "C9", "C10" };
static string notes[] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
static int noteOrders[12] = { 1, 3, 6, 8, 10, 0, 2, 4, 5, 7, 9, 11 };

class PianoKeys {
public:
    struct Key {
        ofRectangle rect;
        bool on;
        int pitch;
        bool sharp;
        string name;
        
        Key(float x, float y, float w, float h, int p, bool sharp=false):
            rect(x, y, w, h), pitch(p), sharp(sharp) {
            name = notes[p % 12] + ofToString(p/12);
        }
        
        void draw() {
            ofSetColor(10);
            if (sharp) {
                ofFill();
            }
            else {
                ofNoFill();
            }
            
            ofDrawRectangle(rect);

            
            if ((pitch) % 12 == 0) {
                ofSetColor(0);
                ofDrawBitmapString("C" + ofToString(pitch/12), rect.getPosition().x+1, rect.getPosition().y+rect.height-2);
            }
        }
    };

protected:
    ofRectangle rect;
    
    int nOctaves;
    float whiteKeyWidth;
    vector<Key> keys;

public:
    Key *foundKey;
    
public:
    
    PianoKeys() {}

    PianoKeys(int startingOctave, int nOctaves, int x=0, int y = 0, int w=ofGetWidth(), int h=50):
        rect(x, y, w, h), nOctaves(nOctaves) {
        
        whiteKeyWidth = rect.width / nOctaves / 7.0;
        int startingNote = startingOctave * 12;
        
        for (int n = 0; n < nOctaves; n++) {

            int noteCounter = 0;
            float x = whiteKeyWidth * 7 * n;
            
            float bkw = whiteKeyWidth * 0.5;
            float bkh = h * 0.6;
            
            keys.push_back(Key(1*whiteKeyWidth + x - bkw*0.5, 0, bkw, bkh, startingNote + noteOrders[noteCounter++], true));
            keys.push_back(Key(2*whiteKeyWidth + x - bkw*0.5, 0, bkw, bkh, startingNote + noteOrders[noteCounter++], true));
            
            keys.push_back(Key(4*whiteKeyWidth + x - bkw*0.5, 0, bkw, bkh, startingNote + noteOrders[noteCounter++], true));
            keys.push_back(Key(5*whiteKeyWidth + x - bkw*0.5, 0, bkw, bkh, startingNote + noteOrders[noteCounter++], true));
            keys.push_back(Key(6*whiteKeyWidth + x - bkw*0.5, 0, bkw, bkh, startingNote + noteOrders[noteCounter++], true));
            
            
            for (int i = 0; i < 7; i++) {
                keys.push_back(Key(i*whiteKeyWidth + x, 0, whiteKeyWidth, h, startingNote + noteOrders[noteCounter++]));
            }
            
            
            
            startingNote+= 12;
        }
        
            foundKey = NULL;
    }
    
    void setNOctaves(int n) {
        nOctaves = n;
        whiteKeyWidth = rect.width / nOctaves / 7.0;
    }
    
    void draw() {
        ofPushStyle();
        ofSetColor(255);
        ofDrawRectangle(rect);
        for (auto &key : keys) {
            key.draw();
        }
        ofPopStyle();
    }
    
    bool findKey(int x, int y) {
        if (rect.inside(x, y)) {
            for (auto &key : keys) {
                if (key.rect.inside(x, y)) {
                    foundKey = &key;
                    cout << "found key " << key.name << endl;
                    return true;
                }
            }
        }
        foundKey = NULL;
        return false;
    }
};