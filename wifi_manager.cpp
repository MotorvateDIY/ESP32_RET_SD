#include "config.h"
#include "wifi_manager.h"
#include "gvret_comm.h"
#include "SerialConsole.h"
#include <WiFi.h>


#include <FS.h>
#include <SD.h>
extern File LogFile;

// used for writing to SD card
extern QueueHandle_t SD_Queue;

// external task to copy from xQueue and write to SD
extern TaskHandle_t xSDWrite;

static IPAddress broadcastAddr(255, 255, 255, 255);

WiFiManager::WiFiManager() {
  lastBroadcast = 0;
}

void WiFiManager::setup() {
  if (settings.wifiMode == 1)  // connect to an AP
  {
    WiFi.mode(WIFI_STA);
    WiFi.setSleep(false);  // sleeping could cause delays
    WiFi.begin((const char *)settings.SSID, (const char *)settings.WPA2Key);

    WiFiEventId_t eventID = WiFi.onEvent([](WiFiEvent_t event, WiFiEventInfo_t info) {
      Serial.print("WiFi lost connection. Reason: ");
      Serial.println(info.wifi_sta_disconnected.reason);
      SysSettings.isWifiConnected = false;
      if ((info.wifi_sta_disconnected.reason == 202) || (info.wifi_sta_disconnected.reason == 3)) {
        Serial.println("Connection failed, rebooting to fix it.");
        esp_sleep_enable_timer_wakeup(10);
        esp_deep_sleep_start();
        delay(100);
      }
    },
                                         WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_DISCONNECTED);
  }
  if (settings.wifiMode == 2)  // BE an AP
  {
    WiFi.mode(WIFI_AP);
    WiFi.setSleep(false);
    WiFi.softAP((const char *)settings.SSID, (const char *)settings.WPA2Key);

    WiFiEventId_t eventID = WiFi.onEvent([](WiFiEvent_t event, WiFiEventInfo_t info) {
      Serial.println("SavvyCAN disconnected from Wifi AP");
      Serial.println("Switching to CAN to SD mode.");

      esp_sleep_enable_timer_wakeup(10);
      esp_deep_sleep_start();
      delay(100);
    },
                                         WiFiEvent_t::ARDUINO_EVENT_WIFI_AP_STADISCONNECTED);
  }
}

