//
// Created by development on 8/9/15.
//

#include "EPD_BASE.h"
#include "EPD_SPI.h"


void EPD_BASE::init() {

// Init for Panel *************************************

    pinMode(Pin_RED_LED, OUTPUT);
    pinMode(Pin_SW2, INPUT);
    pinMode(Pin_BUSY, INPUT);
    pinMode(Pin_RESET, OUTPUT);
    pinMode(Pin_PANEL_ON, OUTPUT);
    pinMode(Pin_DISCHARGE, OUTPUT);
    pinMode(Pin_BORDER, OUTPUT);
    pinMode(Pin_EPD_CS, OUTPUT);
    pinMode(Pin_FLASH_CS, OUTPUT);

    digitalWrite(Pin_RED_LED, LOW);
    digitalWrite(Pin_RESET, LOW);
    digitalWrite(Pin_PANEL_ON, LOW);
    digitalWrite(Pin_DISCHARGE, LOW);
    digitalWrite(Pin_BORDER, LOW);
    digitalWrite(Pin_EPD_CS, LOW);
    digitalWrite(Pin_FLASH_CS, HIGH);
}

void EPD_BASE::power_off(EPD_Class *epd) {

    // turn of power and all signals
    digitalWrite(epd->EPD_Pin_RESET, LOW);
    digitalWrite(epd->EPD_Pin_PANEL_ON, LOW);
    digitalWrite(epd->EPD_Pin_BORDER, LOW);

    // ensure SPI MOSI and CLOCK are Low before CS Low
    EPD_SPI::SPI_off();
    digitalWrite(epd->EPD_Pin_EPD_CS, LOW);

    // pulse discharge pin
    digitalWrite(epd->EPD_Pin_DISCHARGE, HIGH);
    Delay_ms(150);
    digitalWrite(epd->EPD_Pin_DISCHARGE, LOW);

}


