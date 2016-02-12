//
//  Plate.h
//  lexus_plates
//
//  Created by Will Gallia on 08/06/2015.
//
//

#pragma once

#include "ofMain.h"
#include "ofxGui.h"
#include "ofxTools.h"
#include "ofxMaxim.h"

#include "Constants.h"

#define BLEND_FRAMES 30.0
#define FREQ_ADJUST_AMOUNT 50

class Plate {

    int patternNum, frameNum;


public:
    ofVec2f pos;
    ofVec2f size;
    ofMesh mesh;
    ofShader shader;
    
    
    int id;
    ofColor pickColour;
    
    bool listeningForNote;
    int noteOrders[MAX_NOTES+1];
    float tint;
    
    int getPatternNum() { return patternNum; }
    void setPatternNum(int n);
    
    ofParameter<int> audioChannel;
    ofParameter<float> volume;
    ofParameter<bool> randomFigure;
    map<int, shared_ptr< ofParameter<bool> > > midiNotes;
    map<int, shared_ptr< ofParameter<bool> > > channels;
    
    bool connecting, border;
    
    ofParameter<ofColor> dmxColour;
    ofParameter<float> fadeIn, fadeOut;
    float value;
    
    float lastFrameTime;
    float turnedOnAt, turnedOffAt;
    float valueAtOff;
    
    void dmxColourChangedBefore(ofColor &c) {
        cout << "dmx col b4" << endl;
    }
    
    void dmxColourChangedAfter(ofColor &c) {
        cout << "dmx col after" << endl;
    }

    
public:
    ofxMaxiOsc osc;
    float play();
    void stop();
    ofParameter<bool> playing;

    vector< ofParameter<float> > patternFrequencies;
    
public:
    Plate(float x, float y, float w, float h);
    ~Plate();
    
    void draw(render_t renderType=NORMAL, int noteOrder=0);
    void update();
    
protected:
    int lastPatternNum, frameSinceChange;
};


void initAssets();
void delAssets();