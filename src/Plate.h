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


class Plate {

public:
    float fadeInFrames;
    ofVec2f pos;
    float size;
    ofMesh mesh;
    ofShader shader;
    
    int patternNum, frameNum;
    
    int id;
    ofColor pickColour;
    
    bool listeningForNote;
    int noteOrders[MAX_NOTES];
    float tint;
    
    
    ofParameter<int> audioChannel;
    ofParameter<float> volume;
    ofParameter<bool> randomFigure;
    map<int, shared_ptr< ofParameter<bool> > > midiNotes;
    
    bool connecting;
    
public:
    ofxMaxiOsc osc;
    float play();
    
public:
    Plate(float x, float y, float w);
    ~Plate();
    
    void draw(render_t renderType=NORMAL, int noteOrder=0);
};

inline ofColor cidToOfColor(int cid) {
    return ofColor(cid / 65025, cid / 255, cid % 255);
}

inline int ofColorToCid(ofColor c) {
    return c.r * 65025 + c.g * 255 + c.b;
}


class PlateManager {
    
public:
    vector< shared_ptr<Plate> > plates;
    
public:
    float plateWidth, platePadding;
    ofFbo pickFbo;
    ofPixels pickPixels;
    
    map<int, int> colourMap;
    Plate *currentPlate;
    int currentEditingNoteOrder;
    bool listeningForNote;
    
    static bool drawSpecial;
    
    render_t currentRenderType;
    
public:
    PlateManager();
    
    virtual void setup(int w, int h);
    void draw(render_t renderType=NORMAL);
    virtual void update();

    void initPlates();
    Plate* getPlateAt(ofEasyCam &cam, int x, int y);

public:
    void editNoteOrders();
    ofxPanel savePanel;
    ofxButton saveButton;
    ofxGuiGroup savesGroup;
    vector< shared_ptr<ofxToggle> > savedFiles;
    void savedFilePressed(bool &b);
    
    void saveCurrentConfig();
    void loadConfig(string filename);
    void reloadSaves();
    
public:
    void keyPressed(ofKeyEventArgs &args);
    void keyReleased(ofKeyEventArgs &args);
    
};

class PlateManagerMidi : public PlateManager {
public:
    void setup(int w, int h);
    void update();
};