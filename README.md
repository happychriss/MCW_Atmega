# MCW - The Most Complicate Watch


### "Why going easy - if it can be more difficult???"

MyWatch is an for fun project to build an alarm clock based on an e-Ink display.

It using a power saving Arduino pro mini on 3V.

The e-Ink display is 2.7 inch E-paper Display from Embedded Artist.

Its controlled by the very precise clock DS3232RTC.

A PIC16 MCU processor is checking for light changes, sleeping time and waking-up the Arduino 
to calculate the time and update the E-paper Display.

!!! e-Ink displays are using a lot of power when updated !!!

Note:
PCB can be found in folder: pcb-layout of this github
Code for the PIC can be found here: https://github.com/happychriss/MCW_PIC16MCU

## Features
* Time is displayed as text in German, e.g. "Es ist Viertel vor Acht" and updates every 5min
* Using a PIC16(L)F1824 to control sleep mode, light-on / light-off and the Arduino
* When touching the capacitive sensor, time is immediately updated to HH:MM format
* When it is dark outside (so nobody can read the time anyway) the clock goes into very deep sleep mode, woken up 
  by the PIC. 
* Low memory of the Pro Mini requires a new self developed memory saving graphics library
* Character bitmaps are RLE encoded and stored in flash memory to save RAM
* Pro Mini is set to deep sleep mode and wake up by DS3232RTC
* Special circuit disconnects the e-Ink display from power during sleep mode
* Time can be set-up by long-pressing the capacitive button

## ToDo
* Build in Alarm Clock
* Build a nice case

## Some pictures
You find more information here:
https://44-2.de/mcw-most-complicated-watch/