//
//  ChladniDB.cpp
//  lexus_plates
//
//  Created by Will Gallia on 10/06/2015.
//
//

#include "ChladniDB.h"

std::map<int, CPattern> ChladniDB::patterns;

void ChladniDB::setup() {
    
    patterns[0] = CPattern(SINE, 35);
    patterns[1] = CPattern(SINE, 41);
    patterns[2] = CPattern(SINE, 71);
    patterns[3] = CPattern(SINE, 79);
    patterns[4] = CPattern(SINE, 88);
    patterns[5] = CPattern(SINE, 149);
    patterns[6] = CPattern(SINE, 212);
    patterns[7] = CPattern(SINE, 330);
    patterns[8] = CPattern(SINE, 420);
    patterns[9] = CPattern(SINE, 467);
    patterns[10] = CPattern(SINE, 624);


//    patterns[0] = CPattern(SINE, 35);
//    patterns[1] = CPattern(SINE, 41);
//    patterns[2] = CPattern(SINE, 71);
//    patterns[3] = CPattern(SINE, 79);
//    patterns[4] = CPattern(SINE, 88);
//    patterns[5] = CPattern(SINE, 149);
//    patterns[6] = CPattern(SINE, 212);
//    patterns[7] = CPattern(SINE, 330);
//    patterns[8] = CPattern(SINE, 420);
//    patterns[9] = CPattern(SINE, 467);
//    patterns[10] = CPattern(SINE, 624);

}