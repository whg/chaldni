#pragma once

#include "ofMain.h"

#include "ofxMidi.h"

#include "PlateManager.h"
#include "PianoKeys.h"

#include "ofxGui.h"
#include "ofxMaxim.h"
#include "Constants.h"

#include "ofxDmx.h"

class ofApp : public ofBaseApp, public ofxMidiListener {
public:
    ofApp();
    void setup();
    void exit();
    void update();
    void draw();
    
    void keyPressed(int key);
    void keyReleased(int key);
    void mouseMoved(int x, int y);
    void mouseDragged(int x, int y, int button);
    void mousePressed(int x, int y, int button);
    void mouseReleased(int x, int y, int button);
    void windowResized(int w, int h);
    
    ofEasyCam cam;
    
    
public:
    ofxMidiIn midiIn;
    void newMidiMessage(ofxMidiMessage& msg);
    
    
    PlateManager *pm, *plateManager;
    
    
    PianoKeys pianoKeys;
    Channels channels;

    render_t currentRenderType;
    
    void resetView();
    
protected:
    ofxPanel panel;
    ofxGuiGroup midiNotesGroup;
    vector< shared_ptr<ofxToggle> > midiNotes;
    ofxGuiGroup patternFreqsGroup;
    Plate *currentPlate;
    void createPanel(Plate *plate, bool setPos=true, bool forPattern=false);
    bool showPanel;
    bool noSelectionMode;
    
    void valChange(bool &b);
    bool needsToRedrawPanel;
    
    void reset();
    
protected:
    bool connectMode, connecting;
    ofVec3f connectStart, connectEnd;
    Plate *connectedPlate;
    
    
protected:
    map<int, ofxMaxiOsc> generators;
    void audioOut(float *output, int bufferSize, int nChannels);
    
protected:
    PlateManagerMidi *noteToFigureManager;
    
    
public:
    map<int, int> currentNotesPlaying;
    
    map< int, map<int, int> > playingPitches;
    map< int, map<int, vector<Plate*> > > playingPlates;
    
    map< int, pair<Plate*, char> > controlsToParameter;
    
protected:
    modes_t mode;
    
protected:
    ofSoundStream soundStream;
    ofMutex audioMutex;
    
protected:
    void blankAll();
    
    
public:
    ofxDmx dmx;
    string dmxDevice;
    
    ofMutex midiLock;
};
