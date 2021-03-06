/*

Copyright 2015-2020 Igor Petrovic

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

*/

#include "U8X8.h"

using namespace IO;

bool U8X8::initDisplay(displayController_t controller, displayResolution_t resolution)
{
    bool success = false;

    //setup defaults
    u8x8_SetupDefaults(&u8x8);

    //i2c hw access
    auto gpioDelay = [](u8x8_t* u8x8, uint8_t msg, uint8_t arg_int, U8X8_UNUSED void* arg_ptr) -> uint8_t {
        return 0;
    };

    //hack: make the hwa reference static pointer so that access to hwa
    //works inside lambda
    //we are interfacing with C library!

    static HWAI2C* hwaStatic;
    hwaStatic = &this->hwa;

    auto i2cHWA = [](u8x8_t* u8x8, uint8_t msg, uint8_t arg_int, void* arg_ptr) -> uint8_t {
        auto* array = (uint8_t*)arg_ptr;

        switch (msg)
        {
        case U8X8_MSG_BYTE_SEND:
            for (int i = 0; i < arg_int; i++)
                hwaStatic->write(array[i]);
            break;

        case U8X8_MSG_BYTE_INIT:
            hwaStatic->init();
            break;

        case U8X8_MSG_BYTE_START_TRANSFER:
            hwaStatic->transfer(u8x8_GetI2CAddress(u8x8), HWAI2C::transferType_t::write);
            break;

        case U8X8_MSG_BYTE_END_TRANSFER:
            hwaStatic->stop();
            break;

        default:
            return 0;
        }

        return 1;
    };

    //setup specific callbacks depending on controller/resolution
    if ((resolution == displayResolution_t::_128x64) && (controller == displayController_t::ssd1306))
    {
        u8x8.display_cb        = u8x8_d_ssd1306_128x64_noname;
        u8x8.cad_cb            = u8x8_cad_ssd13xx_i2c;
        u8x8.byte_cb           = i2cHWA;
        u8x8.gpio_and_delay_cb = gpioDelay;
        rows                   = 4;
        columns                = 16;
        success                = true;
    }
    else if ((resolution == displayResolution_t::_128x32) && (controller == displayController_t::ssd1306))
    {
        u8x8.display_cb        = u8x8_d_ssd1306_128x32_univision;
        u8x8.cad_cb            = u8x8_cad_ssd13xx_i2c;
        u8x8.byte_cb           = i2cHWA;
        u8x8.gpio_and_delay_cb = gpioDelay;
        rows                   = 2;
        columns                = 16;
        success                = true;
    }

    if (success)
    {
        /* setup display info */
        u8x8_SetupMemory(&u8x8);
        u8x8_InitDisplay(&u8x8);

        clearDisplay();
        setPowerSave(0);

        return true;
    }

    return false;
}

uint8_t U8X8::getColumns()
{
    return columns;
}

uint8_t U8X8::getRows()
{
    return rows;
}

void U8X8::clearDisplay()
{
    u8x8_ClearDisplay(&u8x8);
}

void U8X8::setPowerSave(uint8_t is_enable)
{
    u8x8_SetPowerSave(&u8x8, is_enable);
}

void U8X8::setFlipMode(uint8_t mode)
{
    u8x8_SetFlipMode(&u8x8, mode);
}

void U8X8::setFont(const uint8_t* font_8x8)
{
    u8x8_SetFont(&u8x8, font_8x8);
}

void U8X8::drawGlyph(uint8_t x, uint8_t y, uint8_t encoding)
{
    u8x8_DrawGlyph(&u8x8, x, y, encoding);
}