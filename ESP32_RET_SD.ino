// ESP32_RET_SD
// ESP32 Reverse Engineering Tool with SD CAN bus datalogger for SavvyCAN
// Based on Collin Kidder (https://github.com/collin80) fantastic A0RET (https://github.com/collin80/A0RET)
// Updated by frank @ motorvate Jan 1/2024 to add standalone recording of all CAN data to the SD card in SavvyCAN format

// Main Goal: Easy to use, Plug and Play CAN bus datalogging
// If an SD card *IS* detected at boot up, all CAN bus data is recorded to the SD card, in SavvyCAN CSV format for offline processing
// If an SD card *IS NOT* detected, a WIFI access point is created for SavvyCAN to connect to and receive or send CAN frames

// compatable with Arduino core 2.0.x and 3.0.x
// Now compiles using default 1.2MB APP partition

// With a focus on "easy of use", an offline mode (CAN to SD) has been added to the SavvyCAN mode (CAN to Wifi)
// Change summary:
// Removed: ELM327 compatability
// Removed: LAWICEL
// Removed: Bluetooth
// Removed: Digital and Analog I/O

// Added: saves all CAN data to SD card in SavvyCAN CSV format just by plugging into the OBD port for offline processing/reverse engineering
// Added: when saving to SD card, the ESP32 builtin LED is toggled every 250 (CAN_RX_LED_TOGGLE) CAN frames (set in config.h)
// Added: getting timestamp as soon as possible, see gvret_comm.cpp line 558 and esp32_can_builtin.cpp line 398

// Instructions for use:
// in config.h:
// set SD card pins on lines 43-46. Uses VSPI default pins of: SCK 18, MISO 19, MOSI 23, CS 5
// set CAN Tx/Rx pins on lines 50-51. Defaults to CAN_TX 17, CAN_RX 16
// set CAN0 speed on line 54. Default speed is 500,000 bits/sec
// set SSID and password on lines 57-58. Default SSID: ESP32_RET_SD, password motorvate
// Compile and upload to ESP32!
// full video instructions at youtube.com/motorvateDIY

/*
 A0RET.ino

 Created: June 1, 2020
 Author: Collin Kidder

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
 */

#include <Arduino.h>

#include "config.h"
#include "esp32_can.h"
#include <SPI.h>
#include <Preferences.h>
#include "SerialConsole.h"
#include "wifi_manager.h"
#include "gvret_comm.h"
#include "can_manager.h"

// defines SSID, password, ESP32 built in LED / CAN GPIO pins
#include "config.h"

// for SD card
#include <FS.h>
#include <SD.h>
#include <esp_mac.h>
#include <esp_timer.h>

// Required for SD card
char LogFileName[40];
File LogFile;
bool SD_card_present = false;
char SD_buffer[WIFI_BUFF_SIZE + 1];

// uses VSPI SPI, but can be changed to HSPI
SPIClass spi = SPIClass(VSPI);

byte i = 0;

uint32_t lastFlushMicros = 0;

bool markToggle[6];
uint32_t lastMarkTrigger = 0;

EEPROMSettings settings;
SystemSettings SysSettings;
Preferences nvPrefs;

WiFiManager wifiManager;

GVRET_Comm_Handler serialGVRET;  // gvret protocol over the serial to USB connection
GVRET_Comm_Handler wifiGVRET;    // GVRET over the wifi telnet port
CANManager canManager;           // keeps track of bus load and abstracts away some details of how things are done

SerialConsole console;

// RTOS task to read xQueue and write to SD
void TaskSDWrite(void *pvParameters);

TaskHandle_t xSDWrite;
QueueHandle_t SD_Queue;

// initializes all the system EEPROM values. Chances are this should be broken out a bit but
// there is only one checksum check for all of them so it's simple to do it all here.
// NOTE: IF PREFS ERROR, USE ? AND SET ONE ITEM. This writes the SSID< WPA2KEY and ELM327-BT to flash
void loadSettings() {
  settings.CAN0Speed = CAN_SPEED_500K;
  settings.CAN0_Enabled = true;
  settings.CAN0ListenOnly = false;
  settings.useBinarySerialComm = false;
  settings.logLevel = 1;    // info
  settings.wifiMode = 2;    // Wifi defaults to creating an AP
  settings.systemType = 0;  // ESP32 with single CAN bus
  settings.CAN1Speed = 500000;
  // below needs strcpy to copy the short string into the larger string 32/64
  strcpy(settings.SSID, SSID_NAME);
  strcpy(settings.WPA2Key, WPA2KEY);

  Logger::setLoglevel((Logger::LogLevel)settings.logLevel);

  SysSettings.LED_CANTX = 255;
  // toggle built in LED to show CAN Rx activity
  SysSettings.LED_CANRX = 255;
  SysSettings.LED_SD = ESP32_BUILTIN_LED;
  SysSettings.LED_LOGGING = 255;
  SysSettings.logToggle = false;
  SysSettings.sdToggle = false;
  SysSettings.txToggle = false;
  SysSettings.rxToggle = true;
  SysSettings.numBuses = 1;  // Currently we support CAN0
  SysSettings.isWifiActive = false;
  SysSettings.isWifiConnected = false;
}

