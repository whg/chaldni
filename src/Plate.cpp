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

#define NUM_FRAMES 5
#define NUM_FIGURES 11

static ofImage *imgMap = NULL;
static ofTrueTypeFont *font = NULL;
int pc = 0;

static int audioChannelCounter = 0;

void initAssets() {
    
    if (!font) {
        cout << "loading font, font = " << font << endl;
        font = new ofTrueTypeFont();
        font->load("/Library/Fonts/Arial.ttf", 48, false, true, true);
    }
    
    if (!imgMap) {
        imgMap = new ofImage();
        imgMap->load("m1.png");
    }
}

void delAssets() {
    delete font;
    delete imgMap;
}

Plate::Plate(float x, float y, float w): pos(x, y), size(w), listeningForNote(false), tint(false) {
    
    fadeInFrames = 5;
    
    mesh = ofMesh::plane(w, w);
    
    patternNum = 0;

    /////////////////////////////////////////////////////////
    // vertex shader
    
    string vert = GLSL120(
                          
                          void main() {
                              
                              gl_Position = ftransform();
                              
                              gl_TexCoord[0] = gl_TextureMatrix[0] * gl_MultiTexCoord0;
                              
                          }
                          );
    
    shader.setupShaderFromSource(GL_VERTEX_SHADER, vert);
    
    /////////////////////////////////////////////////////////
    // fragment shader
    
    string frag = GLSL120(
                          varying vec2 texc;
                          
                          uniform int figure;
                          uniform int frame;
                          uniform sampler2DRect tex;
                          uniform vec2 size = vec2(256.0);
                        
                          void main() {
                              vec4 c = texture2DRect(tex, vec2(gl_TexCoord[0].x / 5.0 + float(frame) * size.x,
                                                               gl_TexCoord[0].y / 11.0 + float(figure) * size.y));
                              gl_FragColor = c; //vec4(texc.x, texc.y, 1.0, 1.0); //c; //vec4(1.0, 0.0, 1.0, 1.0);
                              
                          }
                          );
    
    shader.setupShaderFromSource(GL_FRAGMENT_SHADER, frag);
    shader.linkProgram();
    
    for (int i = 1; i <= MAX_NOTES; i++) {
        noteOrders[i] = i;
    }
    
    audioChannel.set("audio channel", audioChannelCounter++, 0, 6);
    volume.set("volume", 0.5, 0.0, 1.0);
    randomFigure.set("randomFigure", false);
    
    for (int i = 0; i < NUM_FIGURES; i++) {
        CPattern pattern = ChladniDB::patternForNo(i);
        float freq = pattern.data.frequency;
        patternFrequencies.push_back(shared_ptr<ofParameter<float> >(new ofParameter<float>(ofToString(i), freq, freq-10, freq+10)));
    }
    
    
    connecting = false;
}

Plate::~Plate() {
//    if (imgMap) {
//        delete imgMap;
//        imgMap = NULL;
//    }
//    if (font) {
//        delete font;
//        font = NULL;
//    }
}

void Plate::draw(render_t renderType, int noteOrder) {
    
    
    ofEnableNormalizedTexCoords();

    ofPushMatrix();
    ofTranslate(pos);
//    ofScale(1, -1);

    
    if (renderType == PICK) {// || renderType == NORMAL) {
        
        ofSetColor(pickColour);
        mesh.draw(OF_MESH_FILL);
        
    }
    else if (renderType == NOTE_SELECT) {

        ofColor c(255, listeningForNote ? 0 : 255, listeningForNote ? 100 : 255);
        c*= tint;
        ofSetColor(c);
        mesh.draw(OF_MESH_FILL);
        ofSetColor(0);
        
        string sid = ofToString(noteOrders[noteOrder]);
        ofRectangle r = font->getStringBoundingBox(sid, 0, 0);
        font->drawStringAsShapes(sid, r.getWidth() * -0.5, r.getHeight() * -0.5);

        
    }
    else if (renderType == NORMAL || renderType == MIDI_MANAGER) {
    
        imgMap->getTexture().bind();
        
        if (ofRandom(0, 1) < 0.5) {
            frameNum++;
            frameNum%= NUM_FRAMES;
        }
        
        shader.begin();
        shader.setUniform1i("figure", patternNum);
        shader.setUniform1i("frame", frameNum);
        shader.setUniformTexture("tex", *imgMap, 0);
        
        mesh.draw(OF_MESH_FILL);
        
        shader.end();
        imgMap->getTexture().unbind();
        
        if (connecting) {
            ofSetColor(100, 255, 160, 150);
            mesh.draw(OF_MESH_FILL);
        }
    }
    
    
    
    ofPopMatrix();

}

