/*
 * sys_io.cpp
 *
 * Handles the low level details of system I/O
 *
Copyright (c) 2013-2018 Collin Kidder, Michael Neuweiler, Charles Galpin

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

some portions based on code credited as:
Arduino Due ADC->DMA->USB 1MSPS
by stimmer
*/

#include "sys_io.h"

void setLED(uint8_t which, boolean hi)
{
    if (which == 255)
    {
        return;
    }
    if (hi)
    {
        digitalWrite(which, HIGH);
    }
    else
    {
        digitalWrite(which, LOW);
    }
}

void toggleRXLED()
{
    static int counter = 0;
    counter++;
    if (counter >= BLINK_SLOWNESS)
    {
        counter = 0;
        SysSettings.rxToggle = !SysSettings.rxToggle;
        setLED(SysSettings.LED_CANRX, SysSettings.rxToggle);
    }
}

void toggleSD_LED()
{
    SysSettings.sdToggle = !SysSettings.sdToggle;
    setLED(SysSettings.LED_SD, SysSettings.sdToggle);
}

void toggleTXLED()
{
    static int counter = 0;
    counter++;
    if (counter >= BLINK_SLOWNESS)
    {
        counter = 0;
        SysSettings.txToggle = !SysSettings.txToggle;
        setLED(SysSettings.LED_CANTX, SysSettings.txToggle);
    }
}