uint32_t chipId = 0;

String getDefaultMacAddress() {

  String mac = "";

  unsigned char mac_base[6] = { 0 };

  if (esp_efuse_mac_get_default(mac_base) == ESP_OK) {
    char buffer[18];  // 6*2 characters for hex + 5 characters for colons + 1 character for null terminator
    sprintf(buffer, "%02X:%02X:%02X:%02X:%02X:%02X", mac_base[0], mac_base[1], mac_base[2], mac_base[3], mac_base[4], mac_base[5]);
    mac = buffer;
  }

  return mac;
}

String getInterfaceMacAddress(esp_mac_type_t interface) {

  String mac = "";

  unsigned char mac_base[6] = { 0 };

  if (esp_read_mac(mac_base, interface) == ESP_OK) {
    char buffer[18];  // 6*2 characters for hex + 5 characters for colons + 1 character for null terminator
    sprintf(buffer, "%02X:%02X:%02X:%02X:%02X:%02X", mac_base[0], mac_base[1], mac_base[2], mac_base[3], mac_base[4], mac_base[5]);
    mac = buffer;
  }

  return mac;
}

void setup() {

  Serial.begin(115200);

  // display ESP32 details
  for (int i = 0; i < 17; i = i + 8) {
    chipId |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;
  }

  Serial.printf("\n\nESP32 model: %s Rev %d\n", ESP.getChipModel(), ESP.getChipRevision());
  Serial.printf("Number of cores: %d\n", ESP.getChipCores());
  Serial.print("Chip ID: ");
  Serial.println(chipId);

  Serial.print("WiFi Soft-AP MAC Address: ");
  // Serial.print("WiFi Soft-AP (using 'esp_read_mac')\t\t\t");
  Serial.println(getInterfaceMacAddress(ESP_MAC_WIFI_SOFTAP));

  // use ESP32 built in LED for SD card activity
  pinMode(ESP32_BUILTIN_LED, OUTPUT);

  Logger::console("\n---------------------------------------");
  Logger::console("Hardware: Motorvate CAN Bus Logger v1.0");
  Logger::console("Firmware: %s", CFG_VERSION);
  // Serial.print("   Build: ");
  // Serial.println(CFG_BUILD_NUM);
  Serial.println();

  // SD card pins are defined in config.h and can easily be changed for your hardware
  // MotorvateDIY CAN BUS LOGGER v1.0 uses the default VSPI pins
  spi.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);

  // set SPI clock speed to 30 MHz
  if (!SD.begin(SD_CS, spi, 30000000)) {
    Serial.print("\nSD card mount failed, ");
  }

  uint8_t cardType = SD.cardType();

  if (cardType == CARD_NONE) {
    Serial.println("no SD card found");
    SD_card_present = false;
  } else {
    SD_card_present = true;

    // SD card Queue creation only if SD card detected
    SD_Queue = xQueueCreate(20, sizeof(SD_buffer));
    if (SD_Queue == NULL) {
      Serial.println("Error Creating the SD Queue");
    }

    // create task to read xQueue and write to SD card
    xTaskCreatePinnedToCore(
      TaskSDWrite,  // task function to call
      "SD Write",   // name of task for optional eports
      2048,         // Stack depth/size in bytes
      NULL,         // pvParameters
      2,            // Priority 1, same as setup() and loop(), now priority of 2
      &xSDWrite,    // task handle
      // NULL,     // no task handle
      1);  // core to use for this task

    Serial.print("SD card type: ");
    if (cardType == CARD_MMC) {
      Serial.println("MMC");
    } else if (cardType == CARD_SD) {
      Serial.println("SDSC");
    } else if (cardType == CARD_SDHC) {
      Serial.println("SDHC");
    } else {
      Serial.println("UNKNOWN");
    }

    // display SD card details
    Serial.printf("Sector Size: %d\n", SD.sectorSize());
    Serial.printf("SD Card Size: %llu MB\n", SD.cardSize() / (1024 * 1024));
    Serial.printf("Available space: %llu MB\n", (SD.totalBytes() - SD.usedBytes()) / (1024 * 1024));

    // create a unique LogFile filename
    int fn = 0;
    do {
      sprintf(LogFileName, "/CAN_LOG_%d.csv", fn++);
    } while (SD.exists(LogFileName));
    LogFile = SD.open(LogFileName, FILE_APPEND);

    // Save SavvyCAN CSV header and flush buffer
    LogFile.print("Time Stamp,ID,Extended,Dir,Bus,LEN,D1,D2,D3,D4,D5,D6,D7,D8\n");
    LogFile.flush();

    Serial.printf("CAN log filename: %s\n", LogFileName);

  }  // end SD_card_present

  // sets CAN speed, LED pins, WIFI, SSID, etc
  loadSettings();

  // starts CAN
  canManager.setup();


  if (SD_card_present) {
    Serial.println("*** Recording CAN bus to SD card ***");
  } else {
    Serial.println("*** Ready for SavvyCAN Network (GVRET) connection ***");
  }

  SysSettings.isWifiConnected = false;

  // allow CAN bus to start up before starting wifi
  delay(250);

  // setup wifi
  wifiManager.setup();
}

