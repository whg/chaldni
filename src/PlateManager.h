//
//  PlateManager.h
//  lexus_plates
//
//  Created by Will Gallia on 15/06/2015.
//
//

#pragma once

#include "Plate.h"

#define MIDI_CONFIG_FILENAME "patternConfig.xml"

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
    float getPlateGap() { return plateWidth + platePadding; }
    
    ofFbo pickFbo;
    ofPixels pickPixels;
    
    map<int, int> colourMap;
    Plate *currentPlate;
    int currentEditingNoteOrder;
    bool listeningForNote;
    
    static bool drawSpecial;
    
    render_t currentRenderType;
    int width, height;
    
public:
    PlateManager();
    ~PlateManager();
    
    virtual void setup(int w, int h);
    void draw(render_t renderType=NORMAL);
    virtual void update();
    
    void initPlates();
    void initBuffers(int w=ofGetWidth(), int h=ofGetHeight());
    Plate* getPlateAt(ofEasyCam &cam, int x, int y);
    
public:
    void editNoteOrders();
    ofxPanel savePanel;
    ofxButton saveButton;
    ofxGuiGroup savesGroup;
    vector< shared_ptr<ofxToggle> > savedFiles;
    void savedFilePressed(bool &b);
    
    virtual void saveCurrentConfig(string filename="main.xml");
    virtual void loadConfig(string filename="main.xml");
    
    void reloadSaves();
    
public:
    virtual void keyPressed(ofKeyEventArgs &args);
    virtual void keyReleased(ofKeyEventArgs &args);
    
};

class PlateManagerMidi : public PlateManager {
public:
    void setup(int w, int h);
    void update();
    
    void saveCurrentConfig(string filename=MIDI_CONFIG_FILENAME);
    void loadConfig(string filename=MIDI_CONFIG_FILENAME);
    
    void keyPressed(ofKeyEventArgs &args);
    void keyReleased(ofKeyEventArgs &args);
};