float Plate::play() {

    patternFrequencies[patternNum];
//    if (pattern.audioType == SINE) {
        return osc.sinewave(patternFrequencies[patternNum].get()->get());
//    }
    return 0.0f;
}


#pragma mark -
#pragma mark PlateManager

//////////////////////////////////////////////////////////////////////////////////////////
#define SAVE_DIRECTORY "plateConfigs/"

PlateManager::PlateManager(): plateWidth(128), platePadding(10) {
    ofRegisterKeyEvents(this);
    currentPlate = NULL;
    currentEditingNoteOrder = 1;
    listeningForNote = false;
    
    drawSpecial = false;
    
    savePanel.setup("save config");
    savesGroup.setup("saves");
    
}

PlateManager::~PlateManager() {
    ofUnregisterKeyEvents(this);
}

void PlateManager::setup(int w, int h) {
    float pd = plateWidth + platePadding;
    
    plates.clear();
    for (int i = 0; i < h; i++) {
        for (int j = 0; j < w; j++) {
            float x = j * pd - (w * 0.5 * pd) + plateWidth * 0.5;
            float y = i * pd - (h * 0.5 * pd) + plateWidth * 0.5;
            plates.push_back(shared_ptr<Plate>(new Plate(x, y, plateWidth)));
        }
    }
    
    int counter = 0;
    for (auto p : plates) {
        p->id = counter++;
    }
    initPlates();
    
    if (!pickFbo.isAllocated()) {
        pickFbo.allocate(ofGetWidth(), ofGetHeight(), GL_RGB);
        pickFbo.begin();
        ofClear(0);
        pickFbo.end();
    }
    
    if (pickPixels.isAllocated()) {
        pickPixels.allocate(pickFbo.getWidth(), pickFbo.getHeight(), OF_PIXELS_RGB);
    }
    

    reloadSaves();
}

void PlateManager::initPlates() {
    colourMap.clear();
    for (auto p : plates) {
        int c2 = (p->id + 1) * 25;
        p->pickColour = cidToOfColor(c2);
        colourMap[c2]  = p->id;
    }
}

void PlateManager::update() {
    
}

void PlateManager::draw(render_t renderType) {
    
    if (renderType == NOTE_SELECT) {
        editNoteOrders();
        
    }
    else {
        for (auto &p : plates) {
            p->draw(renderType);
        }
    }
    
    currentRenderType = renderType;
}

Plate* PlateManager::getPlateAt(ofEasyCam &cam, int x, int y) {
    
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

void PlateManager::editNoteOrders() {
    
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
                ofDrawBitmapString("Editing Layer " + ofToString(currentEditingNoteOrder), p->pos - ofVec2f(p->size, p->size+20));
                doneLabel = true;
            }
        }
        
        
        ofPopMatrix();
    }
    
    
    //        ofDrawBitmapString(, 10, ofGetHeight() - 10);
}


void PlateManager::keyPressed(ofKeyEventArgs &args) {
    int key = args.key;
    if (listeningForNote && key > int('0') && key <= int('9')) {
        currentPlate->noteOrders[currentEditingNoteOrder] = key - int('0');
        currentPlate->listeningForNote = false;
        listeningForNote = false;
    }
    
    KEY(OF_KEY_UP, currentEditingNoteOrder = MIN(MAX_NOTES, currentEditingNoteOrder+1))
    KEY(OF_KEY_DOWN, currentEditingNoteOrder = MAX(1, currentEditingNoteOrder-1))
    
    if (currentRenderType == NOTE_SELECT) {
        KEY('s', saveCurrentConfig())
    }
}

void PlateManager::keyReleased(ofKeyEventArgs &args) {
    
}

