// Copyright 2013 Pervasive Displays, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
//
//   http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing,
// software distributed under the License is distributed on an
// "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
// express or implied.  See the License for the specific language
// governing permissions and limitations under the License.


#include <Arduino.h>
#include <limits.h>
#include "EPD2.h"
#include "EPD_SPI.h"
#include "EPD_BASE.h"


EPD_Class::EPD_Class(EPD_size size,
                     uint8_t panel_on_pin,
                     uint8_t border_pin,
                     uint8_t discharge_pin,
                     uint8_t reset_pin,
                     uint8_t busy_pin,
                     uint8_t chip_select_pin) :

        EPD_Pin_PANEL_ON(panel_on_pin),
        EPD_Pin_BORDER(border_pin),
        EPD_Pin_DISCHARGE(discharge_pin),
        EPD_Pin_RESET(reset_pin),
        EPD_Pin_BUSY(busy_pin),
        EPD_Pin_EPD_CS(chip_select_pin) {

    this->size = size;
    this->setFactor(); // ensure default temperature

    // display size dependant items

    static uint8_t cs[] = {0x72, 0x00, 0x00, 0x00, 0x7f, 0xff, 0xfe, 0x00, 0x00};
    this->lines_per_display = 176;
    this->dots_per_line = 264;
    this->bytes_per_line = 264 / 8;
    this->bytes_per_scan = 176 / 4;
    this->channel_select = cs;
    this->channel_select_length = sizeof(cs);
    this->voltage_level = 0x00;


}


void EPD_Class::quick_frame_data_sprite(SpritePainter *SpritePainter, TextPainter *TxtPainter) {

#define NO_START_LINE 0xff

    uint8_t line_data[33]{0x00};
    uint8_t line_data_old[33]{0x00};
    uint8_t line, start_line = NO_START_LINE;;

    // Update for Sprites*********************************************++++

    if (SpritePainter != NULL) {

        for (line = 0; line < this->lines_per_display; line++) {

            if (start_line == NO_START_LINE) { start_line = line; }

            if (SpritePainter->UpdateEPDLineData(line_data, line)) {

                // if liness are the same and no start-line defined, set start_line with first identical line
                // line_data_old - this line must be written to panel, when a new line is found.

                if ((memcmp(line_data, line_data_old, 33) != 0) || (line == this->lines_per_display - 1)) {

                    paint_line_data(line_data_old, start_line, line, EPD_inverse, this->compensation->stage2_t1);

                    paint_line_data(line_data_old, start_line, line, EPD_normal, this->compensation->stage2_t1);

                    start_line = line;

                    // swap arrary
                    memcpy(line_data_old, line_data, 33);
                }
            }

        }
    }

    if (TxtPainter != NULL) {
        // Update for Text *********************************************++++

        memset(line_data, 0, sizeof(line_data));

        for (line = 0; line < this->lines_per_display; line++) {
            if (TxtPainter->UpdateEPDLineData(line_data, line)) {

                paint_line_data(line_data, line, line, EPD_inverse, this->compensation->stage2_t1 / 2);
                paint_line_data(line_data, line, line, EPD_normal, this->compensation->stage2_t1);
                memset(line_data, 0, sizeof(line_data));

            }
        }
    }

}

long EPD_Class::paint_line_data(const uint8_t *line_data, uint8_t start_line, uint8_t finish_line, EPD_stage stage, long stage_time) {

    do {

        unsigned long t_start = millis();
        this->quick_line(start_line, finish_line, line_data, 0, false, stage, true, false);
        unsigned long t_end = millis();

        if (t_end > t_start) {
            stage_time -= t_end - t_start;
        }
        else {
            stage_time -= t_start - t_end + 1 + ULONG_MAX;
        }

    } while (stage_time > 0);

    return stage_time;
}



// One frame of data is the number of lines * rows. For example:
// The 1.44” frame of data is 96 lines * 128 dots.
// The 2” frame of data is 96 lines * 200 dots.
// The 2.7” frame of data is 176 lines * 264 dots.

