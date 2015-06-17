//
//  PlateManager.cpp
//  lexus_plates
//
//  Created by Will Gallia on 15/06/2015.
//
//

#include "PlateManager.h"


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
    width = w;
    height = h;
    
    float pd = getPlateGap();
    
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
    
    initBuffers();
    
    reloadSaves();
}

void PlateManager::setPositions() {
    
    float pd = getPlateGap();
    
    int k = 0;
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            plates[k]->pos.x = j * pd - (width * 0.5 * pd) + plateWidth * 0.5;
            plates[k]->pos.y = i * pd - (height * 0.5 * pd) + plateWidth * 0.5;
            k++;
        }
    }
}

void PlateManager::initPlates() {
    colourMap.clear();
    for (auto p : plates) {
        int c2 = (p->id + 1) * 25;
        p->pickColour = cidToOfColor(c2);
        colourMap[c2]  = p->id;
    }
}

void PlateManager::initBuffers(int w, int h) {
    pickFbo.allocate(w, h, GL_RGB);
    pickFbo.begin();
    ofClear(0);
    pickFbo.end();
    
    pickPixels.allocate(w, h, OF_PIXELS_RGB);
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
//        cout << "found id: " << colourMap[cid] << endl;
        return plates[colourMap[cid]].get();
    }
    
//    cout << "not found" << endl;
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
        KEY('s', saveCurrentConfig(ofSystemTextBoxDialog("What would you like to call this save?")))
    }
    else {
        KEY('s', saveCurrentConfig())
        KEY('l', loadConfig())
    }
}

void PlateManager::keyReleased(ofKeyEventArgs &args) {
    
}

void PlateManager::saveCurrentConfig(string filename) {
    

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
            notesXml.addValue("channel", q.first);
        }
        pxml.addXml(notesXml);
        
        ofXml freqAdjust;
        freqAdjust.addChild("freqs");
        freqAdjust.setTo("freqs");
        for (auto f : plate->patternFrequencies) {
            freqAdjust.addValue("freq", f);
        }
        pxml.addXml(freqAdjust);

        
        xml.addXml(pxml);
        
    }
    
    
    ofDirectory dir(ofToDataPath(SAVE_DIRECTORY));
    
    if (!dir.exists()) {
        dir.create();
    }
    
    xml.save(SAVE_DIRECTORY + filename);
    
    ofLogNotice() << "saved config to " << filename;
    
    reloadSaves();
}

void PlateManager::loadConfig(string filename) {
    
    ofXml xml;
    
    bool success = xml.load(SAVE_DIRECTORY + filename);
    
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
                int channelNo = xml.getIntValue();
                plate->channels[channelNo] = shared_ptr<ofParameter<bool> >(new ofParameter<bool>("Channel " + ofToString(channelNo), true));
            }
            
//            
            xml.setTo("//plates");
            xml.setToChild(i);
            xml.setTo("freqs");
            int nFreqs = xml.getNumChildren();
//            plate->patternFrequencies.clear();
            for (int j = 0; j < nFreqs; j++) {
                if (j >= NUM_FIGURES) break;
                xml.setTo("//plates");
                xml.setToChild(i);
                xml.setTo("freqs");
                xml.setToChild(j);
                float freq = xml.getFloatValue();
                plate->patternFrequencies[j].set(ofToString(j), freq, freq-FREQ_ADJUST_AMOUNT, freq+FREQ_ADJUST_AMOUNT);
//                plate->patternFrequencies.push_back(ofParameter<float>(ofToString(j), freq, freq-FREQ_ADJUST_AMOUNT, freq+FREQ_ADJUST_AMOUNT));
                
//                plate->channels[channelNo] = shared_ptr<ofParameter<bool> >(new ofParameter<bool>("Channel " + ofToString(channelNo), true));
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


void PlateManagerMidi::setup(int w, int h) {
    PlateManager::setup(w, h);
    int counter = 0;
    for (auto &plate : plates) {
        plate->setPatternNum(counter++);
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

void PlateManagerMidi::saveCurrentConfig(string filename) {
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
    
    xml.save(filename);
    
    ofLogNotice() << "saved config to " << filename;
}


void PlateManagerMidi::loadConfig(string filename) {
    
    ofXml xml;
    
    bool success = xml.load(filename);
    
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
        
            if (i >= NUM_FIGURES) break;
        
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
    
    
    ofLogNotice() << "loaded from " << filename;
    
    
}
