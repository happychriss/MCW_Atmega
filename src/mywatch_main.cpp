// -*- mode: c++ -*-
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

//#define MYDEBUG

#ifdef MYDEBUG
#define MYDEBUG_CORE
#define DP(x)     Serial.print (x)
#define DPD(x)     Serial.print (x, DEC)
#define DPL(x)  Serial.println (x)
#else
#define DP(x)
#define DPD(x)
#define DPL(x)
#endif

#include <inttypes.h>

#include <SPI.h>
#include <EPD2.h>
#include "EPD_BASE.h"
#include <Wire.h>
#include <LM75.h>
#include <DS3232RTC.h>
#include <Time.h>
#include "LowPower.h"


// #include </libraries/DigitalClockBuilder/DigitalClockBuilder.h>
#include "TextClockBuilder.h"
#include "DigitalClockBuilder.h"

#define EPD_SIZE EPD_2_7

// pre-processor convert to string
#define MAKE_STRING1(X) #X
#define MAKE_STRING(X) MAKE_STRING1(X)

// Non Standard PINs used
const uint8_t Pin_Power_EPD = A3; //set to low to disconnect EPD panel from VCC
const uint8_t Pin_INTERRPUPT_0 = 2; // wake up for Clock and ....
const uint8_t Pin_Switch = A0;

// Global Variables
static int state = 0; //global state varaible
boolean alarm_minute, alarm_clock; // set by interrupt routine for RTC alarm


// define the E-Ink display
EPD_Class EPD(EPD_SIZE, Pin_PANEL_ON, Pin_BORDER, Pin_DISCHARGE, Pin_RESET, Pin_BUSY, Pin_EPD_CS);
DigitalClockBuilder MainDigitClock(30, 6, 22);
TextClockBuilder MainTextClock;
SpritePainter MainClockPainter;

int temperature;


TextPainter WelcomeTxtPainter;

TxtLine MyText[3];
char line_1[20];
char line_2[20];
char line_3[20];
tmElements_t tm;

#define INT_PROCESSED 0xff

#define INT_BUFFER 3 //adjust buffer

volatile uint32_t interrupt_list[INT_BUFFER] = {INT_PROCESSED,INT_PROCESSED,INT_PROCESSED};
volatile uint8_t  new_int_count=0;
volatile uint8_t  processed_int_count=0;

volatile uint8_t  current_int_count=0;



void PaintTextClock (uint8_t d1, uint8_t d2, uint8_t d3, uint8_t d4);
void RTC_wakeup(const tmElements_t &tm, uint8_t wake_min);
boolean CheckSwitchLong();
void ReadTimeFromUser(tmElements_t &tm);


// Interrupt Routine



#define INT_WAIT_LIGHT_ON 10  //smaller than 10
#define INT_WAIT_SWITCH 40
#define INT_WAIT_LIGHT_OFF 75
#define INT_WAIT_CLOCK 110


#define STATE_INIT 0
#define STATE_SLEEP 1
#define STATE_CHECK_INTERRUPT 2
#define STATE_FIVE_MIN_TIME 4
#define STATE_ONE_MIN_TXT 5
#define STATE_LIGHT_ON 6
#define STATE_LIGHT_OFF 7
#define STATE_ALARM_CLOCK 8
#define STATE_SET_TIME 9
#define STATE_PAINT_TIME 9


void increment() {

    if (state!=STATE_SET_TIME) {

        uint32_t tmp_count = 0;

        // if tmp_count == INT_WAIT_COUNT, no interrupt was send from the PIC16, therefore interrupt was from the AlarmClock
        while (digitalRead(Pin_INTERRPUPT_0) == 0 && tmp_count != INT_WAIT_CLOCK) { tmp_count++; };

        interrupt_list[new_int_count] = tmp_count;
        new_int_count = (new_int_count + 1) % INT_BUFFER;
    }
}


// I/O setup
void setup() {

    Serial.begin(9600);
    DPL();
    DPL();
    DPL("Init v2");

    pinMode(Pin_INTERRPUPT_0, INPUT_PULLUP);
    pinMode(Pin_Switch, INPUT);

//Init for Clock  *************************************

    tmElements_t tm;

    RTC.set32kHzOutput(false); //http://forums.adafruit.com/viewtopic.php?f=31&t=45933
    RTC.squareWave(SQWAVE_NONE);
    RTC.read(tm);
    DP("Time:");
    DP(tm.Hour);
    DP(":");
    DPL(tm.Minute);


// Init Panel

    digitalWrite(Pin_Power_EPD, HIGH);



    EPD_BASE::init();

    EPD_BASE::begin(&EPD); // power up the EPD panel
    if (!EPD) {
        DP("EPD error = ");
        DP(EPD.error());
        DPL("");
        return;
    }

    DPL("Display: " MAKE_STRING(EPD_SIZE));

    Wire.begin();
    LM75.begin();
    temperature = LM75.read();
    DP("Temp = ");
    DP(temperature);
    DPL(" C");
    EPD.setFactor(temperature); // adjust for current temperature

    DP("Check PROGMEM:");
    uint8_t *data =(uint8_t*) pgm_read_word(&chrtbl_f32['!'-32]);
    DP("Width:");DPL(pgm_read_byte(&data[0]));

    state=STATE_INIT;

}


