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
        font->load("/Library/Fonts/Arial.ttf", 48, false, true, true);
    }
    
    if (!imgMap) {
        imgMap = new ofImage();
        imgMap->load("m256-500.png");
    }
}

void delAssets() {
    delete font;
    delete imgMap;
}

Plate::Plate(float x, float y, float w): pos(x, y), size(w), listeningForNote(false), tint(false) {
    
    mesh = ofMesh::plane(w, w);
    
    patternNum = 0;
    setPatternNum(0);
    frameNum = 0;

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
                          
                          uniform int lastFigure;
                          uniform int figure;
                          uniform int frame;
                          uniform sampler2DRect tex;
                          uniform vec2 size = vec2(256.0);
                          
                          vec2 dim = vec2(26.0, 11.0);
                          
                          uniform float blend = 1.0;
                          
                          void main() {
                              
                              vec2 t = gl_TexCoord[0].xy / dim;
                          
                              vec2 p = vec2(t.x + float(frame) * size.x,
                                            t.y + float(figure) * size.y);
                              vec4 c = texture2DRect(tex, p);
                              
                              p.y = t.y + float(lastFigure) * size.y;
                              vec4 oc = texture2DRect(tex, p);
                              
                              gl_FragColor = c * (blend) + oc * (1.0 - blend); //vec4(texc.x, texc.y, 1.0, 1.0); //c; //vec4(1.0, 0.0, 1.0, 1.0);
                              
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
        patternFrequencies.push_back(ofParameter<float>(ofToString(i), freq, freq-FREQ_ADJUST_AMOUNT, freq+FREQ_ADJUST_AMOUNT));
    }
    playing = false;
    
    connecting = false;
}

Plate::~Plate() {
}

void Plate::draw(render_t renderType, int noteOrder) {
    
    
    ofEnableNormalizedTexCoords();

    ofPushMatrix();
    ofTranslate(pos);

    
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
        
//        if (ofRandom(0, 1) < 0.5) {
            frameNum++;
//            if (frameNum > NUM_FRAMES) 
            frameNum%= (NUM_FRAMES * 2);
//        }
        
        shader.begin();
        shader.setUniform1i("figure", patternNum);
        shader.setUniform1i("lastFigure", lastPatternNum);
        shader.setUniform1i("frame", (frameNum < NUM_FRAMES) ? frameNum : 2*NUM_FRAMES-frameNum-1);
        shader.setUniformTexture("tex", *imgMap, 0);
        
        shader.setUniform1f("blend", MIN(1.0, (frameSinceChange++ / BLEND_FRAMES)));
        
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

    if (playing) {
        patternFrequencies[patternNum];
        return osc.sinewave(patternFrequencies[patternNum]);
    }
    
    return 0.0f;
}

void Plate::setPatternNum(int n) {

    lastPatternNum = patternNum;
    patternNum = n;
    
    frameSinceChange = 0;
    
    playing = true;
}
