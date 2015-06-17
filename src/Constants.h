//
//  Constants.h
//  lexus_plates
//
//  Created by Will Gallia on 10/06/2015.
//
//

#pragma once

#define MAX_NOTES 9

#define NUM_FIGURES 9
#define FIGURE_GRID_X 5
#define FIGURE_GRID_Y 4

#define MAX_CHANNELS 5

enum render_t {
    NORMAL,
    PICK,
    NOTE_SELECT,
    MIDI_MANAGER,
    NUM_RENDER_TYPES,
};


enum modes_t {
    NORMAL_MODE,
    HWAVE,
    VWAVE,
    CWAVE,
    RANDOM,
    SNAKE,
    HSWEEP,
};

