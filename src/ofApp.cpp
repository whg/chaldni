#include "ofApp.h"
#include "ofxTools.h"
#include "ChladniDB.h"

bool PlateManager::drawSpecial;
bool working = false;

ofColor PianoKeys::Key::baseOnCol;

#define NOCTAVES 9
#define NCHANNELS 8

#define NROWS 9



double CosineInterpolate(
                         double y1,double y2,
                         double mu)
{
    double mu2;
    
    mu2 = (1-cos(mu*PI))/2;
    return(y1*(1-mu2)+y2*mu2);
}

struct Note {
    int row, pitch;
};
map<int, Note> currentNotes;


ofApp::ofApp() {
    dmxDevice = "";
}

void ofApp::setup() {

    ofBackground(20);
    ofSetFrameRate(60);
    
    initAssets();
    ChladniDB::setup();

    
    midiIn.ignoreTypes(false, false, false);
    midiIn.addListener(this);
    midiIn.setVerbose(true);
    midiIn.listPorts();
//    midiIn.openPort("virtualMIDI");	// by name
//    midiIn.openPort("Express  128b Port 1");
    
//    panel.setup();
//    panel.add(button.setup("something"));
//    button.setup("something");
    
//    ofSoundStreamListDevices();
//    button.draw();

    PianoKeys::Key::baseOnCol = ofColor(255, 100, 100);

//    ofSetEscapeQuitsApp(false);

    int pianoHeight = 50;
    pianoKeys = PianoKeys(2, 7, 0, 0, ofGetWidth(), pianoHeight);
    channels = Channels(NCHANNELS, 0, pianoHeight);
    
    plateManager = new PlateManager();
//    plateManager->setup(5, 1);
    plateManager->setup(4, 4);
    
    
    
    pm = plateManager;
    
    cam.setTranslationKey('t');
    cam.setFarClip(20000);
    resetView();
    PlateManager::drawSpecial = false;

    panel.setup("");
    midiNotesGroup.setup("MIDI notes");
    patternFreqsGroup.setup("Pattern Frequencies");

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
//    panel.add(&toggle);
//    panel.setPosition(200, 200);
    
    currentRenderType = NORMAL;
    currentPlate = NULL;
    showPanel = false;
    needsToRedrawPanel = false;
    noSelectionMode = false;
    
    connectMode = false;
    connecting = false;
    connectedPlate = NULL;
    
//    noteToFigureManager = new PlateManagerMidi();
//    noteToFigureManager->setup(FIGURE_GRID_X, FIGURE_GRID_Y);
    
    ofSerial serial;
    auto devices = serial.getDeviceList();
    for (ofSerialDeviceInfo device : devices) {
        string name = device.getDeviceName();
        cout << name << endl;
        if (name.find("tty.usb") != string::npos) {
            dmxDevice = device.getDevicePath();
        }
    }

    if (dmxDevice.find("dev") == string::npos) {
        dmxDevice = "/dev/tty.usbserial-EN150107";
    }
    dmx.connect(dmxDevice, 512);
    
    ofSetEscapeQuitsApp(false);
}

void ofApp::exit() {

    ofSoundStreamClose();

    delete plateManager;
    
    delAssets();
}

void ofApp::resetView() {
    cam.setPosition(0, 0, 400);
//    cam.setPosition(-1.8285985, 15.7365398, 329.100342);
//    cam.setPosition(0, 0, 1376.83533);
//    cam.setPosition(start);
    cam.lookAt(ofVec3f(-0, -0, -1));
//    cam.setOrientation(ofVec3f(0, 0, 180));


    
//x = 0.464783788, y = -4.19150448, z = 113.592148)
//zero orien
//x = 0.464783788, y = -4.19150448, z = 1368.20972)



}

void ofApp::update() {

    int offset = 0;

    plateManager->update();

    if (connectMode) cam.disableMouseInput();
    else cam.enableMouseInput();

    for (auto &plate : plateManager->plates) {
        
        plate->update();
    
        dmx.setLevel(plate->id * 3 + 1 + offset, (unsigned char) (plate->dmxColour.get().r * plate->value));
        dmx.setLevel(plate->id * 3 + 3+ offset, (unsigned char) (plate->dmxColour.get().g * plate->value));
        dmx.setLevel(plate->id * 3 + 2+ offset, (unsigned char) (plate->dmxColour.get().b * plate->value));
        
    }

    dmx.update();
}


