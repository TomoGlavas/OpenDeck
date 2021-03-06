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

#pragma once

#include "io/touchscreen/Touchscreen.h"
#include "Config.h"
#include "Commands.h"

class SDW : public IO::Touchscreen::Model
{
    public:
    SDW(IO::Touchscreen::Model::HWA& hwa)
        : hwa(hwa)
    {}

    bool init() override;
    bool setScreen(size_t screenID) override;
    bool update(size_t& buttonID, bool& state) override;
    void setIconState(IO::Touchscreen::icon_t& icon, bool state) override;

    private:
    IO::Touchscreen::Model::HWA& hwa;

    ///
    /// \brief Enumeration holding different byte types for display messages.
    ///
    enum class messageByteType_t : uint8_t
    {
        start,
        content,
        singleByte,
        end
    };

    const uint8_t endCode[END_CODES] = {
        END_CODE1,
        END_CODE2,
        END_CODE3,
        END_CODE4
    };

    void sendMessage(uint8_t value, messageByteType_t messageByteType);

    uint8_t displayRxBuffer[TOUCHSCREEN_RX_BUFFER_SIZE] = {};
    uint8_t bufferIndex_rx                              = 0;
};