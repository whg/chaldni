#include "ofApp.h"
#include "ofxTools.h"
#include "ChladniDB.h"

bool PlateManager::drawSpecial;
bool working = false;

ofColor PianoKeys::Key::baseOnCol;

void ofApp::setup() {

    ofBackground(20);
    ofSetFrameRate(30);
    
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
    pianoKeys = PianoKeys(0, 7, 0, 0, ofGetWidth(), pianoHeight);
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
    noSelectionMode = false;
    
    connectMode = false;
    connecting = false;
    connectedPlate = NULL;
    
    noteToFigureManager = new PlateManagerMidi();
    noteToFigureManager->setup(FIGURE_GRID_X, FIGURE_GRID_Y);
    
    
    
    for (int i = 1; i <= MAX_CHANNELS; i++) {
        playingPitches[i] = map<int,int>();
        currentNotesPlaying[i] = 0;
    }
    
//    soundStream.
//    ofSoundStreamSetup(5, 0, 44100, 512, 2);

    soundStream.printDeviceList();
    soundStream.setDeviceID(1);
    
    soundStream.setup(this, 2, 0, 44100, 512, 2);
    
    
//    plateManager->setup(10, 10);
//    initSnake();
//    mode = SNAKE;

//    tempPanel.setup("tempPanel");
//    tempPanel.add(spacing.set("spacing", 10, 0, 100));
//    
//    loadImage();
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
//    cam.setOrientation(ofVec3f(0, 0, 180));
}

void ofApp::update() {

//    ofScopedLock lock(audioMutex);

//    ofSetWindowTitle(ofToString(ofGetFrameRate()));
    
//    plateManager->platePadding = spacing;
//    plateManager->setPositions();
    
    if (connectMode) cam.disableMouseInput();
    else cam.enableMouseInput();
 
    int k = 0;
    const int SKIP = 40;
    int n = ofGetFrameNum() / SKIP;
    static float tt = 0;
    
//    if (mode == HWAVE) {
//        if (ofGetFrameNum() % SKIP) return;
//        for (int i = 0; i < plateManager->width; i++) {
//            for (int j = 0; j < plateManager->height; j++) {
//                plateManager->plates[k++]->setPatternNum((i + n) % NUM_FIGURES);
//            }
//        }
//    }
//    else if (mode == VWAVE) {
//        if (ofGetFrameNum() % SKIP) return;
//        for (int i = 0; i < plateManager->width; i++) {
//            for (int j = 0; j < plateManager->height; j++) {
//                plateManager->plates[k++]->setPatternNum((j + n) % NUM_FIGURES);
//            }
//        }
//    }
//    else if (mode == CWAVE) {
//        float t = ofGetElapsedTimef();
//        if (ofGetFrameNum() % SKIP) return;
//        tt+= 0.1;
//        t = tt;
//        for (int i = 0; i < plateManager->width; i++) {
//            for (int j = 0; j < plateManager->height; j++) {
//                plateManager->plates[k++]->setPatternNum(int((sin(t + i * 0.1) + cos(t + j * 0.1)  + 2) * 0.25 * NUM_FIGURES));// % NUM_FIGURES;
//            }
//        }
//    }
//    else if (mode == RANDOM) {
//        if (n % SKIP == 0) {
//            for (auto &plate : plateManager->plates) {
//                plate->setPatternNum(int(ofRandom(NUM_FIGURES)));
//            }
//        }
//    }
//    else if (mode == SNAKE) {
//        if (ofGetFrameNum() % 5 == 0) {
//            snake();
//        }
//    }
//    else if (mode == HSWEEP) {
//        if (ofGetFrameNum() % SKIP) return;
//        for (int i = 0; i < plateManager->width; i++) {
//            for (int j = 0; j < plateManager->height; j++) {
//                plateManager->plates[k++]->setPatternNum((i + n) % NUM_FIGURES);
//            }
//        }
//    }
}


