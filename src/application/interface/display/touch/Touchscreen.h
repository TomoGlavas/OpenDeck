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

#include <inttypes.h>

namespace Interface
{
    ///
    /// \brief Touchscreen control.
    /// \defgroup interfaceLCDTouch Touchscreen
    /// \ingroup interfaceLCD
    /// @{

    class Touchscreen
    {
        public:
        class HWA
        {
            public:
            virtual bool init()                                    = 0;
            virtual void setScreen(uint8_t screenID)               = 0;
            virtual bool update(uint8_t& buttonID, bool& state)    = 0;
            virtual void setButtonState(uint8_t index, bool state) = 0;
        };

        Touchscreen(HWA& hwa)
            : hwa(hwa)
        {}
        bool    init();
        void    update();
        void    setScreen(uint8_t screenID);
        uint8_t activeScreen();
        void    setButtonHandler(void (*fptr)(uint8_t index, bool state));
        void    setButtonState(uint8_t index, bool state);

        private:
        HWA& hwa;

        void (*buttonHandler)(uint8_t index, bool state) = nullptr;
        uint8_t activeScreenID                           = 0;
        bool    initialized                              = false;
    };

    /// @}
}    // namespace Interface