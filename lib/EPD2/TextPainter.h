//
// Created by development on 10/10/15.
//

#ifndef MYWATCH_TEXTPAINTER_H
#define MYWATCH_TEXTPAINTER_H


#include <string.h>
#include "TextPainter.h"
#include <inttypes.h>
#include <ctype.h>
#include "arial_36_rle.c"




struct TxtLine {
    uint16_t x;
    uint16_t y;
    uint8_t size;
    char *txt;

};

class TextPainter {

        uint8_t *line_pattern;
        TxtLine *Text;
        uint8_t cnt_txt;
        uint8_t height;

    public:
        TextPainter();
        void BuildText(TxtLine *in_Text, uint8_t in_cnt_txt);
        bool UpdateEPDLineData(uint8_t *line_data, uint8_t y_line);
    };


#endif //MYWATCH_TEXTPAINTER_H