void EPD_BASE::begin(EPD_Class *epd) {

    // assume ok
    epd->status = EPD_OK;


    // power up sequence
    digitalWrite(epd->EPD_Pin_RESET, LOW);
    digitalWrite(epd->EPD_Pin_PANEL_ON, LOW);
    digitalWrite(epd->EPD_Pin_DISCHARGE, LOW);
    digitalWrite(epd->EPD_Pin_BORDER, LOW);
    digitalWrite(epd->EPD_Pin_EPD_CS, LOW);

    EPD_SPI::SPI_on();

    Delay_ms(5);
    digitalWrite(epd->EPD_Pin_PANEL_ON, HIGH);
    Delay_ms(10);

    digitalWrite(epd->EPD_Pin_RESET, HIGH);
    digitalWrite(epd->EPD_Pin_BORDER, HIGH);
    digitalWrite(epd->EPD_Pin_EPD_CS, HIGH);
    Delay_ms(5);

    digitalWrite(epd->EPD_Pin_RESET, LOW);
    Delay_ms(5);

    digitalWrite(epd->EPD_Pin_RESET, HIGH);
    Delay_ms(5);

    // wait for COG to become ready
    while (HIGH == digitalRead(epd->EPD_Pin_BUSY)) {
        Delay_us(10);
    }

    // read the COG ID
    int cog_id = EPD_SPI::SPI_read(epd->EPD_Pin_EPD_CS, CU8(0x71, 0x00), 2);
    cog_id = EPD_SPI::SPI_read(epd->EPD_Pin_EPD_CS, CU8(0x71, 0x00), 2);

    if (0x02 != (0x0f & cog_id)) {
        epd->status = EPD_UNSUPPORTED_COG;
        EPD_BASE::power_off(epd);
        return;
    }

    // Disable OE
    EPD_SPI::SPI_send(epd->EPD_Pin_EPD_CS, CU8(0x70, 0x02), 2);
    EPD_SPI::SPI_send(epd->EPD_Pin_EPD_CS, CU8(0x72, 0x40), 2);

    // check breakage
    EPD_SPI::SPI_send(epd->EPD_Pin_EPD_CS, CU8(0x70, 0x0f), 2);
    int broken_panel = EPD_SPI::SPI_read(epd->EPD_Pin_EPD_CS, CU8(0x73, 0x00), 2);
    if (0x00 == (0x80 & broken_panel)) {
        epd->status = EPD_PANEL_BROKEN;
        EPD_BASE::power_off(epd);
        return;
    }

    // power saving mode
    EPD_SPI::SPI_send(epd->EPD_Pin_EPD_CS, CU8(0x70, 0x0b), 2);
    EPD_SPI::SPI_send(epd->EPD_Pin_EPD_CS, CU8(0x72, 0x02), 2);

    // channel select
    EPD_SPI::SPI_send(epd->EPD_Pin_EPD_CS, CU8(0x70, 0x01), 2);
    EPD_SPI::SPI_send(epd->EPD_Pin_EPD_CS, epd->channel_select, epd->channel_select_length);

    // high power mode osc
    EPD_SPI::SPI_send(epd->EPD_Pin_EPD_CS, CU8(0x70, 0x07), 2);
    EPD_SPI::SPI_send(epd->EPD_Pin_EPD_CS, CU8(0x72, 0xd1), 2);

    // power setting
    EPD_SPI::SPI_send(epd->EPD_Pin_EPD_CS, CU8(0x70, 0x08), 2);
    EPD_SPI::SPI_send(epd->EPD_Pin_EPD_CS, CU8(0x72, 0x02), 2);

    // Vcom level
    EPD_SPI::SPI_send(epd->EPD_Pin_EPD_CS, CU8(0x70, 0x09), 2);
    EPD_SPI::SPI_send(epd->EPD_Pin_EPD_CS, CU8(0x72, 0xc2), 2);

    // power setting
    EPD_SPI::SPI_send(epd->EPD_Pin_EPD_CS, CU8(0x70, 0x04), 2);
    EPD_SPI::SPI_send(epd->EPD_Pin_EPD_CS, CU8(0x72, 0x03), 2);

    // driver latch on
    EPD_SPI::SPI_send(epd->EPD_Pin_EPD_CS, CU8(0x70, 0x03), 2);
    EPD_SPI::SPI_send(epd->EPD_Pin_EPD_CS, CU8(0x72, 0x01), 2);

    // driver latch off
    EPD_SPI::SPI_send(epd->EPD_Pin_EPD_CS, CU8(0x70, 0x03), 2);
    EPD_SPI::SPI_send(epd->EPD_Pin_EPD_CS, CU8(0x72, 0x00), 2);

    Delay_ms(5);

    bool dc_ok = false;

    for (int i = 0; i < 4; ++i) {
        // charge pump positive voltage on - VGH/VDL on
        EPD_SPI::SPI_send(epd->EPD_Pin_EPD_CS, CU8(0x70, 0x05), 2);
        EPD_SPI::SPI_send(epd->EPD_Pin_EPD_CS, CU8(0x72, 0x01), 2);

        Delay_ms(240);

        // charge pump negative voltage on - VGL/VDL on
        EPD_SPI::SPI_send(epd->EPD_Pin_EPD_CS, CU8(0x70, 0x05), 2);
        EPD_SPI::SPI_send(epd->EPD_Pin_EPD_CS, CU8(0x72, 0x03), 2);

        Delay_ms(40);

        // charge pump Vcom on - Vcom driver on
        EPD_SPI::SPI_send(epd->EPD_Pin_EPD_CS, CU8(0x70, 0x05), 2);
        EPD_SPI::SPI_send(epd->EPD_Pin_EPD_CS, CU8(0x72, 0x0f), 2);

        Delay_ms(40);

        // check DC/DC
        EPD_SPI::SPI_send(epd->EPD_Pin_EPD_CS, CU8(0x70, 0x0f), 2);
        int dc_state = EPD_SPI::SPI_read(epd->EPD_Pin_EPD_CS, CU8(0x73, 0x00), 2);
        if (0x40 == (0x40 & dc_state)) {
            dc_ok = true;
            break;
        }
    }
    if (!dc_ok) {
        // output enable to disable
        EPD_SPI::SPI_send(epd->EPD_Pin_EPD_CS, CU8(0x70, 0x02), 2);
        EPD_SPI::SPI_send(epd->EPD_Pin_EPD_CS, CU8(0x72, 0x40), 2);

        epd->status = EPD_DC_FAILED;
        EPD_BASE::power_off(epd);
        return;
    }

    EPD_SPI::SPI_off();
}


