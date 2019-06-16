//
// Created by development on 9/19/15.
//

#include "DigitalClockBuilder.h"


DigitalClockBuilder::DigitalClockBuilder(uint8_t seg_lenght, uint8_t seg_hight, uint8_t doff) {

    items[0].x_l = seg_lenght;
    items[0].y_h = seg_hight;
    items[1].x_l = seg_hight;
    items[1].y_h = seg_lenght;

    Segments[0] = {0, 0, &items[0]}; //1
    Segments[1] = {0, 0, &items[1]}; //2
    Segments[2] = {seg_lenght - seg_hight, 0, &items[1]};//3
    Segments[3] = {0, seg_lenght - seg_hight, &items[0]};//4
    Segments[4] = {0, seg_lenght - seg_hight, &items[1]};//5
    Segments[5] = {seg_lenght - seg_hight, seg_lenght - seg_hight, &items[1]};//6
    Segments[6] = {0, 2 * (seg_lenght - seg_hight), &items[0]};//7


    DigitOffset_Time[0]={20,  doff};
    DigitOffset_Time[1]={56,  doff};
    DigitOffset_Time[2]={102,  doff};
    DigitOffset_Time[3]={137,  doff};


    ScreenPattern[0]=0;
    ScreenPattern[1]= doff; //22
    ScreenPattern[2]= doff +seg_hight; //28
    ScreenPattern[3]= doff + seg_lenght - seg_hight;
    ScreenPattern[4]=doff+seg_lenght;
    ScreenPattern[5]=doff+2*seg_lenght-2*seg_hight;
    ScreenPattern[6]=doff+2*seg_lenght-seg_hight;
    ScreenPattern[7]=176;
    ScreenPattern[8]=NULL;


}

SpritePainter DigitalClockBuilder::BuildClock(uint8_t d1, uint8_t d2, uint8_t d3, uint8_t d4){

    Painter.StartList();

    Painter.AddScreenLinePattern(ScreenPattern);
    BuildNumber(0, d1);
    BuildNumber(1, d2);
    BuildNumber(2, d3);
    BuildNumber(3, d4);

    Painter.EndList();

    return  Painter;

}


void DigitalClockBuilder::BuildNumber(int digit, int display_number) {

    int s;
    uint8_t mask;
    mask = 1;

    for (s = 0; s < SEGMENTS; s++) {

        // Segment must be added
//        if (mask & pgm_read_byte(&SegmentsPerNumbers[display_number])) {
            if (mask & SegmentsPerNumbers[display_number]) {
            Painter.current_sprite->item = Segments[s].item;
            Painter.current_sprite->x = Segments[s].x + DigitOffset_Time[digit].x;
            Painter.current_sprite->y = Segments[s].y + DigitOffset_Time[digit].y;
            Painter.current_sprite++;
        }
        mask = mask << 1;
    }

}