/*
Send a fake frame out USB and maybe to file to show where the mark was triggered at. The fake frame has bits 31 through 3
set which can never happen in reality since frames are either 11 or 29 bit IDs. So, this is a sign that it is a mark frame
and not a real frame. The bottom three bits specify which mark triggered.
*/
void sendMarkTriggered(int which) {
  CAN_FRAME frame;
  frame.id = 0xFFFFFFF8ull + which;
  frame.extended = true;
  frame.length = 0;
  frame.rtr = 0;
  canManager.displayFrame(frame, 0);
}

/*
Loop executes as often as possible all the while interrupts fire in the background.
The serial comm protocol is as follows:
All commands start with 0xF1 this helps to synchronize if there were comm issues
Then the next byte specifies which command this is.
Then the command data bytes which are specific to the command
Lastly, there is a checksum byte just to be sure there are no missed or duped bytes
Any bytes between checksum and 0xF1 are thrown away

Yes, this should probably have been done more neatly but this way is likely to be the
fastest and safest with limited function calls
*/
void loop() {
  // uint32_t temp32;
  bool isConnected = false;
  int serialCnt;
  uint8_t in_byte;

  /*if (Serial)*/ isConnected = true;

  canManager.loop();
  wifiManager.loop();

  size_t wifiLength = wifiGVRET.numAvailableBytes();
  size_t serialLength = serialGVRET.numAvailableBytes();

  size_t maxLength = (wifiLength > serialLength) ? wifiLength : serialLength;

  // If the max time has passed or the buffer is almost filled then send buffered data out
  if ((micros() - lastFlushMicros > SER_BUFF_FLUSH_INTERVAL) || (maxLength > (WIFI_BUFF_SIZE - 40))) {
    lastFlushMicros = micros();
    if (serialLength > 0) {

      if (SD_card_present)
      // saves CAN frames to uSD card in SavvyCAN CSV format
      {
        // set all of SD_buffer to 0
        memset(SD_buffer, 0, WIFI_BUFF_SIZE + 1);
        // copy serialGVRET.getBufferedBytes() > transmitBuffer to SD_buffer
        memcpy(&SD_buffer, serialGVRET.getBufferedBytes(), serialLength);

        if (xQueueSend(SD_Queue, (void *)&SD_buffer, 10 * portTICK_PERIOD_MS) != pdPASS) {
          Serial.println("xQueueSend to SD_Queue 10ms timeout error");
        }

        // flush SD card every 20ms
        LogFile.flush();

        serialGVRET.clearBufferedBytes();
        // only toggle Rx LED when writing to SD card, regardless of CAN activity

        // toggles every 20ms x 100 = 2 seconds
        // toggleRXLED();
        // below works, but blinks too fast
        // toggleSD_LED();
      } else {
        serialGVRET.clearBufferedBytes();
      }
    }

    if (wifiLength > 0) {
      wifiManager.sendBufferedData();
    }
  }

  serialCnt = 0;
  while ((Serial.available() > 0) && serialCnt < 128) {
    serialCnt++;
    in_byte = Serial.read();
    serialGVRET.processIncomingByte(in_byte);
  }
}

// write data to SD card
void TaskSDWrite(void *pvParameters) {
  for (;;) {
    // blocks/waits 1000 ms to receive data from SD_Queue
    if (xQueueReceive(SD_Queue, &SD_buffer, 1000 * portTICK_PERIOD_MS) != pdPASS) {
      Serial.println("SD queue receive error: 1000ms second timeout.");
    } else {
      LogFile.print(SD_buffer);
    }
  }
}
