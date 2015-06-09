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

#define MAX_NOTES 9

enum render_t {
    NORMAL,
    PICK,
    NOTE_SELECT,
};


class Plate {

public:
    float fadeInFrames;
    ofVec2f pos;
    float size;
    ofMesh mesh;
    ofShader shader;
    
    int figureNum, frameNum;
    
    int id;
    ofColor pickColour;
    
    bool listeningForNote;
    int noteOrders[MAX_NOTES];
    float tint;
    
    
    
    ofParameter<int> audioChannel;
    ofParameter<float> volume;
    map<int, shared_ptr< ofParameter<bool> > > midiNotes;
    
    bool connecting;
    
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
    
public:
    PlateManager(): plateWidth(128), platePadding(10) {
        ofRegisterKeyEvents(this);
        currentPlate = NULL;
        currentEditingNoteOrder = 0;
        listeningForNote = false;
        
        drawSpecial = false;
    }
    
    void setup(int w, int h) {
        float pd = plateWidth + platePadding;
    
        plates.clear();
        for (int i = 0; i < w; i++) {
            for (int j = 0; j < h; j++) {
                float x = j * pd - (w * 0.5 * pd) + plateWidth * 0.5;
                float y = i * pd - (h * 0.5 * pd) + plateWidth * 0.5;
                plates.push_back(shared_ptr<Plate>(new Plate(x, y, plateWidth)));
            }
        }
        
        int counter = 0;
        for (auto p : plates) {
            p->id = counter++;
            int c2 = counter * 5;
            p->pickColour = cidToOfColor(c2);
            colourMap[c2]  = p->id;
        }
        
        pickFbo.allocate(ofGetWidth(), ofGetHeight(), GL_RGB);
        pickFbo.begin();
        ofClear(0);
        pickFbo.end();
        
        pickPixels.allocate(pickFbo.getWidth(), pickFbo.getHeight(), OF_PIXELS_RGB);
        
    }
    
    void draw(render_t renderType=NORMAL) {

        if (renderType == NOTE_SELECT) {
            editNoteOrders();
            return;
        }
        
        for (auto p : plates) {
            p->draw(renderType);
        }
        

    }
    
    Plate* getPlateAt(ofEasyCam &cam, int x, int y) {
        
        pickFbo.begin();
        ofBackground(0);
        
        cam.begin();
        draw(PICK);
        cam.end();
        pickFbo.end();
        
        pickFbo.readToPixels(pickPixels);
        
        ofColor col = pickPixels.getColor(x, y);
        int cid = ofColorToCid(col);
        
        if (colourMap.count(cid) > 0) {
            cout << "found id: " << colourMap[cid] << endl;
            return plates[colourMap[cid]].get();
        }

        cout << "not found" << endl;
        return NULL;
    }
    
    void editNoteOrders() {
    
        float space = 600;
        int end = drawSpecial ? MAX_NOTES-1 : currentEditingNoteOrder;
        for (int i = end, j = 0; i >= currentEditingNoteOrder; i--, j++) {
            ofPushMatrix();
            ofTranslate(0, 0, (end+1 - currentEditingNoteOrder -1)*-space + j * space);

            bool doneLabel = false;
            
            for (auto p : plates) {
                p->listeningForNote = p->listeningForNote && i == currentEditingNoteOrder;
                p->tint = currentEditingNoteOrder*(1.0 / (end+1)) +  (j+1) * (1.0 / (end+1));
                p->draw(NOTE_SELECT, i);

                if (!doneLabel && i == currentEditingNoteOrder) {
                    ofSetColor(255);
                    ofDrawBitmapString("Editing Layer " + ofToString(currentEditingNoteOrder+1), p->pos - ofVec2f(p->size, p->size+20));
                    doneLabel = true;
                }
            }

            
            ofPopMatrix();
        }
        

//        ofDrawBitmapString(, 10, ofGetHeight() - 10);
    }
    
    
    void keyPressed(ofKeyEventArgs &args) {
        int key = args.key;
        if (listeningForNote && key > int('0') && key <= int('9')) {
            currentPlate->noteOrders[currentEditingNoteOrder] = key - int('0');
            currentPlate->listeningForNote = false;
            listeningForNote = false;
        }
        
        KEY(OF_KEY_UP, currentEditingNoteOrder = MIN(MAX_NOTES-1, currentEditingNoteOrder+1))
        KEY(OF_KEY_DOWN, currentEditingNoteOrder = MAX(0, currentEditingNoteOrder-1))
    }
    
    void keyReleased(ofKeyEventArgs &args) {
        
    }
};