//
//  Plate.cpp
//  lexus_plates
//
//  Created by Will Gallia on 08/06/2015.
//
//

#include "Plate.h"


ofImage *imgMap = NULL;
ofTrueTypeFont *font = NULL;

static int audioChannelCounter = 0;

Plate::Plate(float x, float y, float w): pos(x, y), size(w), listeningForNote(false), tint(false) {
    
    fadeInFrames = 5;
    
    mesh = ofMesh::plane(w, w);
    
    if (!imgMap) {
        imgMap = new ofImage();
        imgMap->load("map.png");
    }
    
    if (!font) {
        font = new ofTrueTypeFont();
        font->load("/Library/Fonts/Arial.ttf", 48, false, true, true);
    }
    
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
                          uniform vec2 size = vec2(128.0);
                          
                          void main() {
                              vec4 c = texture2DRect(tex, vec2((gl_TexCoord[0].x / 8.0) + float(frame) * size.x, gl_TexCoord[0].y));
                              gl_FragColor = c; //vec4(texc.x, texc.y, 1.0, 1.0); //c; //vec4(1.0, 0.0, 1.0, 1.0);
                              
                          }
                          );
    
    shader.setupShaderFromSource(GL_FRAGMENT_SHADER, frag);
    shader.linkProgram();
    
    for (int i = 0; i < MAX_NOTES; i++) {
        noteOrders[i] = 1;
    }
    
    audioChannel.set("audio channel", audioChannelCounter++, 0, 6);
    volume.set("volume", 0.5, 0.0, 1.0);
    
    connecting = false;
}

Plate::~Plate() {
    if (imgMap) {
        delete imgMap;
        imgMap = NULL;
    }
    if (font) {
        delete font;
        font = NULL;
    }
}

void Plate::draw(render_t renderType, int noteOrder) {
    
    
    ofEnableNormalizedTexCoords();

    ofPushMatrix();
    ofTranslate(pos);
//    ofScale(1, -1);

    
    if (renderType == PICK) {
        
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
    else if (renderType == NORMAL) {
    
        imgMap->getTexture().bind();
        
        if (ofRandom(0, 1) < 0.5) {
            frameNum++;
            frameNum%= 8;
        }
        
        shader.begin();
        shader.setUniform1i("figure", figureNum);
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