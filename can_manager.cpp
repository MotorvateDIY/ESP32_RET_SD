#include <Arduino.h>
#include "can_manager.h"
#include "esp32_can.h"
#include "config.h"
#include "SerialConsole.h"
#include "gvret_comm.h"

uint8_t CAN_frame_count;

CANManager::CANManager()
{
}

void CANManager::setup()
{
    if (settings.CAN0_Enabled)
    {
        if (settings.systemType == 0)
        {
            CAN0.setCANPins(GPIO_CAN_RX, GPIO_CAN_TX);
            CAN0.enable();
            CAN0.begin(settings.CAN0Speed, 255);
            Serial.print("\nCAN0 enabled at: ");
            Serial.print(settings.CAN0Speed);
            Serial.println(" bits/sec");
        }

        if (settings.CAN0ListenOnly)
        {
            CAN0.setListenOnlyMode(true);
        }
        else
        {
            CAN0.setListenOnlyMode(false);
        }
        CAN0.watchFor();
    }
    else
    {
        CAN0.disable();
    }
}

void CANManager::addBits(int offset, CAN_FRAME &frame)
{
    if (offset < 0)
        return;
    if (offset >= NUM_BUSES)
        return;
    busLoad[offset].bitsSoFar += 41 + (frame.length * 9);
    if (frame.extended)
        busLoad[offset].bitsSoFar += 18;
}

void CANManager::sendFrame(CAN_COMMON *bus, CAN_FRAME &frame)
{
    int whichBus;
    if (bus == &CAN0)
        whichBus = 0;
    if (bus == &CAN1)
        whichBus = 1;
    bus->sendFrame(frame);
    addBits(whichBus, frame);
}

void CANManager::displayFrame(CAN_FRAME &frame, int whichBus)
{
    if (settings.enableLawicel && SysSettings.lawicelMode)
    {
        // lawicel.sendFrameToBuffer(frame, whichBus);
    }
    else
    {
        if (SysSettings.isWifiActive)
            wifiGVRET.sendFrameToBuffer(frame, whichBus);
        else
            serialGVRET.sendFrameToBuffer(frame, whichBus);
    }
}

void CANManager::loop()
{
    CAN_FRAME incoming;
    size_t wifiLength = wifiGVRET.numAvailableBytes();
    size_t serialLength = serialGVRET.numAvailableBytes();
    size_t maxLength = (wifiLength > serialLength) ? wifiLength : serialLength;

    while (CAN0.available() > 0 && (maxLength < (WIFI_BUFF_SIZE - 80)))
    {
        CAN0.read(incoming);
        addBits(0, incoming);

        if (SD_card_present)
        {
            // only toggle built in LED when writing to SD card
            // LED will change state every 250 CAN frames
            if (CAN_frame_count >= CAN_RX_LED_TOGGLE)
            {
                // toggle ESP32 builtin LED every 250 frames
                toggleSD_LED();
                CAN_frame_count = 0;
            }

            CAN_frame_count++;
        } // end if (SD_card_present)

        displayFrame(incoming, 0);
        wifiLength = wifiGVRET.numAvailableBytes();
        serialLength = serialGVRET.numAvailableBytes();
        maxLength = (wifiLength > serialLength) ? wifiLength : serialLength;
    }
}