// the image is arranged by line which matches the display size
// so smallest would have 96 * 32 bytes

void EPD_Class::frame_fixed_timed(uint8_t fixed_value, long stage_time) {
    do {
        unsigned long t_start = millis();
        for (uint8_t line = 0; line < this->lines_per_display; ++line) {
            this->line(this->lines_per_display - line - 1, 0, fixed_value, false);
        }
        unsigned long t_end = millis();
        if (t_end > t_start) {
            stage_time -= t_end - t_start;
        } else {
            stage_time -= t_start - t_end + 1 + ULONG_MAX;
        }
    } while (stage_time > 0);
}


void EPD_Class::quick_frame_fixed_timed(uint8_t fixed_value, long stage_time) {
    do {
        unsigned long t_start = millis();

        this->quick_line(0, this->lines_per_display, 0, fixed_value, false, EPD_normal, true, false);

        unsigned long t_end = millis();
        if (t_end > t_start) {
            stage_time -= t_end - t_start;
        } else {
            stage_time -= t_start - t_end + 1 + ULONG_MAX;
        }
    } while (stage_time > 0);
}


void EPD_Class::frame_fixed_13(uint8_t value, EPD_stage stage) {

    int repeat;
    int step;
    int block;
    if (EPD_inverse == stage) {  // stage 1
        repeat = this->compensation->stage1_repeat;
        step = this->compensation->stage1_step;
        block = this->compensation->stage1_block;
    } else {                     // stage 3
        repeat = this->compensation->stage3_repeat;
        step = this->compensation->stage3_step;
        block = this->compensation->stage3_block;
    }

    int total_lines = this->lines_per_display;

    for (int n = 0; n < repeat; ++n) {

        int block_begin = 0;
        int block_end = 0;

        while (block_begin < total_lines) {

            block_end += step;
            block_begin = block_end - block;
            if (block_begin < 0) {
                block_begin = 0;
            } else if (block_begin >= total_lines) {
                break;
            }

            bool full_block = (block_end - block_begin == block);

            for (int line = block_begin; line < block_end; ++line) {
                if (line >= total_lines) {
                    break;
                }
                if (full_block && (line < (block_begin + step))) {
                    this->line(line, 0, 0x00, false, EPD_normal);
                } else {
                    this->line(line, 0, value, false, EPD_normal);
                }
            }
        }
    }
}


void EPD_Class::frame_data_13(const uint8_t *image, EPD_stage stage, bool read_progmem) {


    //uint16_t stage1_repeat; 2
    //uint16_t stage1_step; 8
    //uint16_t stage1_block; 64


    int repeat;
    int step;
    int block;
    if (EPD_inverse == stage) {  // stage 1
        repeat = this->compensation->stage1_repeat;
        step = this->compensation->stage1_step;
        block = this->compensation->stage1_block;
    } else {                     // stage 3
        repeat = this->compensation->stage3_repeat;
        step = this->compensation->stage3_step;
        block = this->compensation->stage3_block;
    }

    int total_lines = this->lines_per_display;


    for (int n = 0; n < repeat; ++n) {

        int block_begin = 0;
        int block_end = 0;

        while (block_begin < total_lines) {

            block_end += step;
            block_begin = block_end - block;
            if (block_begin < 0) {
                block_begin = 0;
            } else if (block_begin >= total_lines) {
                break;
            }

            bool full_block = (block_end - block_begin == block);

//            Serial.print("Block:");Serial.print(block_begin);Serial.print(" - ");Serial.println(block_end);

            for (int line = block_begin; line < block_end; ++line) {
                if (line >= total_lines) {
                    break;
                }

                Serial.print(line);

                if (full_block && (line < (block_begin + step))) {
//                    Serial.println(" EMPTY");
                    this->line(line, 0, 0x00, false, EPD_normal);
                } else {
                    //                  Serial.println(" Full");
                    this->line(line, &image[line * this->bytes_per_line], 0x00, read_progmem, stage);
                }
            }
        }
    }
}


