//
//  Plate.cpp
//  lexus_plates
//
//  Created by Will Gallia on 08/06/2015.
//
//

#include "Plate.h"
#include "ChladniDB.h"
#include "PianoKeys.h"

#define NUM_FRAMES 26


static ofImage *imgMap = NULL;
static ofTrueTypeFont *font = NULL;
int pc = 0;

static int audioChannelCounter = 0;

void initAssets() {
    
    if (!font) {
        cout << "loading font, font = " << font << endl;
        font = new ofTrueTypeFont();
        font->load("/Library/Fonts/Verdana.ttf", 24, false, true, true);
    }
    
    if (!imgMap) {
        imgMap = new ofImage();
//        imgMap->load("m256-500-blank.png");
//        imgMap->load("singles2.png");
    }
}

void delAssets() {
    delete font;
    delete imgMap;
}

Plate::Plate(float x, float y, float w, float h): pos(x, y), size(w, h), listeningForNote(false), tint(false) {
    
    mesh = ofMesh::plane(w, h);
    
    patternNum = 0;
    setPatternNum(0);
    frameNum = 0;

    
    for (int i = 1; i <= MAX_NOTES; i++) {
        noteOrders[i] = i;
    }
    
    audioChannel.set("audio channel", audioChannelCounter++, 0, 6);
    volume.set("volume", 0.5, 0.0, 1.0);
    randomFigure.set("randomFigure", false);
    
    for (int i = 0; i < NUM_FIGURES; i++) {
        CPattern pattern = ChladniDB::patternForNo(i);
        float freq = pattern.data.frequency;
        patternFrequencies.push_back(ofParameter<float>(ofToString(i), freq, freq-FREQ_ADJUST_AMOUNT, freq+FREQ_ADJUST_AMOUNT));
    }
    
    playing.set("on", true);
    
    connecting = false;
    border = false;
    
    dmxColour.set("colour", 0, ofColor(0,0),255);
    fadeIn.set("Fade in", 0.5, 0, 2.5);
    fadeOut.set("Fade out", 0.5, 0, 2.5);
    value = 0.0f;
    lastFrameTime = 0;
    turnedOnAt = 0;
    turnedOffAt = 0;
    valueAtOff = 0;
    
    dmxColour.addListener(this, &Plate::dmxColourChangedBefore);
//    dmxColour.addListener(this, &Plate::dmxColourChangedAfter);
}

Plate::~Plate() {
}

void Plate::update() {
    
    float now = ofGetElapsedTimef();
    float timedelta =  - lastFrameTime;
    
    float a = ofMap(dmxColour->a, 0, 255, 0, 1);
    if (!playing) {
        
//        if (fadeOut < 0.0001) {
//            value = 0;
//        }
//        else {
            value = valueAtOff * (1.0 - MIN(1.0, (now - turnedOffAt) / fadeOut)) * a;
//          }
        
    }
    else {
        
        value = MIN(1.0, (now - turnedOnAt) / fadeIn) * a;
    }
    

    
    lastFrameTime = ofGetElapsedTimef();
}

void Plate::draw(render_t renderType, int noteOrder) {
    
    
    ofEnableNormalizedTexCoords();

    ofPushMatrix();
    ofTranslate(pos);

    
    if (renderType == PICK) {// || renderType == NORMAL) {
        
        ofSetColor(pickColour);
        mesh.draw(OF_MESH_FILL);
        ofPopMatrix();
        
    }
    else if (renderType == NORMAL) {

        if (value < 0.00001) ofSetColor(200);
        else ofSetColor(dmxColour.get() * value);
        
        mesh.draw(OF_MESH_FILL);
        ofSetColor(255);
        
        string sid = ofToString(id + 1);
        ofRectangle r = font->getStringBoundingBox(sid, 0, 0);
        font->drawStringAsShapes(sid, r.getWidth() * -0.5, r.getHeight() * -0.5);


        if (connecting) {
            ofSetColor(100, 255, 160, 200);
            mesh.draw(OF_MESH_FILL);
        }
        
        
        if (border) {
            ofSetColor(255, 255, 0);
            mesh.drawWireframe();
        }
        
        
    }

    
    
    ofPopMatrix();

}

float Plate::play() {

    turnedOnAt = ofGetElapsedTimef();
    playing = true;
    
    return 0.0f;
}

void Plate::stop() {
    playing = false;
    turnedOffAt = ofGetElapsedTimef();
    valueAtOff = value;
}

void Plate::setPatternNum(int n) {

    lastPatternNum = patternNum;
    patternNum = n;
    
    frameSinceChange = 0;
    
    playing = true;
}
