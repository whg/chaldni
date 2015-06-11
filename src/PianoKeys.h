//
//  PianoKeys.h
//  lexus_plates
//
//  Created by Will Gallia on 08/06/2015.
//
//

#pragma once

#include "ofMain.h"
#include "Plate.h"

//static string names[] = { "C0", "C1", "C2", "C3", "C4", "C5", "C6", "C7", "C8", "C9", "C10" };
static string notes[] = { "C", "Cs", "D", "Ds", "E", "F", "Fs", "G", "Gs", "A", "As", "B" };
static int noteOrders[12] = { 1, 3, 6, 8, 10, 0, 2, 4, 5, 7, 9, 11, };
static int drawOrders[12] = { 5, 6, 7, 8, 9, 10, 11, 0, 1, 2, 3, 4, };

class PianoKeys {
public:
    struct Key {
        ofRectangle rect;
        bool on;
        int pitch;
        bool sharp;
        string name;
        ofColor onCol;
        static ofColor baseOnCol;

        Key() {}
        
        Key(float x, float y, float w, float h, int p, bool sharp=false):
            rect(x, y, w, h), pitch(p), sharp(sharp) {
            name = notes[p % 12] + ofToString(p/12);
            on = false;
            onCol = baseOnCol;
        }
        
        void draw() {

            if (on) {
                ofSetColor(onCol);
            }
            else {
                ofSetColor(10);
            }
            
            if (sharp || on) {
                ofFill();
                ofDrawRectangle(rect);
            }
            
            ofSetColor(10);
            ofNoFill();
            ofDrawRectangle(rect);

            
            if ((pitch) % 12 == 0) {
                ofSetColor(0);
                ofDrawBitmapString(name, rect.getPosition().x+2, rect.getPosition().y+rect.height-4);
            }
        }
    };

protected:
    ofRectangle rect;
    
    int nOctaves;
    float whiteKeyWidth;



public:
    Key *foundKey;
    vector< shared_ptr<Key> > keys;
    map<int, int> keyMap;
public:
    
    PianoKeys() {}

    PianoKeys(int startingOctave, int nOctaves, int x=0, int y = 0, int w=ofGetWidth(), int h=50):
        rect(x, y, w, h), nOctaves(nOctaves) {
        
        whiteKeyWidth = rect.width / nOctaves / 7.0;
        int startingNote = startingOctave * 12;
        
        for (int n = 0; n < nOctaves; n++) {

            int noteCounter = 0;
            float x = whiteKeyWidth * 7 * n;
            

            // do sharps first so they are found on mouse hits
            
            float bkw = whiteKeyWidth * 0.5;
            float bkh = h * 0.6;
            
            keys.push_back(shared_ptr<Key>(new Key(1*whiteKeyWidth + x - bkw*0.5, 0, bkw, bkh, startingNote + noteOrders[noteCounter++], true)));
            keys.push_back(shared_ptr<Key>(new Key(2*whiteKeyWidth + x - bkw*0.5, 0, bkw, bkh, startingNote + noteOrders[noteCounter++], true)));
            
            keys.push_back(shared_ptr<Key>(new Key(4*whiteKeyWidth + x - bkw*0.5, 0, bkw, bkh, startingNote + noteOrders[noteCounter++], true)));
            keys.push_back(shared_ptr<Key>(new Key(5*whiteKeyWidth + x - bkw*0.5, 0, bkw, bkh, startingNote + noteOrders[noteCounter++], true)));
            keys.push_back(shared_ptr<Key>(new Key(6*whiteKeyWidth + x - bkw*0.5, 0, bkw, bkh, startingNote + noteOrders[noteCounter++], true)));
            
            
            for (int i = 0; i < 7; i++) {
                keys.push_back(shared_ptr<Key>(new Key(i*whiteKeyWidth + x, 0, whiteKeyWidth, h, startingNote + noteOrders[noteCounter++])));
            }
            
            startingNote+= 12;
        }
        
        for (int i = 0; i < keys.size(); i++) {
            keyMap[keys[i]->pitch] = i;
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
        for (int i = 0; i < keys.size(); i++) {
//        for (auto &key : keys) {
            int bi = i / 12;
            keys[bi * 12 + drawOrders[i%12]]->draw();
        }
        ofPopStyle();
    }
    
    bool findKey(int x, int y) {
        if (rect.inside(x, y)) {
            for (auto &key : keys) {
                if (key->rect.inside(x, y)) {
                    foundKey = key.get();
                    cout << "found key " << key->name << endl;
                    return true;
                }
            }
        }
        foundKey = NULL;
        return false;
    }
    
    void highlightKeysForPlate(Plate *plate) {
    
        for (auto &key : keys) {
            for (auto &note : plate->midiNotes) {
                key->on = key->pitch == note.first;
                if (key->on) break;
                if (key->on) cout << "found pitch " << note.first << " " << key->rect.getX();
            }
        }
    }
    
    void highlightKeys(bool v) {
        for (auto &key : keys) {
            key->on = v;
        }
    }
};

class Channels : public PianoKeys {

public:
    class Channel : public Key {
    public:
        int number;
//        ofColor col;
        
        Channel(int n, float x, float y, float w, float h): PianoKeys::Key(x, y, w, h, 0) {
            name = "Channel " + ofToString(n);
            number = n;
            onCol = ofColor::fromHsb(float(n)/6.0 * 255, 120, 255);
            on = true;
        }
    };

public:
    Channels(int n=5, int x=0, int y = 0, int w=ofGetWidth(), int h=20) {
        rect = ofRectangle(x, y, w, h);
        
        int tw = float(w) / n;
        for (int i = 0; i < n; i++) {
            keys.push_back(shared_ptr<Key>(new Channel(i+1, i * tw + x, y, tw, h)));
        }
        
    }
    
    void draw() {
        ofPushStyle();
        ofSetColor(255);
        ofDrawRectangle(rect);
        for (auto &key : keys) {
            key->draw();
        }
        ofPopStyle();
    }
    
};