ESP32_RET_SD
=======
ESP32 Reverse Engineering Tool with stand alone SD CAN bus recorder and wifi support for SavvyCAN.
 Based on [Collin Kidder](https://github.com/collin80) fantastic [A0RET](https://github.com/collin80/A0RET).

 ### Main Goal: Easy to use CAN Bus Recording to SD Card or with [SavvyCAN](https://github.com/collin80/SavvyCAN)
 - If an SD card  *IS*  detected at power up, all CAN bus data is recorded to the SD card, in SavvyCAN, CSV/GVRET format 
 - If an SD card  *IS NOT*  detected, a WIFI access point is created for a SavvyCAN connection to receive and/or send CAN frames


With a focus of ease of use the following changes have been made from [A0RET](https://github.com/collin80/A0RET)
 - Removed: ELM327 compatability
 - Removed: LAWICEL
 - Removed: Bluetooth
 - Removed: Digital and Analog I/O
 - Added: Saving all CAN data to SD card in SavvyCAN CSV format
 
 Now compiles using default 1.2MB APP partition 

#### Requirements:
You will need the following to be able to compile the run this project:

- [Arduino IDE](https://www.arduino.cc/en/software) Tested on IDE v2.3.4
- [Arduino-ESP32 Support](https://docs.espressif.com/projects/arduino-esp32/en/latest/getting_started.html) - Allows for programming the ESP32 with the Arduino IDE (Tested on Arduino core 2.0.17 and 3.0.7)
- [esp32_can](https://github.com/MotorvateDIY/esp32_can) - A unified CAN library that supports the built-in CAN, updated for accurate SD card CAN time stamps
- [can_common](https://github.com/collin80/can_common) - Used by esp32_can
- To record to a SD card, it must be 32GB or less and formatted in FAT32
- At a minimum, an ESP32, SD module, and OBD male connector. See parts list and wiring diagram below.


Instructions for use:
=======
 in config.h set:
 - SD card pins on lines 43-46 (default: Standard VSPI where SD_SCK 18, SD_MISO 19, SD_MOSI 23, SD_CS 5)
 - CAN Tx/Rx pins on lines 50-51 (defaut: GPIO_CAN_TX 17, GPIO_CAN_RX 16)
 - CAN bus speed on lines 54 (default: 500,000 bits/second)
 - SSID and password on lines 54-55 (default ESP32_RET_SD and motorvate)

DIY: CAN Data Logger for ESP32_RET_SD 
=======
You can easily build your own hardware for the ESP32_RET_SD
### You will need:
- 30 Pin ESP32 Dev Kit
- 3.3v CAN Bus Tranceiver (SN65HVD230)
- SD or micro SD module
- OBD Connector (Male)

### Then, connect everything as shown below: 

![CAN_logger_wiring_2](https://github.com/user-attachments/assets/4d57f81c-e4bc-42eb-9a5c-c3b9351b880e)

### Once Assembled, you can use the ESP Web Flasher (aka ESP Web Tools) and program it directly from Google Chrome or Microsoft Edge `(link to be added soon)` 


MotorvateDIY CAN LOGGER Story
=======

### First Version
Four years ago, this was my solution for low cost CAN bus data logging with SavvyCAN, using [A0RET](https://github.com/collin80/A0RET). That used a 30 pin, ESP32 dev kit and a 3.3v CAN bus transceiver (SN65HVD230) module and OBD pin 6 is connected to CAN HI, and pin 14 is CAN LO. 

Just plug it into the OBD diagnostic port (pre ~2016, no CAN gayeway!) connect it to a USB power bank and use a laptop running SavvyCAN to record the CAN bus over wifi. It was a low cost and reliable solution.
![orig_A0RET](https://github.com/user-attachments/assets/4bcdec63-5843-4e18-8ced-a2629d984728)

### Second Version
Sometimes, you don't want a laptop with you to record the CAN bus, all you want to do is plug something in and go. So...
I took the first version, added a DC to DC converter to drop the car's 12-14 volts down to ESP32 levels and added a power switch. 12v and ground is supplied from the OBD port (pin 16 - battery and pins 4+5 ground)

With a FAT32 formatted micro SD card (32GB or less), just plug it into the OBD diagnostic port and all the CAN bus data is recorded to the SD card in SavvyCAN CSV/GVRET format for easy analysis!

![Wired_CAN_logger_sm](https://github.com/user-attachments/assets/4f83cc47-bd61-4f2f-8e70-38bee62f55de)

### Third Version
I wanted to learn electronics design in Fusion360, so I started with this project. This resulted in a nice small PCB that fits inside a standard(ish) AliExpress OBD enclosure.

<img width="640" alt="Fusion_CAN_logger" src="https://github.com/user-attachments/assets/2c7e9d5f-ff47-48da-b6c1-d362de58234f" />


At this point I wanted the best of both worlds, plug and play CAN bus logging to SD card, and using SavvyCAN on wifi to send/receive CAN messages. This is how ESP32_RET_SD came to be. Over the last few months I've tested it on a few older vehicles (2011 & 2015) and all looks to be working well.

![Motorvate_CAN_Bus_Logger](https://github.com/user-attachments/assets/c75c5205-55a6-466d-a38e-6ca1c673e5ef)

Soooo, if you want a good and low cost way to start to learn about CAN bus, this just might be a good place to start.

### Step by Step CAN bus Sniffing and Reverse Analysis videos will be posted here, once editing is done.


### NOTE: Some vehicles after around 2015 may have a CAN bus gateway that block CAN bus messages from the OBD port, and nothing is recorded.


