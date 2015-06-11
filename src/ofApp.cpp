#include "ofApp.h"
#include "ofxTools.h"
#include "ChladniDB.h"

bool PlateManager::drawSpecial;
bool working = false;

ofColor PianoKeys::Key::baseOnCol;

void ofApp::setup() {

    ofBackground(20);
    
    initAssets();
    ChladniDB::setup();

    
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

    PianoKeys::Key::baseOnCol = ofColor(255, 100, 100);

//    ofSetEscapeQuitsApp(false);

    int pianoHeight = 50;
    pianoKeys = PianoKeys(2, 7, 0, 0, ofGetWidth(), pianoHeight);
    channels = Channels(5, 0, pianoHeight);
    
    plateManager = new PlateManager();
//    plateManager->setup(5, 5);
    plateManager->setup(5, 1);
    
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
    
    connectMode = false;
    connecting = false;
    connectedPlate = NULL;
    
    noteToFigureManager = new PlateManagerMidi();
    noteToFigureManager->setup(FIGURE_GRID_X, FIGURE_GRID_Y);
    
    
    currentNotesPlaying = 0;
    for (int i = 1; i <= MAX_CHANNELS; i++) {
        playingPitches[i] = map<int,int>();

    }
    
    ofSoundStreamSetup(2, 0, 44100, 512, 2);
}

void ofApp::exit() {

    ofSoundStreamClose();

    delete noteToFigureManager;
    delete plateManager;
    
    delAssets();
}

void ofApp::resetView() {
    cam.setPosition(0, 0, 500);
    cam.lookAt(ofVec3f(-0, -0, -1));
}

void ofApp::update() {
    ofSetWindowTitle(ofToString(ofGetFrameRate()));
    
    if (connectMode) cam.disableMouseInput();
    else cam.enableMouseInput();
}


void ofApp::draw() {

    if (currentRenderType == MIDI_MANAGER) {
        ofBackground(100, 75, 150);

    }
    else ofBackground(0);

    if (currentRenderType == NORMAL) {
        channels.draw();
    }
    if (currentRenderType == NORMAL || currentRenderType == MIDI_MANAGER) {
        pianoKeys.draw();
    }
    
    
    cam.begin();
    
//    cout << cam.getLookAtDir() << endl;
    pm->draw(currentRenderType);
    
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
//    panel.draw();

//    ofTranslate(mouseX, mouseY);
//    button.draw();
}


void ofApp::keyPressed(int key) {

    KEY('r', currentRenderType = NORMAL)
    KEY('e', currentRenderType = NOTE_SELECT)
    KEY('m', currentRenderType = MIDI_MANAGER)
    

    
    KEY('v', resetView())
    
    KEY('q', showPanel = false)
    
    KEY(OF_KEY_CONTROL, connectMode = true)
    KEY('`', PlateManager::drawSpecial = true)



    if (currentRenderType == MIDI_MANAGER) {
        pm = noteToFigureManager;
        
        for (auto &key : pianoKeys.keys) {
            key->onCol = PianoKeys::Key::baseOnCol;
        }
    }
    else pm = plateManager;
    
    // this seems to make sense...
    pianoKeys.highlightKeys(false);
    
    if (key != OF_KEY_CONTROL) showPanel = false;
}


void ofApp::keyReleased(int key) {
    KEY(OF_KEY_CONTROL, connectMode = false)
    KEY('`', PlateManager::drawSpecial = false)
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
    cout << "starting\n";
    
    float t = ofGetElapsedTimef();
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
    
    cout << "took: " << ofGetElapsedTimef() - t << endl;
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
    
    
}