void EPD_Class::frame_cb_13(uint32_t address, EPD_reader *reader, EPD_stage stage) {
    uint8_t buffer[264 / 8]; // allows for 2.70" panel
    int repeat;
    int step;
    int block;
    if (EPD_inverse == stage) {  // stage 1
        repeat = this->compensation->stage1_repeat;
        step = this->compensation->stage1_step;
        block = this->compensation->stage1_block;
    } else {                     // stage 3
        repeat = this->compensation->stage3_repeat;
        step = this->compensation->stage3_step;
        block = this->compensation->stage3_block;
    }

    int total_lines = this->lines_per_display;

    for (int n = 0; n < repeat; ++n) {

        int block_begin = 0;
        int block_end = 0;

        while (block_begin < total_lines) {

            block_end += step;
            block_begin = block_end - block;
            if (block_begin < 0) {
                block_begin = 0;
            } else if (block_begin >= total_lines) {
                break;
            }

            bool full_block = (block_end - block_begin == block);

            for (int line = block_begin; line < block_end; ++line) {
                if (line >= total_lines) {
                    break;
                }
                if (full_block && (line < (block_begin + step))) {
                    this->line(line, 0, 0x00, false, EPD_normal);
                } else {
                    reader(buffer, address + line * this->bytes_per_line, this->bytes_per_line);
                    this->line(line, buffer, 0, false, stage);
                }
            }
        }
    }
}


void EPD_Class::frame_stage2() {
    for (uint16_t i = 0; i < this->compensation->stage2_repeat; ++i) {
        this->frame_fixed_timed(0xff, this->compensation->stage2_t1);
        this->frame_fixed_timed(0xaa, this->compensation->stage2_t2);
    }
}

void EPD_Class::quick_frame_stage2() {
    for (uint16_t i = 0; i < this->compensation->stage2_repeat; ++i) {
        this->quick_frame_fixed_timed(0xff, this->compensation->stage2_t1 * 6);
        this->quick_frame_fixed_timed(0xaa, this->compensation->stage2_t2 * 6);
    }

}