void PlateManager::saveCurrentConfig() {

    string filename = ofSystemTextBoxDialog("What would you like to call this save?");
    if (filename == "") return;
    
    ofXml xml;
    
    xml.addChild("platesConfig");
    xml.setTo("platesConfig");
    xml.setAttribute("version", "0.1");
    
    xml.addChild("plates");
    xml.setTo("plates");

    for (auto &plate : plates) {
        ofXml pxml;
        string name = "plate";
        pxml.addChild(name);
        pxml.setTo(name);
        pxml.addValue("id", plate->id);
        pxml.addValue("x", plate->pos.x);
        pxml.addValue("y", plate->pos.y);
        pxml.addValue("size", plate->size);
        pxml.addValue("volume", plate->volume);
        pxml.addValue("audioChannel", plate->audioChannel);
        
        ofXml ordersXml;
        ordersXml.addChild("orders");
        ordersXml.setTo("orders");
        for (int i = 0; i < MAX_NOTES; i++) {
            ordersXml.addValue("o" + ofToString(i), plate->noteOrders[i]);
        }
        pxml.addXml(ordersXml);
        
        
        ofXml notesXml;
        notesXml.addChild("channels");
        notesXml.setTo("channels");
        for (auto q : plate->channels) {
            notesXml.addValue(q.second->getName(), q.first);
        }
        pxml.addXml(notesXml);
        
        xml.addXml(pxml);
        
    }
    
    
    ofDirectory dir(ofToDataPath(SAVE_DIRECTORY));
    
    if (!dir.exists()) {
        dir.create();
    }
    
    xml.save(SAVE_DIRECTORY + filename + ".xml");
    
    ofLogNotice() << "saved config to " << filename;
    
    reloadSaves();
}

void PlateManager::loadConfig(string filename) {

    ofXml xml;
    
    bool success = xml.load(filename);
    
    if (!success) {
        ofLogError() << "can't load confing from " << filename;
        return;
    }
    
    
    xml.setTo("platesConfig");
    float version = ofToFloat(xml.getAttribute("version"));
    
    if (version >= 0.1) {
        xml.setTo("//plates");
        int nplates = xml.getNumChildren();
        plates.clear();
        
        for (int i = 0; i < nplates; i++) {
            xml.setTo("//plates");
            xml.setToChild(i);
            float x = xml.getFloatValue("x");
            float y = xml.getFloatValue("y");
            float size = xml.getFloatValue("size");
            int id = xml.getIntValue("id");

            shared_ptr<Plate> plate = shared_ptr<Plate>(new Plate(x, y, size));
            plate->id = id;
            
            int audioChannel = xml.getIntValue("audioChannel");
            float volume = xml.getFloatValue("volume");
            plate->audioChannel = audioChannel;
            plate->volume = volume;
            
            for (int j = 0; j < MAX_NOTES; j++) {
                xml.setTo("//plates");
                xml.setToChild(i);
                xml.setTo("orders");
                xml.setToChild(j);
                plate->noteOrders[j] = xml.getIntValue();
            }

            xml.setTo("//plates");
            xml.setToChild(i);
            xml.setTo("channels");
            int nChannels = xml.getNumChildren();
            for (int j = 0; j < nChannels; j++) {
                xml.setTo("//plates");
                xml.setToChild(i);
                xml.setTo("channels");
                xml.setToChild(j);
                plate->channels[xml.getIntValue()] = shared_ptr<ofParameter<bool> >(new ofParameter<bool>(xml.getName(), true));
            }
            
            plates.push_back(plate);
        }
    }
    
    // setup the ids, so the picking works
    initPlates();
    
    ofLogNotice() << "loaded from " << filename;


    for (auto &toggle : savedFiles) {
        ofParameter<bool> *p = (ofParameter<bool>*) &toggle->getParameter();
        p->set(false);
    }

}

void PlateManager::reloadSaves() {

    savePanel.clear();
    savePanel.add(saveButton.setup("save"));
    

    ofDirectory dir(ofToDataPath(SAVE_DIRECTORY));
    
    
    dir.listDir();
    vector<ofFile> files = dir.getFiles();
//    return;
    
    for (auto &button : savedFiles) {
        button->removeListener(this, &PlateManager::savedFilePressed);
    }
    
    savedFiles.clear();
    savesGroup.clear();
    for (ofFile &file : files) {
        shared_ptr<ofxToggle> button(new ofxToggle);
        button->setup(file.getFileName(), false);
        button->addListener(this, &PlateManager::savedFilePressed);
        ofxToggle *t = button.get();
        savesGroup.add(t);
        savedFiles.push_back(button);
    }
    
    savePanel.add(&savesGroup);
}


