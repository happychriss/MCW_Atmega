// Copyright 2014 Pervasive Displays, Inc.
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

#if !defined(EPD2_H)
#define EPD2_H 1

#include <Arduino.h>
#include <SPI.h>
#include <avr/pgmspace.h>
#include "SpritePainter.h"
#include "TextPainter.h"

typedef enum {
	EPD_2_7          // 264 x 176
} EPD_size;

typedef enum {           // Image pixel -> Display pixel
	EPD_inverse,     // B -> W, W -> B (New Image)
	EPD_normal       // B -> B, W -> W (New Image)
} EPD_stage;

typedef enum {           // error codes
	EPD_OK,
	EPD_UNSUPPORTED_COG,
	EPD_PANEL_BROKEN,
	EPD_DC_FAILED
} EPD_error;

// values for border byte
const uint8_t EPD_BORDER_BYTE_BLACK = 0xff;
const uint8_t EPD_BORDER_BYTE_WHITE = 0xaa;
const uint8_t EPD_BORDER_BYTE_NULL  = 0x00;

typedef void EPD_reader(void *buffer, uint32_t address, uint16_t length);

class EPD_Class {
public:
	uint8_t EPD_Pin_PANEL_ON;
    uint8_t EPD_Pin_BORDER;
    uint8_t EPD_Pin_DISCHARGE;
    uint8_t EPD_Pin_RESET;
    uint8_t EPD_Pin_BUSY;
    uint8_t EPD_Pin_EPD_CS;

	EPD_size size;
	uint16_t lines_per_display;
	uint16_t dots_per_line;
	uint16_t bytes_per_line;
	uint16_t bytes_per_scan;

	uint8_t voltage_level;

	EPD_error status;

    const uint8_t *channel_select;
	uint16_t channel_select_length;

	typedef struct {
		uint16_t stage1_repeat;
		uint16_t stage1_step;
		uint16_t stage1_block;
		uint16_t stage2_repeat;
		uint16_t stage2_t1;
		uint16_t stage2_t2;
		uint16_t stage3_repeat;
		uint16_t stage3_step;
		uint16_t stage3_block;
	} compensation_type;

	const compensation_type *compensation;
	uint16_t temperature_offset;

	void nothing_frame();
	void dummy_line();
	void border_dummy_line();

public:
	// power up and power down the EPD panel

    // Init
    EPD_Class(const EPD_Class &f);  // prevent copy

    EPD_Class(EPD_size size,
              uint8_t panel_on_pin,
              uint8_t border_pin,
              uint8_t discharge_pin,
              uint8_t reset_pin,
              uint8_t busy_pin,
              uint8_t chip_select_pin);

	const bool operator!() const {
		return EPD_OK != this->status;
	}

	EPD_error error() const {
		return this->status;
	}

	// clear display (anything -> white)
	void clear() {
		this->frame_fixed_13(0xff, EPD_inverse);
		this->frame_stage2();
		this->frame_fixed_13(0xaa, EPD_normal);
	}

    void setFactor(int temperature = 25);


    // Quick Functions

    void quick_frame_data_sprite(SpritePainter *SpritePainter, TextPainter *TxtPainter);

    void quick_frame_stage2();


    void quick_line(uint16_t line_start, uint16_t line_finish, const uint8_t *data, uint8_t fixed_value,
                    bool read_progmem, EPD_stage stage, uint8_t border_byte, bool set_voltage_limit);


    // single frame refresh
	void frame_fixed_timed(uint8_t fixed_value, long stage_time);

	// the B/W toggle stage
	void frame_stage2();

	// stages 1/3 functions
	void frame_fixed_13(uint8_t fixed_value, EPD_stage stage);
	void frame_data_13(const uint8_t *image_data, EPD_stage stage, bool read_progmem = true);
	void frame_cb_13(uint32_t address, EPD_reader *reader, EPD_stage stage);

	// single line display - very low-level
	// also has to handle AVR progmem
	void line(uint16_t line, const uint8_t *data, uint8_t fixed_value,
		  bool read_progmem, EPD_stage stage = EPD_normal, uint8_t border_byte = EPD_BORDER_BYTE_NULL, bool set_voltage_limit = false);



    void quick_frame_fixed_timed(uint8_t fixed_value, long stage_time);

    long paint_line_data(const uint8_t *line_data, uint8_t start_line, uint8_t finish_line, EPD_stage stage, long stage_time);

    void paint_display(SpritePainter *spritePainter, TextPainter *textPainter);
};

#endif
