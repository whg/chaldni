#include "ofApp.h"
#include "ofxTools.h"

bool PlateManager::drawSpecial;
bool working = false;

void ofApp::setup() {

    ofBackground(20);
    
    midiIn.ignoreTypes(false, false, false);
    midiIn.addListener(this);
    midiIn.setVerbose(true);
//    midiIn.listPorts();
    midiIn.openPort("virtualMIDI Bus 1");	// by name
    
//    panel.setup();
//    panel.add(button.setup("something"));
//    button.setup("something");
    
//    ofSoundStreamListDevices();
//    button.draw();


//    ofSetEscapeQuitsApp(false);

    pianoKeys = PianoKeys(2, 7);
    
    pm.setup(5, 5);
    
    cam.setTranslationKey('t');
    cam.setFarClip(20000);
    resetView();
    PlateManager::drawSpecial = false;

    panel.setup("");
    midiNotesGroup.setup("MIDI notes");

//    panel.add(xx.set("something", 2, 0, 3));
//
//
//    ofParameterGroup g;
//    g.setName("asdf 2");
//    g.add(xx.set("something 2", 1, 0, 3));
//    panel.add(g);
//    panel.add(label.setup(ofParameter<string>("", "hi there")));
//    toggle.setup(ofParameter<bool>("tog", true));
//    toggle.addListener(this, &ofApp::valChange);
//
    panel.add(&toggle);
    panel.setPosition(200, 200);
    
    currentRenderType = NORMAL;
    currentPlate = NULL;
    showPanel = false;
    needsToRedrawPanel = false;
    
    connectMode = false;
    connecting = false;
    connectedPlate = NULL;
}

void ofApp::resetView() {
    cam.setPosition(0, 0, 1400);
    cam.lookAt(ofVec3f(-0, -0, -1));
}

void ofApp::update() {
    ofSetWindowTitle(ofToString(ofGetFrameRate()));
    
    if (connectMode) cam.disableMouseInput();
    else cam.enableMouseInput();
}


void ofApp::draw() {

    pianoKeys.draw();

    cam.begin();
    
//    cout << cam.getLookAtDir() << endl;
    pm.draw(currentRenderType);
    
    cam.end();
    
    ofSetColor(255);
    if (showPanel) {
        if (needsToRedrawPanel && currentPlate) {
            createPanel(currentPlate, false);
            cout << "redraw" << endl;
            needsToRedrawPanel = false;
        }
        panel.draw();
    }
    
    if (connecting) {
        ofPushStyle();
        ofSetColor(100, 255, 160);
        ofSetLineWidth(2);
        ofDrawLine(connectStart, connectEnd);
        ofPopStyle();
    }
    
//    panel.draw();

//    ofTranslate(mouseX, mouseY);
//    button.draw();
}


void ofApp::keyPressed(int key) {

    KEY('r', currentRenderType = NORMAL)
    KEY('e', currentRenderType = NOTE_SELECT)
    
    KEY('v', resetView())
    
    KEY('q', showPanel = false)
    
    KEY(OF_KEY_CONTROL, connectMode = true)
    KEY('s', PlateManager::drawSpecial = true)
}


void ofApp::keyReleased(int key) {
    KEY(OF_KEY_CONTROL, connectMode = false)
    KEY('s', PlateManager::drawSpecial = false)
}


void ofApp::mouseMoved(int x, int y) {

}


void ofApp::mouseDragged(int x, int y, int button) {
    Plate *plate = pm.getPlateAt(cam, x, y);
    
    
    if (plate && connecting) {
        if (connectedPlate) {
            connectedPlate->connecting = false;
        }
        plate->connecting = true;
        connectedPlate = plate;
    }
    else {
        if (connectedPlate) {
            connectedPlate->connecting = false;
            connectedPlate = NULL;
        }
    }
    
    connectEnd = ofVec2f(x, y);
}


void ofApp::mousePressed(int x, int y, int button) {

    if (working) return;
    working = true;
    cout << "starting\n";
    
    float t = ofGetElapsedTimef();
    Plate* plate = pm.getPlateAt(cam, x, y);
    
    if (currentRenderType == NOTE_SELECT) {
        if (plate && !pm.listeningForNote) {
            pm.listeningForNote = true;
            pm.currentPlate = plate;
            plate->listeningForNote = true;
        }
    }
    else if (currentRenderType == NORMAL) {
        if (plate) {
            createPanel(plate);
        }
        
    }
    
    if (connectMode) {
        connecting = pianoKeys.findKey(x, y);
        if (connecting) {
            connectStart = ofVec2f(x, y);
            connectEnd = ofVec2f(x, y);
        }
    }
    
    working = false;
    
    cout << "took: " << ofGetElapsedTimef() - t << endl;
}


void ofApp::mouseReleased(int x, int y, int button) {

    if (connecting) {
        cout << connectedPlate << endl;
        if (connectedPlate) {
            PianoKeys::Key *key = pianoKeys.foundKey;
            connectedPlate->midiNotes[key->pitch] = shared_ptr<ofParameter<bool> >(new ofParameter<bool>(key->name, true));
            cout << "added to " << key->name << endl;
            connectedPlate->connecting = false;
            
            if (connectedPlate == currentPlate && showPanel) {
                createPanel(connectedPlate, false);
            }

        }
        connecting = false;
        connectedPlate = NULL;
    }
    
    
}


void ofApp::createPanel(Plate *plate, bool setPos) {
    
    if (currentPlate) {
        for(auto &e : currentPlate->midiNotes) {
            e.second->removeListener(this, &ofApp::valChange);
        }
    }
    
    panel.setName("Plate " + ofToString(plate->id));
    panel.clear();
    panel.add(plate->audioChannel);
    panel.add(plate->volume);
    

    midiNotes.clear();
    for (auto &e : plate->midiNotes) {
        int pitch = e.first;
        ofParameter<bool> *param = e.second.get();
        if (param->get()) {
            midiNotes.push_back(shared_ptr<ofxToggle>(new ofxToggle(*param)));
            param->addListener(this, &ofApp::valChange);
            cout << "name = " << e.second->getName() << endl;
        }
        
    }
    
    midiNotesGroup.clear();

    for (auto &e : midiNotes) {
        midiNotesGroup.add(e.get()->setup(e->getName(), true));
    }
    
    panel.add(&midiNotesGroup);
    
    if (setPos) {
        panel.setPosition(ofPoint(mouseX, mouseY));
    }
    
    currentPlate = plate;
    showPanel = true;
}

void ofApp::valChange(bool &b) {
    if (!b) {
//        createPanel(currentPlate, false);
        needsToRedrawPanel = true;
    }
    cout << "val change " << b << endl;
}

void ofApp::windowResized(int w, int h) {

}


void ofApp::gotMessage(ofMessage msg) {

}


void ofApp::dragEvent(ofDragInfo dragInfo) { 

}

void ofApp::newMidiMessage(ofxMidiMessage& msg) {
    stringstream ss;
    cout << "status: " << msg.status << endl;
    cout << "channel: " << msg.channel << endl;
    cout << "pitch: " << msg.pitch << endl;
    cout << "velocity: " << msg.velocity << endl;
    cout << endl;
    
    pitches.insert(msg.pitch);
    
    for (auto p : pitches) {
        cout << p << ", ";
    }
    cout << endl;
//    cout << ss;
}