void EPD_BASE::end(EPD_Class *epd) {

    epd->nothing_frame();
    epd->dummy_line();


    // only pulse bord.er pin for 2.70" EPD
    digitalWrite(epd->EPD_Pin_BORDER, LOW);
    Delay_ms(200);
    digitalWrite(epd->EPD_Pin_BORDER, HIGH);

    EPD_SPI::SPI_on();

    // check DC/DC
    EPD_SPI::SPI_send(epd->EPD_Pin_EPD_CS, CU8(0x70, 0x0f), 2);
    int dc_state = EPD_SPI::SPI_read(epd->EPD_Pin_EPD_CS, CU8(0x73, 0x00), 2);
    if (0x40 != (0x40 & dc_state)) {
        epd->status = EPD_DC_FAILED;
        EPD_BASE::power_off(epd);
        return;
    }

    // latch reset turn on
    EPD_SPI::SPI_send(epd->EPD_Pin_EPD_CS, CU8(0x70, 0x03), 2);
    EPD_SPI::SPI_send(epd->EPD_Pin_EPD_CS, CU8(0x72, 0x01), 2);

    // output enable off
    EPD_SPI::SPI_send(epd->EPD_Pin_EPD_CS, CU8(0x70, 0x02), 2);
    EPD_SPI::SPI_send(epd->EPD_Pin_EPD_CS, CU8(0x72, 0x05), 2);

    // power off charge pump Vcom
    EPD_SPI::SPI_send(epd->EPD_Pin_EPD_CS, CU8(0x70, 0x05), 2);
    EPD_SPI::SPI_send(epd->EPD_Pin_EPD_CS, CU8(0x72, 0x03), 2);

    // power off charge pump neg voltage
    EPD_SPI::SPI_send(epd->EPD_Pin_EPD_CS, CU8(0x70, 0x05), 2);
    EPD_SPI::SPI_send(epd->EPD_Pin_EPD_CS, CU8(0x72, 0x01), 2);

    Delay_ms(240);

    // power off all charge pumps
    EPD_SPI::SPI_send(epd->EPD_Pin_EPD_CS, CU8(0x70, 0x05), 2);
    EPD_SPI::SPI_send(epd->EPD_Pin_EPD_CS, CU8(0x72, 0x00), 2);

    // turn of osc
    EPD_SPI::SPI_send(epd->EPD_Pin_EPD_CS, CU8(0x70, 0x07), 2);
    EPD_SPI::SPI_send(epd->EPD_Pin_EPD_CS, CU8(0x72, 0x01), 2);

    // discharge internal on
    EPD_SPI::SPI_send(epd->EPD_Pin_EPD_CS, CU8(0x70, 0x04), 2);
    EPD_SPI::SPI_send(epd->EPD_Pin_EPD_CS, CU8(0x72, 0x83), 2);

    Delay_ms(30);

    // discharge internal off
    //SPI_send(this->EPD_Pin_EPD_CS, CU8(0x70, 0x04), 2);
    //SPI_send(this->EPD_Pin_EPD_CS, CU8(0x72, 0x00), 2);

    power_off(epd);

}