void ofApp::draw() {

    if (currentRenderType == MIDI_MANAGER) {
        ofBackground(155); //100, 75, 150);

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
    
    tempPanel.draw();
//    panel.draw();

//    ofTranslate(mouseX, mouseY);
//    button.draw();
}


void ofApp::keyPressed(int key) {

    KEY('r', currentRenderType = NORMAL)
    KEY('e', currentRenderType = NOTE_SELECT)
    KEY('m', currentRenderType = MIDI_MANAGER)
    
    KEY('f', ofToggleFullscreen())
    
    KEY('v', resetView())
    
    KEY('q', showPanel = false)
    
    KEY(OF_KEY_CONTROL, connectMode = true)
    KEY('`', PlateManager::drawSpecial = true)
    KEY('z', noSelectionMode = true)

    KEY(']', awesomeMode())
    KEY('=', setSize())
    KEY(';', muteAll())

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
    
    for (auto plate : noteToFigureManager->plates) {
        for (auto note : plate->midiNotes) {
            plate->border = false;
        }
    }

    
    
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

    
    if (!forPattern) {
        panel.setName("Plate " + ofToString(plate->id));
        panel.add(plate->audioChannel);
        panel.add(plate->randomFigure);
    }
    else {
        panel.setName("Pattern " + ofToString(plate->getPatternNum()));
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
    float pianoHeight = pianoKeys.rect.getHeight();
    pianoKeys = PianoKeys(0, 7, 0, 0, ofGetWidth(), pianoHeight);
    channels = Channels(5, 0, pianoHeight);
    
    noteToFigureManager->initBuffers(ofGetWidth(), ofGetHeight());
    plateManager->initBuffers(ofGetWidth(), ofGetHeight());
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

//    if (currentRenderType == NORMAL || currentRenderType == MIDI_MANAGER) {
        if (msg.status == MIDI_NOTE_ON) {
            pianoKeys.keys[index]->onCol = channels.keys[msg.channel-1]->onCol;
            pianoKeys.keys[index]->on = true;
            pianoKeys.keys[index]->channel = msg.channel;
            cout << "on " << index << endl;
        }
        else if (msg.status == MIDI_NOTE_OFF) {
            
            if (msg.channel == pianoKeys.keys[index]->channel) {
                pianoKeys.keys[index]->onCol = PianoKeys::Key::baseOnCol;
                pianoKeys.keys[index]->on = false;
                cout << "off " << index << endl;
            }
        }
//    }
    
    if (currentRenderType == NORMAL) {
        
        if (msg.status == MIDI_NOTE_ON) {

            currentNotesPlaying[msg.channel]++;
            playingPitches[msg.channel][currentNotesPlaying[msg.channel]] = msg.pitch;
                        
            for (auto &plate : plateManager->plates) {
                if (plate->channels.count(msg.channel)) {
                
                    int pitchToFind = playingPitches[msg.channel][plate->noteOrders[currentNotesPlaying[msg.channel]]];
                    
//                    cout << "currentNotesPlaying = " << currentNotesPlaying[msg.channel] << endl;
//                    cout << "pitch = " << msg.pitch << ", noteOrder = " << plate->noteOrders[currentNotesPlaying[msg.channel]] << ", pitchTofind " << pitchToFind << endl;
                    
                    for (auto &figurePlate : noteToFigureManager->plates) {
                    
                        if (figurePlate->midiNotes.count(pitchToFind)) {
//                            cout << "got hit at pitch " << msg.pitch << endl;
                            plate->setPatternNum(figurePlate->id);
                            plate->playing = true;
                            playingPlates[msg.channel][pitchToFind][plate->id] = plate.get();
                        }
                    }
                }
            }

        }
        else if (msg.status == MIDI_NOTE_OFF) {
            
            playingPitches[msg.channel].erase(msg.pitch);
            
            for (auto &plate : playingPlates[msg.channel]) {
                if (plate.first == msg.pitch) {
                    for (auto &playingPlates : plate.second) {
                        playingPlates.second->playing = false;
                    }
                }
            }
            
            if (playingPlates[msg.channel].count(msg.pitch)) {
            
//                playingPlates[msg.channel][msg.pitch]->playing = false;
                playingPlates[msg.channel].erase(msg.pitch);
            }
            
            currentNotesPlaying[msg.channel]--;
        }
    }
    
}

void ofApp::audioOut(float *output, int bufferSize, int nChannels) {
    
    ofScopedLock lock(audioMutex);

    vector<shared_ptr<Plate> > &plates = plateManager->plates;
    static int baseIndex = 0;
    
    if (plates.size() > 20) return;
    
    for (int i = 0; i < bufferSize; i++) {
        baseIndex = i * nChannels;
        
        for (auto &plate : plates) {
            if (plate->audioChannel > nChannels) continue;
            
            output[baseIndex + plate->audioChannel] = plate->play() * noteToFigureManager->plates[plate->getPatternNum()]->volume;
        }
    }
}

void ofApp::muteAll() {
    for (auto &plate : plateManager->plates) {
        plate->playing = false;
    }
    playingPlates.clear();
    playingPitches.clear();
}



void ofApp::setSize() {
    string s = ofSystemTextBoxDialog("How big shall we go?");
    
    if (currentPlate) {
        for(auto &e : currentPlate->midiNotes) {
            e.second->removeListener(this, &ofApp::valChange);
        }
        currentPlate = NULL;
    }
    
    int start = 0;
    for (int i = 0; i < s.length(); i++) {
        if (s[i] == 'x') {
            int w = ofToInt(s.substr(0, i));
            int h = ofToInt(s.substr(i+1, s.length()));

            if (w == 0 || h == 0) {
                ofLogError() << "bad sizes!!!";
            }
            else {
                plateManager->setup(w, h);
            }
        }
    }
    
}

void ofApp::awesomeMode() {
    string s = ofSystemTextBoxDialog("What would you like to do?");
    
    if (s == "") {
        mode = NORMAL_MODE;
        muteAll();
    }
    else if (s == "hwave") {
        mode = HWAVE;
    }
    else if (s == "vwave") {
        mode = VWAVE;
    }
    else if (s == "cwave") {
        mode = CWAVE;
    }
    else if (s == "random") {
        mode = RANDOM;
    }
    else if (s == "+") {
        if (plateManager->plates.size() == 5) {
            plateManager->plates[0]->pos.x = plateManager->plates[2]->pos.x;
            plateManager->plates[4]->pos.x = plateManager->plates[2]->pos.x;
            
            plateManager->plates[0]->pos.y = plateManager->plates[2]->pos.y - plateManager->getPlateGap();
            plateManager->plates[4]->pos.y = plateManager->plates[2]->pos.y + plateManager->getPlateGap();
        }
    }
    else if (s == "-") {
        if (plateManager->plates.size() == 5) {
            plateManager->plates[0]->pos.x = plateManager->plates[1]->pos.x - plateManager->getPlateGap();
            plateManager->plates[4]->pos.x = plateManager->plates[3]->pos.x + plateManager->getPlateGap();
            
            plateManager->plates[0]->pos.y = plateManager->plates[2]->pos.y;
            plateManager->plates[4]->pos.y = plateManager->plates[2]->pos.y;
        }
    }
    else if (s == "snake") {
        initSnake();
        mode = SNAKE;
    }
    
}

void ofApp::initSnake() {
    snakeData.clear();
    
    for (int i = 0; i < 5; i++) {
        snakeData.push_back(ofVec2f(7-i, 2));
    }
    
//    for (auto &plate : plateManager->plates) {
//        snakeData.push_back(false);
//    }
}

void ofApp::snake() {
    
    int w = plateManager->width;
    int h = plateManager->height;
    
    auto &plates = plateManager->plates;
    
    for (auto &plate : plateManager->plates) {
        plate->setPatternNum(1); //off
    }
    
    size_t s = snakeData.size();
    ofVec2f dir = snakeData[0] - snakeData[1];
    
    float r = ofRandom(1);
    float t = 0.18;
    if (r < t) {
        dir.rotate(90);
    }
    else if (r < t*2) {
        dir.rotate(-90);
    }
    
//    ofVec2f np = snakeData[0] + dir;
    snakeData.push_front(snakeData[0] + dir);
    
    snakeData.pop_back();
    
    for (ofVec2f p : snakeData) {
        while (p.x < 0) p.x+= w;
        while (p.y < 0) p.y+= h;
        
        int k = int(p.x) % w + w * (int(p.y) % h);
        plateManager->plates[k]->setPatternNum(9);
        plateManager->plates[k]->setPatternNum(9);
    }
    
    
}

void ofApp::loadImage() {
    ofImage img;
    img.load("/Users/whg/Desktop/original1.jpg");
    
//    plateManager->plates.clear();
    int w = plateManager->plateWidth;
    int g = plateManager->getPlateGap();
    ofPixelsRef pr = img.getPixels();
    plateManager->setup(img.width, img.height);
    
    int k = 0;
    for (int i = 0; i < img.height; i++) {
        for (int j = 0; j < img.width; j++) {
            shared_ptr<Plate> &p = plateManager->plates[k++]; //shared_ptr<Plate>(new Plate(i*g, j*g, w));
//            p->pos.x = 0;//i*g;
//            p->pos.y = 1;//j*g;
//            cout << pr.getColor(i, j).getBrightness() << endl;
            if (pr.getColor(j, i).getBrightness() > 128) {
                p->setPatternNum(12);
                p->setPatternNum(12);
//                cout << "a" << endl;
            }
            else {
                p->setPatternNum(0);
                p->setPatternNum(0);
//                cout << "b" << endl;
            }
//            plateManager->plates.push_back(p);
        }
    }
}

void ofApp::blankAll() {

    for (auto &p : plateManager->plates) {
        
    }
            //
}
