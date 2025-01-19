ESP32 Based CAN Bus Reverse Engineering Tool with SD Card CAN Logging
=======
ESP32 Reverse Engineering Tool with stand alone SD CAN bus logging and wifi support for SavvyCAN.  
Based on [Collin Kidder](https://github.com/collin80) fantastic [A0RET](https://github.com/collin80/A0RET).

<p align="center">
<img width="640" alt="Fusion_CAN_logger" src="https://github.com/user-attachments/assets/10b025e7-0b5e-4ccc-b989-f0ef8a4e0be3" />
</p>



 ### Main Goal: Easy to use CAN Bus Recording to SD Card or with [SavvyCAN](https://github.com/collin80/SavvyCAN)
 - If an SD card  *IS*  detected at power up, all CAN bus data is saved to the SD card, in SavvyCAN, CSV/GVRET format 
 - If an SD card  *IS NOT*  detected, a WIFI access point is created for a SavvyCAN connection to receive and/or send CAN frames
 - When saving to the SD card and SavvyCAN connects, the SD stops recording and swithes to SavvyCAN mode.
 - When in SavvyCAN mode and the ESP32 wifi connection is disconnected, it restarts and will either save CAN data to the SD card (if SD card present), or be in wifi mode.

With a focus of ease of use the following changes have been made from [A0RET](https://github.com/collin80/A0RET)
 - Removed: ELM327 compatability
 - Removed: LAWICEL
 - Removed: Bluetooth
 - Removed: Digital and Analog I/O
 - Added: Saving all CAN data to SD card in SavvyCAN CSV format
 
 Now compiles using default 1.2MB APP partition 
 
SD, CAN and WIFI Configuration Settings
=======

**If using the MotorvateDIY CAN BUS LOGGER V1.0 board, no configuration changes are required.**\
OR change the required setting(s) in config.h to match your hardware:
 - SD card pins on lines 43-46 (default: Standard VSPI where SD_SCK 18, SD_MISO 19, SD_MOSI 23, SD_CS 5)
 - CAN Tx/Rx pins on lines 50-51 (defaut: GPIO_CAN_TX 17, GPIO_CAN_RX 16)
 - CAN bus speed on lines 54 (default: 500,000 bits/second)
 - SSID and password on lines 54-55 (default ESP32_RET_SD and motorvate)


How to Program/Flash the ESP32
=======

#### Web Flash Requirements:
- Your hardware must use the default config.h settings or use the MotorvateDIY CAN BUS LOGGER V1.0 board
- Only supported by Google Chrome or Microsoft Edge at this time
- If the above is good, use this link: [ESP32_RET_SD_webflash](https://motorvatediy.github.io/ESP32_RET_SD_WebFlash/flash.html)

#### Compile/Programming Requirements:
You will need the following to be able to compile the run this project:
- [Arduino IDE](https://www.arduino.cc/en/software) Tested on IDE v2.3.4
- [Arduino-ESP32 Support](https://docs.espressif.com/projects/arduino-esp32/en/latest/getting_started.html) - Allows for programming the ESP32 with the Arduino IDE (Tested on Arduino core 2.0.17 and 3.0.7)
- [esp32_can](https://github.com/MotorvateDIY/esp32_can) - A unified ESP32 CAN library. (Must use this version if you want SD CAN log timestamps)
- [can_common](https://github.com/collin80/can_common) - Used by esp32_can
- To record to a SD card, it must be 32GB or less and formatted in FAT32

**See parts list and wiring diagram below.**

DIY: CAN Data Logger for ESP32_RET_SD 
=======
You can easily build your own hardware for the ESP32_RET_SD
### You will need:
- 30 Pin ESP32 Dev Kit
- 3.3v CAN Bus Tranceiver (SN65HVD230)
- SD or micro SD module
- OBD Connector (Male)
- Micro USB cable and laptop or power bank for power or 5v DC to DC converter

### Then, connect everything as shown below: (click to enlarge)

<p align="center">
<img width="640" alt="Fusion_CAN_logger" src="https://github.com/user-attachments/assets/b3f7ae7b-d5ba-4573-8a0b-91fa7296a562" />
</p>



**Once Assembled, you can use the ESP Web Flasher with Google Chrome or Microsoft Edge: [ESP32_RET_SD_webflash](https://motorvatediy.github.io/ESP32_RET_SD_WebFlash/flash.html)**


MotorvateDIY CAN LOGGER Story
=======

### First Version
Four years ago, this what I used for a low cost (<$15!) CAN bus data logging tool: SavvyCAN, with [A0RET](https://github.com/collin80/A0RET). I used a 30 pin ESP32 dev kit, a 3.3v CAN bus transceiver (SN65HVD230) module and a male OBD connector. 

Plugging it into the OBD diagnostic port (pre ~2018, no CAN gateway!), connect it to a USB power bank and use a laptop running SavvyCAN to record the CAN bus over wifi. It was a low cost and reliable solution.

<p align="center">
<img width="480" alt="Fusion_CAN_logger_v1" src="https://github.com/user-attachments/assets/4bcdec63-5843-4e18-8ced-a2629d984728" />
</p>


### Second Version
Sometimes, you don't want a laptop with you to record the CAN bus, all you want to do is plug something in and go. So...
I switched to a ESP32 D1 Mini, added a micro SD card shield, a DC to DC converter to drop the car's 12-14 volts down to ESP32 levels and added a power switch. 

With a FAT32 formatted micro SD card (32GB or less) and plugging it into the OBD diagnostic port, all the CAN bus data is recorded to the SD card in SavvyCAN CSV/GVRET format for easy analysis.  I used this for three years without a single issue.

<p align="center">
<img width="480" alt="Fusion_CAN_logger_v2" src="https://github.com/user-attachments/assets/4f83cc47-bd61-4f2f-8e70-38bee62f55de" />
</p>

### Third Version
Using the electronics design feature in Fusion 360, I made nice small PCB that fits inside a standard(ish) AliExpress OBD enclosure. Assembly is now much faster than cutting, stripping and soldering a bunch of wires, like the second version -AND- you don't need to double check to make sure each wire is connected to the correct pin!

<p align="center">
<img width="640" alt="Fusion_CAN_logger_v3" src="https://github.com/user-attachments/assets/2c7e9d5f-ff47-48da-b6c1-d362de58234f" />
</p>

Why ESP32_RET_SD?
===
At this point I wanted the best of both worlds, **plug and play CAN bus logging to SD card**, and using **SavvyCAN on wifi to send/receive CAN messages**. This is how ESP32_RET_SD came to be. Over the last few months I've tested it on a few older vehicles (2011 & 2015) and all looks to be working well.

<p align="center">
<img width="640" alt="Fusion_CAN_logger" src="https://github.com/user-attachments/assets/c75c5205-55a6-466d-a38e-6ca1c673e5ef" />
</p>

Soooo, if you want a good and low cost way to start to learn about CAN bus, this just might be a good place to start.


**Step by Step CAN bus Sniffing and Reverse Analysis videos will be posted here and on my [YouTube Channel](https://www.youtube.com/MotorvateDIY), once editing is done.**


### NOTE: Some vehicles after around 2015 may have a CAN bus gateway that block CAN bus messages from the OBD port, and nothing is recorded.


