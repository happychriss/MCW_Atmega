//
// Created by development on 10/10/15.
//

#include <string.h>
#include <bitmap_db.h>
#include <HardwareSerial.h>


// can be removed in real app
// #include "arial18.cpp"
#include "TextPainter.h"


TextPainter::TextPainter() {
    height = 18;

    /*flash_address = pgm_read_word(&chrtbl_f16[uniCode]);
    width = pgm_read_byte(widtbl_f16 + uniCode);
    height = chr_hgt_f16;*/

}


void TextPainter::BuildText(TxtLine *in_Text, uint8_t in_cnt_txt) {
    Text = in_Text;
    cnt_txt = in_cnt_txt;
}


bool TextPainter::UpdateEPDLineData(uint8_t *line_data, uint8_t y_line) {


    bool changed_line = false;

    for (uint8_t i = 0; i < cnt_txt; i++) {  //loop through all text elements

        uint16_t text_y = Text[i].y;
        uint16_t text_x = Text[i].x;
//        uint8_t size = Text[i].size;
#define size 1
        uint16_t x_offset = text_x;

        // one line before and after character will be painted empty for better readability
        if ((y_line == text_y - 1) || (y_line == text_y + FONT_HEIGHT * size + 1)) {
            changed_line = true;
            break;
        }

        // only paint, if text is in relevant area of FONT_HEIGHT
        if ((y_line >= text_y) && (y_line < text_y + FONT_HEIGHT * size)) {

            for (uint8_t s = 0; s < strlen(Text[i].txt); s++) {  //loop through all characters

//                unsigned char txt_to_write = (uint8_t) Text[i].txt[s] - arial_36ptFontInfo.startChar;
                unsigned char txt_to_write = (uint8_t) Text[i].txt[s] - 32;

//                uint8_t *data = (unsigned char *) chrtbl_f32[txt_to_write];
                uint8_t *data = (uint8_t *) pgm_read_word(&chrtbl_f32[txt_to_write]);

//                uint8_t height = arial_36ptDescriptors[txt_to_write].heightBits;
//                uint8_t width = arial_36ptDescriptors[txt_to_write].widthBits;

                uint8_t width = pgm_read_byte(&data[0]);
                uint8_t height = pgm_read_byte(&data[1]);

                // current y-line in the charchater arrya data text to be checked
                int y_char_line = (y_line - text_y - (FONT_HEIGHT - height)) / size;

                // now we are in the line were the real character starts

                if (y_char_line >= 0) {

                    changed_line = true;

/*
                    // RLE decode
                    uint8_t data[500] = {0};
                    EncodeRLE encode;
                    encode.RLEEncoder(txt_to_write, data);
*/
//                  uint8_t *data = (unsigned char *) chrtbl_f32[txt_to_write];


                    // start position in the character data to use to start
                    uint16_t start_pos = (uint8_t) y_char_line * width;

                    uint8_t z = 2; //start with second byte, byte 0 and 1 have hight and width

                    //***************************************************
                    // find the right position in RLE data: Z: number of byte, rle - number of bits to paint

                    uint8_t rle = 0;
                    bool b_black;

                    if (start_pos == 0) {
                        b_black = (bool) (pgm_read_byte(&data[z]) & 0x80);
                        rle = (uint8_t) (pgm_read_byte(&data[z]) & 0x7f);
                    } else {

                        uint16_t rle_pos = (uint16_t) (pgm_read_byte(&data[z]) & 0x7f);

                        while (true) {

                            if (rle_pos > start_pos) {

//                                rle_offset = start_pos - (rle_pos - data_tmp);
//                                rle = data[z]- (start_pos - (rle_pos - data_tmp)); // this is the magic, dont change
//                                rle = data[z] - start_pos + rle_pos - data_tmp; // this is the magic, dont change

                                b_black = (bool) (pgm_read_byte(&data[z]) & 0x80);
                                rle = (uint8_t) (rle_pos - start_pos); // this is the magic, dont change

                                break;
                            };

                            z++;

                            rle_pos = rle_pos + (uint16_t) (pgm_read_byte(&data[z]) & 0x7f);

                        }
                    }

                    //// ********************+ LOOOOP

                    uint8_t x_bit = 0;

                    while (rle != 0) {

                        // pixel must be set
                        if (b_black) {

                            while (rle--) {

                                if (x_bit < width) {
                                    uint16_t bit = x_offset + x_bit;
                                    line_data[bit / 8] = (uint8_t) (line_data[bit / 8] | 1 << ((bit % 8)));
                                    x_bit++;
                                } else {
                                    break;
                                }
                            }

                        } else {

                            x_bit = x_bit + rle;
                            if (x_bit >= width) {
                                break;
                            }
                        }

                        z++;
                        b_black = (bool) (pgm_read_byte(&data[z]) & 0x80);
                        rle = (uint8_t) (pgm_read_byte(&data[z]) & 0x7f);

                    }


                }

                x_offset = x_offset + width * size + 3;
            }
        }


    }

    return changed_line;
}



