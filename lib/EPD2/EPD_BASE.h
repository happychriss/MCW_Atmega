//
// Created by development on 8/9/15.
//

#ifndef MYWATCH_EPD_POWER_H
#define MYWATCH_EPD_POWER_H

#include "EPD2.h"


// Arduino IO layout
//const int Pin_TEMPERATURE = A0; // Temperature is handled by LM75 over I2C and not an analog pin
const uint8_t Pin_PANEL_ON = 5;
const uint8_t Pin_BORDER = 10;
const uint8_t Pin_DISCHARGE = 4;
//const int Pin_PWM = 5;    // Not used by COG v2
const uint8_t Pin_RESET = 6;
const uint8_t Pin_BUSY = 7;
const uint8_t Pin_EPD_CS = 8;
const uint8_t Pin_FLASH_CS = 9;
const uint8_t Pin_SW2 = 12;
const uint8_t Pin_RED_LED = 13;




class EPD_BASE {


public:
    // power up and power down the EPD panel

    static void init();

    static void power_off(EPD_Class *epd);

    static void begin(EPD_Class *epd);

    static void end(EPD_Class *epd);
};


#endif //MYWATCH_EPD_POWER_H