void ofApp::createPanel(Plate *plate, bool setPos, bool forPattern) {
    
    if (currentPlate) {
        for(auto &e : currentPlate->midiNotes) {
            e.second->removeListener(this, &ofApp::valChange);
        }
    }
    
    panel.clear();

    
    if (!forPattern) {
        panel.setName("Plate " + ofToString(plate->id));
        panel.add(plate->audioChannel);
        panel.add(plate->randomFigure);
    }
    else {
        panel.setName("Pattern " + ofToString(plate->patternNum));
    }
    panel.add(plate->volume);
    
    map<int, shared_ptr< ofParameter<bool> > > *data;
    
    if (currentRenderType == NORMAL) {
        data = &plate->channels;
        midiNotesGroup.setName("Channels");
    }
    else if (currentRenderType == MIDI_MANAGER) {
        data = &plate->midiNotes;
        midiNotesGroup.setName("MIDI notes");
    }

    midiNotes.clear();
    vector<int> toErase;
    for (auto &e : *data) {
        int pitch = e.first;
        ofParameter<bool> *param = e.second.get();
        if (param->get()) {
            midiNotes.push_back(shared_ptr<ofxToggle>(new ofxToggle(*param)));
            param->addListener(this, &ofApp::valChange);
            cout << "name = " << e.second->getName() << endl;
        }
        else {
            toErase.push_back(pitch);
        }
        
    }
    
    for (int pitch : toErase) {
        cout << "erasing " << pitch << endl;
        data->erase(pitch);
    }
    
    midiNotesGroup.clear();

    for (auto &e : midiNotes) {
        midiNotesGroup.add(e.get()->setup(e->getName(), true));
    }
    
    panel.add(&midiNotesGroup);
    
    
    if (currentRenderType == NORMAL) {
        patternFreqsGroup.clear();
        for (int i = 0; i < NUM_FIGURES; i++) {
            patternFreqsGroup.add(plate->patternFrequencies[i]);
        }
        panel.add(&patternFreqsGroup);
    }
    
    
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

}


void ofApp::newMidiMessage(ofxMidiMessage& msg) {
    stringstream ss;
//    cout << "status: " << msg.status << endl;
//    cout << "channel: " << msg.channel << endl;
//    cout << "pitch: " << msg.pitch << endl;
//    cout << "velocity: " << msg.velocity << endl;
//    cout << endl;
//    


    int index = pianoKeys.keyMap[msg.pitch];

    if (currentRenderType == NORMAL || currentRenderType == MIDI_MANAGER) {
        if (msg.status == MIDI_NOTE_ON) {
            pianoKeys.keys[index]->onCol = channels.keys[msg.channel-1]->onCol;
            pianoKeys.keys[index]->on = true;
        }
        else if (msg.status == MIDI_NOTE_OFF) {
            pianoKeys.keys[index]->onCol = PianoKeys::Key::baseOnCol;
            pianoKeys.keys[index]->on = false;
        }
    }
    
    if (currentRenderType == NORMAL) {
        
        if (msg.status == MIDI_NOTE_ON) {

            currentNotesPlaying++;
            playingPitches[msg.channel][currentNotesPlaying] = msg.pitch;
                        
            for (auto &plate : plateManager->plates) {
                if (plate->channels.count(msg.channel)) {
                
                    int pitchToFind = playingPitches[msg.channel][plate->noteOrders[currentNotesPlaying]];
                    
                    for (auto &figurePlate : noteToFigureManager->plates) {
                    
                        if (figurePlate->midiNotes.count(msg.pitch)) {
                            cout << "got hit at pitch " << msg.pitch << endl;
                            plate->patternNum = figurePlate->id;
                            plate->playing = true;
                            playingPlates[msg.channel][msg.pitch] = plate.get();
                        }
                    }
                }
            }

        }
        else if (msg.status == MIDI_NOTE_OFF) {
            
            playingPitches[msg.channel].erase(msg.pitch);
            
            if (playingPlates[msg.channel].count(msg.pitch)) {
            
                playingPlates[msg.channel][msg.pitch]->playing = false;
                playingPlates[msg.channel].erase(msg.pitch);
            }
            
            currentNotesPlaying--;
        }
    }
    
}

void ofApp::audioOut(float *output, int bufferSize, int nChannels) {
    vector<shared_ptr<Plate> > plates = pm->plates;
    static int baseIndex = 0;
    
    for (int i = 0; i < bufferSize; i++) {
        baseIndex = i * nChannels;
        
        for (auto plate : plates) {
            if (plate->audioChannel > nChannels) continue;
            
            output[baseIndex + plate->audioChannel] = plate->play();
        }
    }
}