// main loop
void loop() {

    switch (state) {
        default:
            DP("!!! UNKOWN CODE:");DP(state);


// INIT *******************************************************************
        case STATE_INIT: {

            DPL("State: Init");

            attachInterrupt(0, increment, FALLING);

            state = STATE_FIVE_MIN_TIME;

            break;
        }

// Wakeup *****************************************************************
        case STATE_CHECK_INTERRUPT: {

            uint32_t current_interrupt;

            DPL("State: Check Interrupt");


            // Check for interrupts

            state = STATE_SLEEP;

            for(uint8_t count = 0; count <INT_BUFFER; ++count)
            {
                uint8_t tmp_int_count;
                tmp_int_count=(current_int_count+count) % INT_BUFFER;
                DP(tmp_int_count);DP("-");DPL(interrupt_list[tmp_int_count]);
                if (interrupt_list[tmp_int_count]!=INT_PROCESSED) {
                    current_interrupt=interrupt_list[tmp_int_count];
                    state = STATE_CHECK_INTERRUPT;
                }
            }

            if (state==STATE_SLEEP) {
                break;
            }

            DP("*** Interrupt Processing***:  ");DP(current_int_count);DP("-");DPL(current_interrupt);

            // clean processed interrupt
            interrupt_list[current_int_count]=INT_PROCESSED;
            current_int_count= ( current_int_count+ 1) % INT_BUFFER;

//
//#define INT_WAIT_LIGHT_ON 10  //smaller than 10
//#define INT_WAIT_SWITCH 30
//#define INT_WAIT_LIGHT_OFF 80
//#define INT_WAIT_CLOCK 110
//

            if (current_interrupt < INT_WAIT_LIGHT_ON) {
                state = STATE_LIGHT_ON;
                break;
            }


            if (current_interrupt < INT_WAIT_SWITCH) { //e.g. 40
                DPL("Switch pressed");
                if (CheckSwitchLong() == true) {
                    state = STATE_SET_TIME;
                } else {
                    state = STATE_LIGHT_ON;
                }
                break;

            }

            if (current_interrupt < INT_WAIT_LIGHT_OFF) { //e.g. 75
                state = STATE_LIGHT_OFF;
                break;
            }


            if (current_interrupt == INT_WAIT_CLOCK) {

                alarm_minute = RTC.alarm(ALARM_2);
                alarm_clock = RTC.alarm(ALARM_1);

//              I am only using ALARM2 to trigger wakekup

//                if (alarm_clock) {
//                    state = STATE_ALARM_CLOCK;
//                }
//
//                if (alarm_minute) {
//                    state = STATE_FIVE_MIN_TIME;
//                }

                state = STATE_FIVE_MIN_TIME;
                break;

            }

            break;
        }


        case STATE_SLEEP: {



            DPL("Sleep...");
            delay(250);
            LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF);

            // no I am really deep sleeping


            state = STATE_CHECK_INTERRUPT;

            break;
        }


// Set time **************************************************************
        case STATE_SET_TIME: {

            DP("State SetUpMode");

            tmElements_t tm;
            ReadTimeFromUser(tm);
            RTC.write(tm);

            state = STATE_LIGHT_ON;

            break;
        }


// Sleep for 5min , write text time **************************************
        case STATE_FIVE_MIN_TIME: {

            DPL("State: 5min Wakeup");

            tmElements_t tm;
            RTC.read(tm);


            RTC_wakeup(tm, 5);

            // Paint Time
            memset(line_1, 0, sizeof(line_1));
            memset(line_2, 0, sizeof(line_2));
            memset(line_3, 0, sizeof(line_3));

            MainTextClock.BuildText(tm.Hour, tm.Minute, line_1, line_2, line_3);

            MyText[0].x = 1;MyText[0].y = 10; MyText[0].size = 1; MyText[0].txt = line_1;
            MyText[1].x = 1;MyText[1].y = 60; MyText[1].size = 1; MyText[1].txt = line_2;
            MyText[2].x = 1;MyText[2].y = 110;MyText[2].size = 1; MyText[2].txt = line_3;

            WelcomeTxtPainter.BuildText(MyText, 3);

            digitalWrite(Pin_Power_EPD, HIGH);delay(200);
            EPD.paint_display(NULL,&WelcomeTxtPainter);
            digitalWrite(Pin_Power_EPD, LOW);delay(200);

            state = STATE_CHECK_INTERRUPT;
            break;
        }

// Light is off, sleep ***************************************************
        case STATE_LIGHT_OFF: {

            DPL("State: Ligth off");

            RTC.alarm(ALARM_1); RTC.alarm(ALARM_2);//Reset Al
            RTC.alarmInterrupt(ALARM_2, 0);//disable Alarm..

            memset(line_1, 0, sizeof(line_1));
            MyText[0].x = 1;MyText[0].y = 10; MyText[0].size = 1; MyText[0].txt = "Tzz....";
            WelcomeTxtPainter.BuildText(MyText, 1);

            digitalWrite(Pin_Power_EPD, HIGH);delay(200);
            EPD.paint_display(NULL,&WelcomeTxtPainter);
            digitalWrite(Pin_Power_EPD, LOW);delay(200);

            state = STATE_CHECK_INTERRUPT;
            break;

        }

// Light is on , update time ***************************************************
        case STATE_LIGHT_ON: {
            DPL("State: Ligth on");

            tmElements_t tm;
            RTC.read(tm);

            PaintTextClock (tm.Hour / 10, tm.Hour % 10, tm.Minute / 10, tm.Minute % 10);

            RTC_wakeup(tm, 1);

            state = STATE_CHECK_INTERRUPT;
            break;
        }

    }

}