void WiFiManager::loop() {
  boolean needServerInit = false;
  int i;
  // only enter if wifi is 1 (connect to AP) or 2 (be an AP)
  if (settings.wifiMode > 0) {
    if (!SysSettings.isWifiConnected) {
      if (WiFi.isConnected()) {
        WiFi.setSleep(false);
        Serial.print("Wifi now connected to SSID: ");
        Serial.println((const char *)settings.SSID);
        Serial.print("IP address: ");
        Serial.println(WiFi.localIP());
        Serial.print("RSSI: ");
        Serial.println(WiFi.RSSI());
        needServerInit = true;
      }

      // For SoftAP
      if (settings.wifiMode == 2) {
        Serial.print("\nWifi SSID: ");
        Serial.println((const char *)settings.SSID);
        Serial.print("IP address: ");
        Serial.println(WiFi.softAPIP());
        Logger::console("---------------------------------------");
        needServerInit = true;
      }
      if (needServerInit) {
        SysSettings.isWifiConnected = true;
        wifiServer.begin(23);  // setup as a telnet server
        wifiServer.setNoDelay(true);
        ArduinoOTA.setPort(3232);
        ArduinoOTA.setHostname("A0RET");
        // No authentication by default
        // ArduinoOTA.setPassword("admin");

        ArduinoOTA.onStart([]() {
                    String type;
                    if (ArduinoOTA.getCommand() == U_FLASH)
                      type = "sketch";
                    else  // U_SPIFFS
                      type = "filesystem";

                    // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
                    Serial.println("Start updating " + type);
                  })
          .onEnd([]() {
            Serial.println("\nEnd");
          })
          .onProgress([](unsigned int progress, unsigned int total) {
            Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
          })
          .onError([](ota_error_t error) {
            Serial.printf("Error[%u]: ", error);
            if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
            else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
            else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
            else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
            else if (error == OTA_END_ERROR) Serial.println("End Failed");
          });

        ArduinoOTA.begin();
      }
    } else {
      // if connecte to STAtion or wifi mode is SoftAP
      if (WiFi.isConnected() || settings.wifiMode == 2) {
        if (wifiServer.hasClient()) {
          for (i = 0; i < MAX_CLIENTS; i++) {
            if (!SysSettings.clientNodes[i] || !SysSettings.clientNodes[i].connected()) {
              if (SysSettings.clientNodes[i])
                SysSettings.clientNodes[i].stop();
              SysSettings.clientNodes[i] = wifiServer.available();
              if (!SysSettings.clientNodes[i])
                Serial.println("Couldn't accept client connection!");
              else {
                Serial.print("New client #");
                Serial.print(i);
                Serial.print(" @ ");
                Serial.println(SysSettings.clientNodes[i].remoteIP());
                // turn off ESP32 built in LED
                setLED(SysSettings.LED_CANRX, 0);
                if (SD_card_present) {
                  // stop logging to SD since SavvyCAN is connected
                  LogFile.flush();
                  LogFile.close();
                  Serial.println("\nSavvyCAN connected, SD logging finished");
                  SD_card_present = false;

                  SysSettings.sdToggle = false;
                  setLED(SysSettings.LED_SD, SysSettings.sdToggle);

                  // delete xQueue
                  vQueueDelete(SD_Queue);
                  // delete SD Writing task
                  vTaskDelete(xSDWrite);
                }
              }
            }
          }
          if (i >= MAX_CLIENTS) {
            // no free/disconnected spot so reject
            wifiServer.available().stop();
          }
        }

        // check clients for data
        for (i = 0; i < MAX_CLIENTS; i++) {
          if (SysSettings.clientNodes[i] && SysSettings.clientNodes[i].connected()) {
            if (SysSettings.clientNodes[i].available()) {
              // get data from the telnet client and push it to input processing
              while (SysSettings.clientNodes[i].available()) {
                uint8_t inByt;
                inByt = SysSettings.clientNodes[i].read();
                SysSettings.isWifiActive = true;
                // Serial.write(inByt); //echo to serial - just for debugging. Don't leave this on!
                wifiGVRET.processIncomingByte(inByt);
              }
            }
          } else {
            if (SysSettings.clientNodes[i]) {
              SysSettings.clientNodes[i].stop();
            }
          }
        }
      } else {
        if (settings.wifiMode == 1) {
          Serial.println("WiFi disconnected. Bummer!");
          SysSettings.isWifiConnected = false;
          SysSettings.isWifiActive = false;
        }
      }
    }
  }

  if (SysSettings.isWifiConnected && ((micros() - lastBroadcast) > 1000000ul))  // every second send out a broadcast ping
  {
    uint8_t buff[4] = { 0x1C, 0xEF, 0xAC, 0xED };
    lastBroadcast = micros();
    wifiUDPServer.beginPacket(broadcastAddr, 17222);
    wifiUDPServer.write(buff, 4);
    wifiUDPServer.endPacket();
  }

  ArduinoOTA.handle();
}

void WiFiManager::sendBufferedData() {
  for (int i = 0; i < MAX_CLIENTS; i++) {
    size_t wifiLength = wifiGVRET.numAvailableBytes();
    uint8_t *buff = wifiGVRET.getBufferedBytes();
    if (SysSettings.clientNodes[i] && SysSettings.clientNodes[i].connected()) {
      SysSettings.clientNodes[i].write(buff, wifiLength);
    }
  }
  wifiGVRET.clearBufferedBytes();
}