void EPD_Class::quick_line(uint16_t line_start, uint16_t line_finish, const uint8_t *data, uint8_t fixed_value,
                           bool read_progmem, EPD_stage stage, uint8_t border_byte, bool set_voltage_limit) {

    uint8_t line_byte;
    int current_line, i;
    uint16_t line_tmp;

    // fix - y-achse is mirrowed
    line_tmp = 176 - line_start;
    line_start = 176 - line_finish;
    line_finish = line_tmp;


/*
    Serial.print(line_start);
    Serial.print("-");
    Serial.println(line_finish);


    if (0 != data) {
        Serial.print("Data:");
        for (i = 0; i < this->bytes_per_line; i++) {
            Serial.print(data[i]);
        }
        Serial.println();
    } else {
        Serial.print("Fixed:");
        Serial.println(fixed_value);
    }
*/

    EPD_SPI::SPI_on();

    if (set_voltage_limit) {
        // charge pump voltage level reduce voltage shift
        EPD_SPI::SPI_send(this->EPD_Pin_EPD_CS, CU8(0x70, 0x04), 2);
        uint8_t tmp[] = {0x72, this->voltage_level};
        EPD_SPI::SPI_send(this->EPD_Pin_EPD_CS, tmp, 2);
    }

    // send data
    EPD_SPI::SPI_send(this->EPD_Pin_EPD_CS, CU8(0x70, 0x0a), 2);

    Delay_us(10);

    // CS low
    digitalWrite(this->EPD_Pin_EPD_CS, LOW);
    EPD_SPI::SPI_put(0x72);

    // border byte
    EPD_SPI::SPI_put(border_byte);

    // odd pixels
    for (uint16_t b = this->bytes_per_line; b > 0; --b) {
        if (0 != data) {
            // AVR has multiple memory spaces
            uint8_t pixels;
            if (read_progmem) {
                pixels = pgm_read_byte_near(data + b - 1);
            } else {
                pixels = data[b - 1];
            }

            switch (stage) {
                case EPD_inverse:      // B -> W, W -> B
                    pixels ^= 0xff;
                    break;
                case EPD_normal:       // B -> B, W -> W
                    break;
            }
            pixels = 0xaa | pixels;
            EPD_SPI::SPI_put(pixels);
        } else {
            EPD_SPI::SPI_put(fixed_value);
        }
    }

    // scan line

    line_byte = 0;
    for (current_line = 0; current_line < this->lines_per_display; current_line++) {
        if ((current_line >= line_start) && (current_line <= line_finish)) {
            line_byte = line_byte | (0xc0 >> (2 * (current_line & 0x03)));
        }
        if (((current_line + 1) % 4) == 0) {  //divided by 4, one byte has 4 pixels

            EPD_SPI::SPI_put(line_byte);
            line_byte = 0;

        }
    }

    // even pixels
    for (uint16_t b = 0; b < this->bytes_per_line; ++b) {
        if (0 != data) {

            // AVR has multiple memory spaces
            uint8_t pixels;
            if (read_progmem) {
                pixels = pgm_read_byte_near(data + b);
            } else {
                pixels = data[b];
            }

            switch (stage) {
                case EPD_inverse:      // B -> W, W -> B (Current Image)
                    pixels ^= 0xff;
                    break;
                case EPD_normal:       // B -> B, W -> W (New Image)
                    break;
            }
            pixels >>= 1;
            pixels |= 0xaa;

            pixels = ((pixels & 0xc0) >> 6)
                     | ((pixels & 0x30) >> 2)
                     | ((pixels & 0x0c) << 2)
                     | ((pixels & 0x03) << 6);
            EPD_SPI::SPI_put(pixels);
        } else {
            EPD_SPI::SPI_put(fixed_value);
        }
    }

    // CS high
    digitalWrite(this->EPD_Pin_EPD_CS, HIGH);

    // output data to panel
    EPD_SPI::SPI_send(this->EPD_Pin_EPD_CS, CU8(0x70, 0x02), 2);
    EPD_SPI::SPI_send(this->EPD_Pin_EPD_CS, CU8(0x72, 0x07), 2);

    EPD_SPI::SPI_off();
}


void EPD_Class::line(uint16_t line, const uint8_t *data, uint8_t fixed_value,
                     bool read_progmem, EPD_stage stage, uint8_t border_byte,
                     bool set_voltage_limit) {

    EPD_SPI::SPI_on();

    if (set_voltage_limit) {
        // charge pump voltage level reduce voltage shift
        EPD_SPI::SPI_send(this->EPD_Pin_EPD_CS, CU8(0x70, 0x04), 2);
        uint8_t tmp[] = {0x72, this->voltage_level};
        EPD_SPI::SPI_send(this->EPD_Pin_EPD_CS, tmp, 2);
    }

    // send data
    EPD_SPI::SPI_send(this->EPD_Pin_EPD_CS, CU8(0x70, 0x0a), 2);

    Delay_us(10);

    // CS low
    digitalWrite(this->EPD_Pin_EPD_CS, LOW);
    EPD_SPI::SPI_put(0x72);

    // border byte
    EPD_SPI::SPI_put(border_byte);

    // odd pixels
    for (uint16_t b = this->bytes_per_line; b > 0; --b) {
        if (0 != data) {
            // AVR has multiple memory spaces
            uint8_t pixels;
            if (read_progmem) {
                pixels = pgm_read_byte_near(data + b - 1);
            } else {
                pixels = data[b - 1];
            }

            switch (stage) {
                case EPD_inverse:      // B -> W, W -> B
                    pixels ^= 0xff;
                    break;
                case EPD_normal:       // B -> B, W -> W
                    break;
            }
            pixels = 0xaa | pixels;
            EPD_SPI::SPI_put(pixels);
        } else {
            EPD_SPI::SPI_put(fixed_value);
        }
    }

    // scan line
    int scan_pos = (this->lines_per_display - line - 1) >> 2;
    int scan_shift = (line & 0x03) << 1;
    for (int b = 0; b < this->bytes_per_scan; ++b) {
        if (scan_pos == b) {
            EPD_SPI::SPI_put(0x03 << scan_shift);
        } else {
            EPD_SPI::SPI_put(0x00);
        }
    }

    // even pixels
    for (uint16_t b = 0; b < this->bytes_per_line; ++b) {
        if (0 != data) {

            // AVR has multiple memory spaces
            uint8_t pixels;
            if (read_progmem) {
                pixels = pgm_read_byte_near(data + b);
            } else {
                pixels = data[b];
            }

            switch (stage) {
                case EPD_inverse:      // B -> W, W -> B (Current Image)
                    pixels ^= 0xff;
                    break;
                case EPD_normal:       // B -> B, W -> W (New Image)
                    break;
            }
            pixels >>= 1;
            pixels |= 0xaa;

            pixels = ((pixels & 0xc0) >> 6)
                     | ((pixels & 0x30) >> 2)
                     | ((pixels & 0x0c) << 2)
                     | ((pixels & 0x03) << 6);
            EPD_SPI::SPI_put(pixels);
        } else {
            EPD_SPI::SPI_put(fixed_value);
        }
    }

    // CS high
    digitalWrite(this->EPD_Pin_EPD_CS, HIGH);

    // output data to panel
    EPD_SPI::SPI_send(this->EPD_Pin_EPD_CS, CU8(0x70, 0x02), 2);
    EPD_SPI::SPI_send(this->EPD_Pin_EPD_CS, CU8(0x72, 0x07), 2);

    EPD_SPI::SPI_off();
}

