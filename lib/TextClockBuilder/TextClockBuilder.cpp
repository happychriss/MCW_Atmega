//
// Created by development on 11/14/15.
//

#include <HardwareSerial.h>
#include "TextClockBuilder.h"

TextClockBuilder::TextClockBuilder(){

    time_min[0] = {UHR, false};
    time_min[1] = {FUENF, false};
    time_min[2] = {ZEHN, false};
    time_min[3] = {VIERTEL, false};
    time_min[4] = {ZWANZIG, false};
    time_min[5] = {FUENFVOR, true}; // tricky one..
    time_min[6] = {HALB, true};
    time_min[7] = {FUENFNACH, true};
    time_min[8] = {ZWANZIG, true};
    time_min[9] = {VIERTEL, true};
    time_min[10] = {ZEHN, true};
    time_min[11] = {FUENF, true};

}

void TextClockBuilder::BuildText(uint8_t hour, uint8_t min,char *line_1,char *line_2, char *line_3) {

    min = (min / 5);

    if (time_min[min].offset) hour++;

    // hours start with 0 for midnight / noon
    if (hour >= 12) {
        hour = hour- 12;
    }

    if (hour==12) {hour=0;}

    strcpy_P(line_1, (char*)pgm_read_word(&(time_text[ES_IST])));

// build line 2

    // replace "eins" by "ein" if full hour
    if ((min == 0) && (hour ==1)) {
        strcpy_P(line_2, (char*)pgm_read_word(&(time_numbers[12])));
        strcpy_P(line_3, (char*)pgm_read_word(&(time_text[UHR])));
    }

        // full our, this line contains the hour
    else if (min == 0) {
        strcpy_P(line_2, (char*)pgm_read_word(&(time_numbers[hour])));
        strcpy_P(line_3, (char*)pgm_read_word(&(time_text[UHR])));
    }

        // half, line will not have "vor" or "nach"
    else if (min == 6) {
        strcpy_P(line_2, (char*)pgm_read_word(&(time_text[HALB])));
        strcpy_P(line_3, (char*)pgm_read_word(&(time_numbers[hour])));

    }
        // "fünf vor" oder "fünf nach"
    else if ((min == 5)||(min==7)) {
        strcpy_P(line_2, (char*)pgm_read_word(&(time_text[time_min[min].c])));
        strcpy_P(line_3, (char*)pgm_read_word(&(time_text[HALB])));
        strcat_P(line_3,(char*)pgm_read_word(&(time_numbers[hour])));

    }

    // all the other times depending on the offset

    else {
        strcpy_P(line_2, (char*)pgm_read_word(&(time_text[time_min[min].c])));
        if (time_min[min].offset) {
            strcat_P(line_2,(char*)pgm_read_word(&(time_text[VOR])));
        } else {
            strcat_P(line_2,(char*)pgm_read_word(&(time_text[NACH])));
        }
        strcat_P(line_3,(char*)pgm_read_word(&(time_numbers[hour])));
    }

    strcat(line_3,".");

}