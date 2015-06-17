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
    
    patterns[0] = CPattern(SINE, 100.1);
    patterns[1] = CPattern(SINE, 168.8);
    patterns[2] = CPattern(SINE, 208.1);
    patterns[3] = CPattern(SINE, 360);
    patterns[4] = CPattern(SINE, 385);
    patterns[5] = CPattern(SINE, 525);
    patterns[6] = CPattern(SINE, 545);
    patterns[7] = CPattern(SINE, 573);
    patterns[8] = CPattern(SINE, 1227);
    patterns[9] = CPattern(SINE, 2084);
    
    patterns[10] = CPattern(SINE, 476);
    patterns[11] = CPattern(SINE, 606);
    patterns[12] = CPattern(SINE, 957);
    patterns[13] = CPattern(SINE, 2754);
    
//    patterns[0] = CPattern(SINE, 107.25);
//    patterns[1] = CPattern(SINE, 165.55);
//    patterns[2] = CPattern(SINE, 208.1);
//    patterns[3] = CPattern(SINE, 340);
//    patterns[4] = CPattern(SINE, 400);
//    patterns[5] = CPattern(SINE, 505);
//    patterns[6] = CPattern(SINE, 525);
//    patterns[7] = CPattern(SINE, 590);
//    patterns[8] = CPattern(SINE, 1277);
//    patterns[9] = CPattern(SINE, 1682);
//    patterns[10] = CPattern(SINE, 2073);
//    
//    
//    patterns[11] = CPattern(SINE, 463);
//    patterns[12] = CPattern(SINE, 604);
//    patterns[13] = CPattern(SINE, 606);
//    patterns[14] = CPattern(SINE, 953);
//    patterns[15] = CPattern(SINE, 2743);



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