//
// Created by development on 8/9/15.
//

#ifndef MYWATCH_EPD_LOWLEVEL_H
#define MYWATCH_EPD_LOWLEVEL_H

#include <Arduino.h>
#include <SPI.h>
#include <avr/pgmspace.h>
#include <EPD2.h>

// delays - more consistent naming
#define Delay_ms(ms) delay(ms)
#define Delay_us(us) delayMicroseconds(us)
#define CU8(...) (ARRAY(const uint8_t, __VA_ARGS__))
#define ARRAY(type, ...) ((type[]){__VA_ARGS__})

class EPD_SPI {

public:

    static void SPI_on();

    static void SPI_off();

    static void SPI_put(uint8_t c);

    static void SPI_send(uint8_t cs_pin, const uint8_t *buffer, uint16_t length);

    static uint8_t SPI_read(uint8_t cs_pin, const uint8_t *buffer, uint16_t length);
};


#endif //MYWATCH_EPD_SPI_H