/*bool TextPainter::UpdateEPDLineData(uint8_t *line_data, uint8_t y_line) {



    uint16_t x_offset = 0;
    bool changed_line = false;

    for (uint8_t i = 0; i < cnt_txt; i++) {  //loop through all text elements

        uint16_t text_y=Text[i].y;
        uint16_t text_x=Text[i].x;
        uint8_t size=Text[i].size;

        if ((y_line == text_y-1) || (y_line == text_y + height*size+1)) {
            changed_line=true;
            break;
        }

        // only paint, if text is in relevant area
        if ((y_line >= text_y) && (y_line < text_y + height*size)) {

            changed_line = true;

            uint8_t y_char_line = (y_line - text_y)/size; // current y-line in the charchater arrya data text to be checked

            for (uint8_t s = 0; s < strlen(Text[i].txt); s++) {  //loop through all characters

                unsigned char txt_to_write = (uint8_t) Text[i].txt[s] - 33; //offset comma

//                offset = arial_18ptDescriptors[txt_to_write].offset;
//                y_char_line_data =18-arial_18ptDescriptors[txt_to_write].heightBits; /// this is the first line, we have data in the character array.
//                width = arial_18ptDescriptors[txt_to_write].widthBits;

                uint16_t offset = pgm_read_word(&arial_18ptDescriptors[txt_to_write].offset);
                uint8_t y_char_line_data = 18 - pgm_read_byte(&arial_18ptDescriptors[txt_to_write].heightBits); /// this is the first line, we have data in the character array.
                uint8_t width = pgm_read_byte(&arial_18ptDescriptors[txt_to_write].widthBits);

                // only paint if the characters first lines contains something to paint
                if (y_char_line >= y_char_line_data) {

                    uint8_t y_char = y_char_line - y_char_line_data; //line that must be read from fonts-data
                    uint8_t w = (width + 7) / 8; // number of bytes to read from character

                    // Current byte and bit that have to be changed in line_data
                    uint8_t x_char_start_byte = (text_x + x_offset) / 8;
                    uint8_t x_char_start_bit = ((text_x + x_offset) % 8);

                    for (int k = 0; k < w*size; k++) {

//                        line = arial_18ptBitmaps[offset+w * y_char_line + k];

                        uint8_t current_byte = x_char_start_byte + k/size;

                        uint8_t line = pgm_read_byte(&arial_18ptBitmaps[offset + w * y_char + k/size]);

                        // adjust line to x position (move bits) and collect fallen out bits in line_rest
                        uint8_t line_rest = line >> (8 - x_char_start_bit);
                        line = line << x_char_start_bit;

                        line_data[current_byte] = line_data[current_byte] | line;
                        line_data[current_byte + 1] = line_data[current_byte + 1] | line_rest;

                    }
                }
                x_offset = x_offset + width*size + 5;

            }
            x_offset = 0;
        }


    }

    return changed_line;
}*/