//boolean AskUser(char *question) {
//    memset(line_1, 0, sizeof(line_1));
//    MyText[0].x = 1;
//    MyText[0].y = 10;
//    MyText[0].size = 1;
//    MyText[0].txt = question;
//    WelcomeTxtPainter.BuildText(MyText, 1);
//
//    digitalWrite(Pin_Power_EPD, HIGH);
//    delay(200);
//    EPD.paint_display(NULL,&WelcomeTxtPainter);
//    digitalWrite(Pin_Power_EPD, LOW);
//    delay(200);
//
//    while (digitalRead(Pin_Switch) == 0) { delay(50); };
//    if (CheckSwitchLong() == true) return true;
//    return false;
//
//}

void ReadTimeFromUser(tmElements_t &tm) {
    uint8_t set_time_digit=0;
    uint8_t max_digit_count[4];

    uint8_t digit[4]; //time read from user feedback
    digit[0] = 8;
    digit[1] = 8;
    digit[2] = 8;
    digit[3] = 8;
    max_digit_count[0]=2;
    max_digit_count[1]=9;
    max_digit_count[2]=5;
    max_digit_count[3]=9;


    // loop though all digits of the watch
    while (set_time_digit < 4) {

                // count the numbers
                uint8_t count = 0;
                while (1) {

                    digit[set_time_digit] = count;
                    PaintTextClock (digit[0], digit[1], digit[2], digit[3]);

                    while (digitalRead(Pin_Switch) == 0) { delay(50); };
                    if (CheckSwitchLong() == true) break;

                    // keep numbers in range for a clock
                    count++;
                    if (count>max_digit_count[set_time_digit]) count=0;
                }

                set_time_digit++;

            }

    tm.Hour=digit[0]*10+digit[1];
    tm.Minute=digit[2]*10+digit[3];

}

void RTC_wakeup(const tmElements_t &tm, uint8_t wake_up_interval) {// Start New alarm *******************************

    RTC.alarm(ALARM_1);
    RTC.alarm(ALARM_2);//Reset Al
    RTC.alarmInterrupt(ALARM_2, 1);//alarm 2 interrupt A2IE , wakeup every minute

    uint8_t next_wake_up = (((tm.Minute + wake_up_interval) / wake_up_interval) * wake_up_interval);
    if (next_wake_up >= 60) next_wake_up = 0;
    DP("Next Wakeup: ");
    DPL(next_wake_up);
    RTC.setAlarm(ALM2_MATCH_MINUTES, 0, next_wake_up, 0, 0);
}


// return true, if button was pressed longer than delay
boolean CheckSwitchLong() {

        uint8_t switch_count=0;

        while(switch_count<15) {
            delay(100);
            if (digitalRead(Pin_Switch)==0) {
                return false;
            }
            switch_count++;
        };

    return true;
};


void PaintTextClock (uint8_t d1, uint8_t d2, uint8_t d3, uint8_t d4) {
    MainClockPainter = MainDigitClock.BuildClock(d1,d2,d3,d4);
    digitalWrite(Pin_Power_EPD, HIGH);
    delay(200);
    EPD.paint_display(&MainClockPainter, NULL);
    digitalWrite(Pin_Power_EPD, LOW);
    delay(200);
}


