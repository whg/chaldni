//
//  ChladniDB.h
//  lexus_plates
//
//  Created by Will Gallia on 10/06/2015.
//
//

#pragma once

#include <map>

enum audio_t {
    SINE,
    SAMPLE,
};

struct CPattern {
    audio_t audioType;
    union {
        float frequency;
        char filename[50];
    } data;
    
    CPattern(){}

    CPattern(audio_t at, float f): audioType(at) {
        data.frequency = f;
    }
    
    CPattern(audio_t at, char* fn): audioType(at) {
//        data.filename = fn.c_str();
        strcpy(data.filename, fn);
    }
};

class ChladniDB {
    
    static std::map<int, CPattern> patterns;
    
public:
    ChladniDB();
    ~ChladniDB();
    
    static void setup();
    
    static CPattern patternForNo(int patternNo) {
        return patterns[patternNo];
    }
    
    
};