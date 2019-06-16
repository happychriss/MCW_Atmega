//
// Created by development on 9/19/15.
//

#ifndef MYWATCH_CLOCKPAINTER_H
#define MYWATCH_CLOCKPAINTER_H

#include <stddef.h>
// #include <avr/pgmspace.h>
#include "SpritePainter.h"

#define MAX_ITEMS 2
#define SEGMENTS 7

// segments to be enabled per digit
// const uint8_t SegmentsPerNumbers[] PROGMEM  = {119, 36, 93, 109, 46, 107, 123, 37, 127, 111};
const uint8_t SegmentsPerNumbers[] = {119, 36, 93, 109, 46, 107, 123, 37, 127, 111};

class DigitalClockBuilder {


    SpritePainter Painter;
    Item items[MAX_ITEMS]= {};
    Sprite Segments[SEGMENTS]= {};;

    Pos DigitOffset_Time[4]= {};
    uint8_t ScreenPattern[9]= {};

    void BuildNumber(int digit, int display_number);

public:

    SpritePainter BuildClock(uint8_t d1, uint8_t d2, uint8_t d3, uint8_t d4);
    DigitalClockBuilder(uint8_t seg_lenght, uint8_t seg_hight, uint8_t doff);

    void InitClock(uint8_t seg_lenght, uint8_t seg_hight, uint8_t doff);

};


#endif //MYWATCH_CLOCKPAINTER_H
