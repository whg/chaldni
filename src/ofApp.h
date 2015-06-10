#pragma once

#include "ofMain.h"

#include "ofxMidi.h"
#include "Plate.h"

#include "PianoKeys.h"

#include "ofxGui.h"
#include "ofxMaxim.h"

class ofApp : public ofBaseApp, public ofxMidiListener {
public:
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
    void dragEvent(ofDragInfo dragInfo);
    void gotMessage(ofMessage msg);
    
    ofEasyCam cam;
    
    
public:
    ofxMidiIn midiIn;
    void newMidiMessage(ofxMidiMessage& msg);
    
    
    PlateManager *pm, *plateManager;
    
//    ofxPanel panel;
//    ofxButton button;
//    ofxGuiGroup group;
//    ofParameter<float> xx;
//    ofxLabel label;
//    ofxToggle toggle;
    
    //vector<int> pitches;
    set<int> pitches;
    
    PianoKeys pianoKeys;

    render_t currentRenderType;
    
    void resetView();
    
protected:
    ofxPanel panel;
    ofxGuiGroup midiNotesGroup;
    vector< shared_ptr<ofxToggle> > midiNotes;
    Plate *currentPlate;
    void createPanel(Plate *plate, bool setPos=true, bool forPattern=false);
    bool showPanel;

    void valChange(bool &b);
    bool needsToRedrawPanel;
    
protected:
    bool connectMode, connecting;
    ofVec3f connectStart, connectEnd;
    Plate *connectedPlate;
    
    
protected:
    map<int, ofxMaxiOsc> generators;
    void audioOut(float *output, int bufferSize, int nChannels);
    
protected:
    PlateManagerMidi *noteToFigureManager;
};