void  EPD_Class::setFactor(int temperature) {
    // stage1: repeat, step, block
    // stage2: repeat, t1, t2
    // stage3: repeat, step, block


    //
//uint16_t stage1_repeat; 2
//uint16_t stage1_step; 4
//uint16_t stage1_block; 32

//uint16_t stage2_repeat;
//uint16_t stage2_t1;
//uint16_t stage2_t2;

//uint16_t stage3_repeat;
//uint16_t stage3_step;
//uint16_t stage3_block;

    static const EPD_Class::compensation_type compensation_270[3] = {
            {2, 8, 64, 4, 392, 392, 2, 8, 64},  //  0 ... 10 Celcius
            {1, 8, 32, 1, 49,  48,  1, 8, 32},  // 10 ... 40 Celcius
            {4, 8, 64, 4, 196, 196, 4, 8, 64}   // 40 ... 50 Celcius
    };

    // old for range of 20 Celsius
    // { 1, 8, 32,   2, 98, 98,   1, 8, 32 },  // 10 ... 40 Celcius

    if (temperature < 10) {
        this->temperature_offset = 0;
    } else if (temperature > 40) {
        this->temperature_offset = 2;
    } else {
        this->temperature_offset = 1;
    }
    this->compensation = &compensation_270[this->temperature_offset];
}

void EPD_Class::nothing_frame() {
    for (uint16_t line = 0; line < this->lines_per_display; ++line) {
        this->line(line, 0, 0x00, false, EPD_normal, EPD_BORDER_BYTE_NULL, true);
    }
}


void EPD_Class::dummy_line() {
    this->line(0x7fffu, 0, 0x00, false, EPD_normal, EPD_BORDER_BYTE_NULL, true);
}


void EPD_Class::border_dummy_line() {
    this->line(0x7fffu, 0, 0x00, false, EPD_normal, EPD_BORDER_BYTE_BLACK);
    Delay_ms(40);
    this->line(0x7fffu, 0, 0x00, false, EPD_normal, EPD_BORDER_BYTE_WHITE);
    Delay_ms(200);
}

void EPD_Class::paint_display(SpritePainter *spritePainter, TextPainter *textPainter) {
    EPD_BASE::init();
    EPD_BASE::begin(this); // power up the EPD panel
    this->quick_frame_stage2();
    this->quick_frame_data_sprite(spritePainter, textPainter);
    EPD_BASE::end(this);   // power down the EPD panel
}