void ofApp::draw() {

    
    ofBackground(50); //100, 75, 150);
    
    ofScopedLock scopedLock(midiLock);
    
    cam.disableMouseInput();
    cam.begin();
    
    pm->draw(NORMAL);
    
    cam.end();
    
    ofSetColor(255);
    if (showPanel) {
        if (needsToRedrawPanel && currentPlate) {
            createPanel(currentPlate, false, currentRenderType == MIDI_MANAGER);
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
    
    if (currentRenderType == NOTE_SELECT) {
        pm->savePanel.draw();
    }
    
    
    
    
    ofSetColor(255);
    ofDrawBitmapString(ofToString(currentRenderType), 10, ofGetHeight()- 10);
    
//    tempPanel.draw();
//    panel.draw();

//    ofTranslate(mouseX, mouseY);
//    button.draw();
}


void ofApp::keyPressed(int key) {

    KEY('r', currentRenderType = NORMAL)
    
    KEY('f', ofToggleFullscreen())
    
    KEY('v', resetView())
    
    KEY(OF_KEY_ESC, showPanel = false)
    
    
    KEY(OF_KEY_CONTROL, connectMode = true)
    KEY('`', PlateManager::drawSpecial = true)
    KEY('z', noSelectionMode = true)

    KEY('b', blankAll())
    
    KEY('m', currentRenderType = LEARN)
    
    KEY('l', needsToRedrawPanel = true);

    if (currentRenderType == MIDI_MANAGER) {
        pm = noteToFigureManager;
        
        for (auto &key : pianoKeys.keys) {
            key->onCol = PianoKeys::Key::baseOnCol;
        }
    }
    else pm = plateManager;
    
    // this seems to make sense...
    pianoKeys.highlightKeys(false);
    
//    if (key != OF_KEY_CONTROL) showPanel = false;
}


void ofApp::keyReleased(int key) {
    KEY(OF_KEY_CONTROL, connectMode = false)
    KEY('`', PlateManager::drawSpecial = false)
    KEY('z', noSelectionMode = true)
}


void ofApp::mouseMoved(int x, int y) {

}


void ofApp::mouseDragged(int x, int y, int button) {
    Plate *plate = pm->getPlateAt(cam, x, y);
    
    
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
//    cout << "starting\n";
    
//    float t = ofGetElapsedTimef();
    Plate* plate = pm->getPlateAt(cam, x, y);
    
    if (currentRenderType == NOTE_SELECT) {
        if (plate && !pm->listeningForNote) {
            pm->listeningForNote = true;
            pm->currentPlate = plate;
            plate->listeningForNote = true;
        }
    }
    else if (currentRenderType == NORMAL || currentRenderType == MIDI_MANAGER) {
        if (plate) {
            createPanel(plate, true, currentRenderType == MIDI_MANAGER);
            
        }
        
        if (currentRenderType == MIDI_MANAGER) {
            if (pianoKeys.findKey(x, y)) {
                for (auto plate : noteToFigureManager->plates) {
                    for (auto note : plate->midiNotes) {
                        if (note.first == pianoKeys.foundKey->pitch) {
                            plate->border = true;
                        }
                    }
                }
            }
        }
    }
    
    if (connectMode) {
        connecting = false;
        if (currentRenderType == MIDI_MANAGER) {
            connecting = pianoKeys.findKey(x, y);
        }
        else if (currentRenderType == NORMAL) {
            connecting = channels.findKey(x, y);
        }
        if (connecting) {
            connectStart = ofVec2f(x, y);
            connectEnd = ofVec2f(x, y);
        }
    }
    
    working = false;
    
//    cout << "took: " << ofGetElapsedTimef() - t << endl;
}


void ofApp::mouseReleased(int x, int y, int button) {

    if (connecting) {
        cout << connectedPlate << endl;
        if (connectedPlate) {
            
            if (currentRenderType == MIDI_MANAGER) {
                PianoKeys::Key *key = pianoKeys.foundKey;
                connectedPlate->midiNotes[key->pitch] = shared_ptr<ofParameter<bool> >(new ofParameter<bool>(key->name, true));
                cout << "added to " << key->name << endl;
            }
            else if (currentRenderType == NORMAL) {
                Channels::Channel *channel = (Channels::Channel*) channels.foundKey;
                connectedPlate->channels[channel->number] = shared_ptr<ofParameter<bool> >(new ofParameter<bool>(channel->name, true));
                cout << "added channel " << channel->number << "\n";
            }
            
            connectedPlate->connecting = false;
            
            if (connectedPlate == currentPlate && showPanel) {
                createPanel(connectedPlate, false, currentRenderType == MIDI_MANAGER);
            }

        }
        connecting = false;
        connectedPlate = NULL;
    }
    
//    for (auto plate : noteToFigureManager->plates) {
//        for (auto note : plate->midiNotes) {
//            plate->border = false;
//        }
//    }

    
    
}

void ofApp::reset() {
    if (currentPlate) {
        for(auto &e : currentPlate->midiNotes) {
            e.second->removeListener(this, &ofApp::valChange);
        }
        currentPlate = NULL;
    }
}


void ofApp::createPanel(Plate *plate, bool setPos, bool forPattern) {
    
    if (currentPlate) {
        for(auto &e : currentPlate->midiNotes) {
            e.second->removeListener(this, &ofApp::valChange);
        }
    }
    
    panel.clear();

    panel.setName("Output " + ofToString(plate->id + 1));

    panel.add(plate->playing);
    panel.add(plate->dmxColour);
    
    
    if (setPos) {
        panel.setPosition(ofPoint(mouseX, mouseY));
    }

    // does this belong here?
    pianoKeys.highlightKeysForPlate(plate);

    
    currentPlate = plate;
    showPanel = true;
    
}

void ofApp::valChange(bool &b) {
    if (!b) {
//        createPanel(currentPlate, false);
        needsToRedrawPanel = true;
    }
}

void ofApp::windowResized(int w, int h) {
    float pianoHeight = pianoKeys.rect.getHeight();
    pianoKeys = PianoKeys(2, 7, 0, 0, ofGetWidth(), pianoHeight);
    channels = Channels(NCHANNELS, 0, pianoHeight);
    

    plateManager->initBuffers(ofGetWidth(), ofGetHeight());
}


void ofApp::newMidiMessage(ofxMidiMessage& msg) {

    midiLock.lock();
    
    stringstream ss;
//    cout << "status: " << msg.status << endl;
//    cout << "channel: " << msg.channel << endl;
//    cout << "pitch: " << msg.pitch << endl;
//    cout << "velocity: " << msg.velocity << endl;
//    cout << endl;

//    printf("MIDI %d: %d, %d, %d, %d\n", msg.status, msg.channel, msg.pitch, msg.control, msg.value);
//



    int index = pianoKeys.keyMap[msg.pitch];

//    if (currentRenderType == NORMAL || currentRenderType == MIDI_MANAGER) {
        if (msg.status == MIDI_NOTE_ON) {
            pianoKeys.keys[index]->onCol = channels.keys[msg.channel-1]->onCol;
            pianoKeys.keys[index]->on = true;
            pianoKeys.keys[index]->channel = msg.channel;
//            cout << "on " << index << endl;
        }
        else if (msg.status == MIDI_NOTE_OFF) {
            
            if (msg.channel == pianoKeys.keys[index]->channel) {
                pianoKeys.keys[index]->onCol = PianoKeys::Key::baseOnCol;
                pianoKeys.keys[index]->on = false;
//                cout << "off " << index << endl;
            }
        }
    
    
//    }

    
    if (currentRenderType == NORMAL) {
        
        if (msg.status == MIDI_NOTE_ON) {
            
            for (auto &plate : plateManager->plates) {
                if (plate->channels.count(msg.channel)) {
                    plate->play();
                    playingPlates[msg.channel][msg.pitch].push_back(plate.get());

                }
            }

        }
        else if (msg.status == MIDI_NOTE_OFF) {
            
            if (playingPlates[msg.channel].count(msg.pitch)) {
                for (Plate *p : playingPlates[msg.channel][msg.pitch]) {
                    p->stop();
                }
                
                playingPlates[msg.channel].erase(msg.pitch);
            }
    
            
         }
    }
    
    if (msg.status == MIDI_CONTROL_CHANGE) {
        if (currentRenderType == LEARN) {
            Plate *p = plateManager->plates[plateManager->lastChangedNumber].get();
            controlsToParameter[msg.control] = pair<Plate*, char>(p, plateManager->lastChangedChannel);
            
            currentRenderType = NORMAL;
//            createPanel(p, false);
            needsToRedrawPanel = true;
            cout << "SET to " << msg.control << endl;
        }
        else {
            int v = msg.value * 2;
            if (controlsToParameter.count(msg.control)) {
                ofParameter<ofColor> &p = controlsToParameter[msg.control].first->dmxColour;
                switch (controlsToParameter[msg.control].second) {
                    case 'r':
                        p.set(ofColor(v, p->g, p->b, p->a));
                        break;
                    case 'g':
                        p.set(ofColor(p->r, v, p->b, p->a));
                        break;
                    case 'b':
                        p.set(ofColor(p->r, p->g, v, p->a));
                        break;
                    case 'a':
                        p.set(ofColor(p->r, p->g, p->b, v));
                        break;

                }
            }
        }
//        cout << msg.control << endl;
//        cout << msg.value << endl;
    }
    
    midiLock.unlock();
}

void ofApp::audioOut(float *output, int bufferSize, int nChannels) {
    
}


void ofApp::blankAll() {

    for (auto &p : plateManager->plates) {
        p->setPatternNum(NUM_FIGURES);
    }
            //
}
