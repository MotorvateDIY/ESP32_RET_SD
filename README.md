ESP32_RET_SD
=======
ESP32 Reverse Engineering Tool with SD CAN bus datalogger for SavvyCAN
Based on Collin Kidder (https://github.com/collin80) fantastic A0RET (https://github.com/collin80/A0RET)
SD logging added by frank @ motorvateDIY Jan 1/2024

 Main Goal: Easy to use CAN bus datalogging
 If an SD card *IS* detected at boot up, all CAN bus data is recorded to the SD card, in SavvyCAN CSV format for offline processing
 If an SD card *IS NOT* detected, a WIFI access point is created for a SavvyCAN connection to receive and/or send CAN frames

Compatable with Arduino core 2.0.x and 3.0.x 

With a focus of easy to use offline mode (CAN to SD) and SavvyCAN mode (CAN to Wifi), the following changes have been made:
 Removed: ELM327 compatability
 Removed: LAWICEL
 Removed: Bluetooth
 Removed: Digital and Analog I/O
 Added: Saving all CAN data to SD card in SavvyCAN CSV format for offline processing/reverse engineering
 
 Now compiles using default 1.2MB APP partition 

Instructions for use:
=======
 in config.h:
 set SD card pins on lines 43-46
 set CAN Tx/Rx pins on lines 50-51
 set SSID and password on lines 54-55


=======
Previous A0RET Readme
=======

A0RET
=======

Reverse Engineering Tool running on ESP32 based hardware. Originally meant for Macchina A0 but also
has support for EVTV ESP32 Board (which was originally supported by ESP32RET). 

A fork of the ESP32RET project, itself a fork of the M2RET project, 
itself a fork of the GVRET project.

#### Requirements:

You will need the following to be able to compile the run this project:

- [Arduino IDE](https://www.arduino.cc/en/Main/Software) Tested on 1.8.13
- [Arduino-ESP32](https://github.com/espressif/arduino-esp32) - Allows for programming the ESP32 with the Arduino IDE
- [esp32_can](https://github.com/collin80/esp32_can) - A unified CAN library that supports the built-in CAN plus MCP2515 and MCP2517FD

PLEASE NOTE: The Macchina A0 uses a WRover ESP32 module which includes PSRAM. But, do NOT use the WRover
board in the Arduino IDE nor try to enable PSRAM. Doing so causes a fatal crash bug.

The EVTV board has no PSRAM anyway.

All libraries belong in %USERPROFILE%\Documents\Arduino\hardware\esp32\libraries (Windows) or ~/Arduino/hardware/esp32/libraries (Linux/Mac).

The canbus is supposed to be terminated on both ends of the bus. This should not be a problem as this firmware will be used to reverse engineer existing buses. However, do note that CAN buses should have a resistance from CAN_H to CAN_L of 60 ohms. This is affected by placing a 120 ohm resistor on both sides of the bus. If the bus resistance is not fairly close to 60 ohms then you may run into trouble.  

#### The firmware is a work in progress. What works:
- CAN0 reading and writing
- Preferences are saved and loaded
- Text console is active (configuration and CAN capture display)
- Can connect as a GVRET device with SavvyCAN
- LAWICEL support (somewhat tested. Still experimental)
- Bluetooth works to create an ELM327 compatible interface (tested with Torque app)

#### What does not work:
- Digital and Analog I/O

#### License:

This software is MIT licensed:

Copyright (c) 2014-2020 Collin Kidder, Michael Neuweiler

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

