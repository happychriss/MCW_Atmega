//
// Created by development on 8/9/15.
//

#include <EPD2.h>
#include "EPD_SPI.h"


    void EPD_SPI::SPI_on() {
        SPI.end();
        SPI.begin();
        SPI.setBitOrder(MSBFIRST);
        SPI.setDataMode(SPI_MODE0);
        SPI.setClockDivider(SPI_CLOCK_DIV2);
        SPI_put(0x00);
        SPI_put(0x00);
        Delay_us(10);
    }


    void EPD_SPI::SPI_off() {
        // SPI.begin();
        // SPI.setBitOrder(MSBFIRST);
        SPI.setDataMode(SPI_MODE0);
        // SPI.setClockDivider(SPI_CLOCK_DIV2);
        SPI_put(0x00);
        SPI_put(0x00);
        Delay_us(10);
        SPI.end();
    }


    void EPD_SPI::SPI_put(uint8_t c) {
        SPI.transfer(c);
    }


    void EPD_SPI::SPI_send(uint8_t cs_pin, const uint8_t *buffer, uint16_t length) {
        // CS low
        digitalWrite(cs_pin, LOW);

        // send all data
        for (uint16_t i = 0; i < length; ++i) {
            SPI_put(*buffer++);
        }

        // CS high
        digitalWrite(cs_pin, HIGH);
    }

    uint8_t EPD_SPI::SPI_read(uint8_t cs_pin, const uint8_t *buffer, uint16_t length) {
        // CS low
        digitalWrite(cs_pin, LOW);

        uint8_t rbuffer[4];
        uint8_t result = 0;

        // send all data
        for (uint16_t i = 0; i < length; ++i) {
            result = SPI.transfer(*buffer++);
            if (i < 4) {
                rbuffer[i] = result;
            }
        }

        // CS high
        digitalWrite(cs_pin, HIGH);
        return result;
    }
