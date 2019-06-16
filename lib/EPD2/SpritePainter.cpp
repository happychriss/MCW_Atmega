//
// Created by development on 8/11/15.
//

#include <stddef.h>
#include <string.h>
#include <limits.h>

// #include <HardwareSerial.h>
#include "SpritePainter.h"


 SpritePainter::SpritePainter(){
     current_sprite=&sprites[0];

 };



void SpritePainter::AddScreenLinePattern(uint8_t data_line_pattern[]){
    line_pattern=data_line_pattern;
};

uint8_t* SpritePainter::GetScreenLinePattern(){
    return line_pattern;
};


bool SpritePainter::IsLineInPattern(uint8_t line) {
    int i;

    for (i=0;i<9;i++){
        if (line==line_pattern[i]) {
            return true;
        }
    }
    return false;
}

void SpritePainter::AddSprite(Sprite *sprite){
    if (sprite==NULL) {
        current_sprite->x=0;
        current_sprite->y=0;
        current_sprite->item=NULL;
    }   else {
        current_sprite=sprite;
        current_sprite++;
    }

}


 void SpritePainter::EndList(){
     current_sprite->x=0;
     current_sprite->y=0;
     current_sprite->item=NULL;

};

void SpritePainter::StartList() {
    current_sprite=&sprites[0];
};

bool SpritePainter::UpdateEPDLineData(uint8_t *line_data, uint8_t line) {

    if (IsLineInPattern(line)) {

        bool p_found, b_equal_line;
        int x_byte_shift,n,x_byte,x;
        uint8_t mask, x_bit;

        // loop over each pixel and check if used by a sprite
        for (x_byte = 0; x_byte < 33; x_byte++) {

            line_data[x_byte] = 0;
            mask = 1;


            x_byte_shift = x_byte << 3;

            for (x_bit = 0; x_bit < 8; x_bit++) {

                x = x_byte_shift + x_bit; //this is the x position to be tested

                // looop through all sprites and check if a pixel is set (x,y)
                for (n = 0; (n < MAX_SPRITES) && (sprites[n].item != NULL); n++) {

                    p_found = ((x >= sprites[n].x) && (x < (sprites[n].x + sprites[n].item->x_l)) &&
                               (line >= sprites[n].y) &&
                               (line < (sprites[n].y + sprites[n].item->y_h)));

                    if (p_found) {
                        line_data[x_byte] = line_data[x_byte] + mask;
                        break;
                    }
                }

                mask = mask << 1;
            }


        }
        return true;
    } else {
        return false;
    }
}

