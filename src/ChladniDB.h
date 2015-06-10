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
};

class ChladniDB {
    
    static std::map<int, CPattern> patterns;
    
public:
    ChladniDB();
    ~ChladniDB();
    
    static CPattern patternForNo(int patternNo) {
        return patterns[patternNo];
    }
    
    
};