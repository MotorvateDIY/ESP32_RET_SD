ESP32_RET_SD
=======
ESP32 Reverse Engineering Tool with stand alone SD CAN bus recorder and wifi support for SavvyCAN.
 Based on [Collin Kidder](https://github.com/collin80) fantastic [A0RET](https://github.com/collin80/A0RET)

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


Instructions for use:
=======
 in config.h set:
 - SD card pins on lines 43-46 (default: Standard VSPI where SD_SCK 18, SD_MISO 19, SD_MOSI 23, SD_CS 5)
 - CAN Tx/Rx pins on lines 50-51 (defaut: GPIO_CAN_TX 17, GPIO_CAN_RX 16)
 - CAN bus speed on lines 54 (default: 500,000 bits/second)
 - SSID and password on lines 54-55 (default ESP32_RET_SD and motorvate)