void PlateManager::savedFilePressed(bool &b) {
    for (auto &toggle : savedFiles) {
//        bool b = toggle.get();
        ofParameter<bool> *p = (ofParameter<bool>*) &toggle->getParameter();
        if (p->get()) {
            ofLogNotice() << "loading " << toggle->getName();
            loadConfig(ofToDataPath(SAVE_DIRECTORY + toggle->getName()));
        }
    }
}

#pragma mark -

#define MIDI_CONFIG_FILENAME "patternConfig.xml"

void PlateManagerMidi::setup(int w, int h) {
    PlateManager::setup(w, h);
    int counter = 0;
    for (auto &plate : plates) {
        plate->patternNum = counter++;
//        plate->midiNotes.clear();

//        for (int i = 36; i < 72; i++) {
//            plate->midiNotes[i] = shared_ptr<ofParameter<bool> >(new ofParameter<bool>(notes[i%12] + ofToString(i/12), true));
//        }
        
    }
    
    int n = plates.size();
    int j = 0;
    for(int i = 0; i < n; i++) {
        if (i >= NUM_FIGURES) {
            plates.erase(plates.begin() + j);
            
            continue;
        }
        j++;
    }
    
}

void PlateManagerMidi::update() {
//    ChladniDB::
}

void PlateManagerMidi::keyPressed(ofKeyEventArgs &args) {
    int key = args.key;
    
    KEY('s', saveCurrentConfig())
    
    KEY('l', loadConfig())
}

void PlateManagerMidi::keyReleased(ofKeyEventArgs &args) {
    int key = args.key;
}

void PlateManagerMidi::saveCurrentConfig() {
    cout << "PlateManagerMidi::saveCurrentConfig()" << endl;
    
    ofXml xml;
    
    xml.addChild("patternConfig");
    xml.setTo("patternConfig");
    xml.setAttribute("version", "0.1");
    
    xml.addChild("patterns");
    xml.setTo("patterns");
    
    for (auto &plate : plates) {
        ofXml pxml;
        string name = "pattern";
        pxml.addChild(name);
        pxml.setTo(name);
        pxml.addValue("id", plate->id);
        pxml.addValue("volume", plate->volume);
        
        
        ofXml notesXml;
        notesXml.addChild("midiNotes");
        notesXml.setTo("midiNotes");
        for (auto q : plate->midiNotes) {
            notesXml.addValue(q.second->getName(), q.first);
        }
        pxml.addXml(notesXml);
        
        xml.addXml(pxml);
        
    }
    
    
    ofDirectory dir(ofToDataPath(SAVE_DIRECTORY));
    
    if (!dir.exists()) {
        dir.create();
    }

    string filename = MIDI_CONFIG_FILENAME;
    xml.save(filename);
    
    ofLogNotice() << "saved config to " << filename;
}


void PlateManagerMidi::loadConfig() {
    
    ofXml xml;
    
    bool success = xml.load(MIDI_CONFIG_FILENAME);
    
    if (!success) {
        ofLogError() << "can't load confing from " << MIDI_CONFIG_FILENAME;
        return;
    }
    
    
    xml.setTo("patternConfig");
    float version = ofToFloat(xml.getAttribute("version"));
    
    if (version >= 0.1) {
        xml.setTo("//patterns");
        int nplates = xml.getNumChildren();
        
        for (int i = 0; i < nplates; i++) {
            xml.setTo("//patterns");
            xml.setToChild(i);
            int id = xml.getIntValue("id");
            
            plates[i]->id = id;

            float volume = xml.getFloatValue("volume");
            plates[i]->volume = volume;
            
            
            xml.setTo("//patterns");
            xml.setToChild(i);
            xml.setTo("midiNotes");
            int nChannels = xml.getNumChildren();
            plates[i]->midiNotes.clear();
            for (int j = 0; j < nChannels; j++) {
                xml.setTo("//patterns");
                xml.setToChild(i);
                xml.setTo("midiNotes");
                xml.setToChild(j);
                plates[i]->midiNotes[xml.getIntValue()] = shared_ptr<ofParameter<bool> >(new ofParameter<bool>(xml.getName(), true));
            }
            
        }
    }

    
    ofLogNotice() << "loaded from pattern config";
    
    
}
