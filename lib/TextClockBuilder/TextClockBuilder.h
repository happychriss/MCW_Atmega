//
// Created by development on 11/14/15.
//

#ifndef MYWATCH_TEXTCLOCKBUILDER_H
#define MYWATCH_TEXTCLOCKBUILDER_H

#include <avr/pgmspace.h>


#define UHR 0
#define ES_IST  1
#define NACH    2
#define HALB    3
#define VOR     4
#define FUENF    5
#define ZEHN        6
#define VIERTEL     7
#define ZWANZIG     8
#define FUENFVOR     9
#define FUENFNACH    10


// Store numbers in PROGMEM to save RAM
const char num_string_0[] PROGMEM ="zwölf";
const char num_string_1[] PROGMEM ="eins";
const char num_string_2[] PROGMEM ="zwei";
const char num_string_3[] PROGMEM ="drei";
const char num_string_4[] PROGMEM ="vier";
const char num_string_5[] PROGMEM ="fünf";
const char num_string_6[] PROGMEM ="sechs";
const char num_string_7[] PROGMEM ="sieben";
const char num_string_8[] PROGMEM ="acht";
const char num_string_9[] PROGMEM ="neun";
const char num_string_10[] PROGMEM ="zehn";
const char num_string_11[] PROGMEM ="elf ";
const char num_string_12[] PROGMEM ="ein"; /// es ist ein uhr !!



const char* const time_numbers[] PROGMEM = {num_string_0,num_string_1,num_string_2,num_string_3,num_string_4,num_string_5,num_string_6,
                                            num_string_7,num_string_8,num_string_9,num_string_10,num_string_11,num_string_12};

const char txt_string_0[] PROGMEM ="Uhr";
const char txt_string_1[] PROGMEM ="Es ist";
const char txt_string_2[] PROGMEM =" nach";
const char txt_string_3[] PROGMEM ="halb ";
const char txt_string_4[] PROGMEM =" vor";
const char txt_string_5[] PROGMEM ="fünf";
const char txt_string_6[] PROGMEM ="zehn";
const char txt_string_7[] PROGMEM ="Viertel";
const char txt_string_8[] PROGMEM ="zwanzig";
const char txt_string_9[] PROGMEM ="fünf vor";
const char txt_string_10[] PROGMEM ="fünf nach";

const char* const time_text[] PROGMEM = {txt_string_0,txt_string_1,txt_string_2,txt_string_3,txt_string_4,
                                         txt_string_5,txt_string_6,txt_string_7,txt_string_8,txt_string_9,txt_string_10};

struct TimeTxt {
    uint8_t c;
    bool offset;
};




class TextClockBuilder {

    TimeTxt time_min[12];

public:
    TextClockBuilder();
    void BuildText(uint8_t hour,uint8_t min,char *line_1,char *line_2, char *line_3);
};


#endif //MYWATCH_TEXTCLOCKBUILDER_H
