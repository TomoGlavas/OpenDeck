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

#include "database/Database.h"
#include "midi/src/MIDI.h"

namespace IO
{
    ///
    /// \brief LED handling.
    /// \defgroup leds LEDs
    /// \ingroup interface
    /// @{
    ///
    class LEDs
    {
        public:
        enum class rgbIndex_t : uint8_t
        {
            r,
            g,
            b
        };

        enum class color_t : uint8_t
        {
            off,
            red,
            green,
            yellow,
            blue,
            magenta,
            cyan,
            white,
            AMOUNT
        };

        enum class setting_t : uint8_t
        {
            blinkWithMIDIclock,
            fadeSpeed,
            useStartupAnimation,
            AMOUNT
        };

        enum class controlType_t : uint8_t
        {
            midiInNoteForStateCCforBlink,
            localNoteForStateNoBlink,
            midiInCCforStateNoteForBlink,
            localCCforStateNoBlink,
            midiInPCforStateNoBlink,
            localPCforStateNoBlink,
            midiInNoteForStateAndBlink,
            localNoteForStateAndBlink,
            midiInCCforStateAndBlink,
            localCCforStateAndBlink,
            AMOUNT
        };

        enum class blinkSpeed_t : uint8_t
        {
            noBlink,
            s100ms,
            s200ms,
            s300ms,
            s400ms,
            s500ms,
            s600ms,
            s700ms,
            s800ms,
            s900ms,
            s1000ms,
            AMOUNT
        };

        enum class blinkType_t : uint8_t
        {
            timer,
            midiClock
        };

        class HWA
        {
            public:
            HWA() {}
            virtual void   setState(size_t index, bool state)                                      = 0;
            virtual size_t rgbSingleComponentIndex(size_t rgbIndex, LEDs::rgbIndex_t rgbComponent) = 0;
            virtual size_t rgbIndex(size_t singleLEDindex)                                         = 0;
            virtual void   setFadeSpeed(size_t transitionSpeed)                                    = 0;
        };

        LEDs(HWA& hwa, Database& database)
            : hwa(hwa)
            , database(database)
        {}

        void        init(bool startUp = true);
        void        checkBlinking(bool forceChange = false);
        void        setAllOn();
        void        setAllOff();
        void        setColor(uint8_t ledID, color_t color);
        color_t     getColor(uint8_t ledID);
        void        setBlinkState(uint8_t ledID, blinkSpeed_t value);
        bool        getBlinkState(uint8_t ledID);
        size_t      rgbSingleComponentIndex(size_t rgbIndex, LEDs::rgbIndex_t rgbComponent);
        size_t      rgbIndex(size_t singleLEDindex);
        bool        setFadeSpeed(uint8_t transitionSpeed);
        void        midiToState(MIDI::messageType_t messageType, uint8_t data1, uint8_t data2, uint8_t channel, bool local);
        void        setBlinkType(blinkType_t blinkType);
        blinkType_t getBlinkType();
        void        resetBlinking();

        private:
        enum class ledBit_t : uint8_t
        {
            active,     ///< LED is active (either it blinks or it's constantly on), this bit is OR function between blinkOn and state
            blinkOn,    ///< LED blinks
            state,      ///< LED is in constant state
            rgb,        ///< RGB enabled
            rgb_r,      ///< R index of RGB LED
            rgb_g,      ///< G index of RGB LED
            rgb_b       ///< B index of RGB LED
        };

        void         updateState(uint8_t index, ledBit_t bit, bool state, bool setOnBoard = true);
        bool         getState(uint8_t index, ledBit_t bit);
        void         resetState(uint8_t index);
        color_t      valueToColor(uint8_t receivedVelocity);
        blinkSpeed_t valueToBlinkSpeed(uint8_t value);
        void         handleLED(uint8_t ledID, bool state, bool rgbLED, rgbIndex_t index = rgbIndex_t::r);
        void         startUpAnimation();

        HWA&                    hwa;
        Database&               database;
        static constexpr size_t maxLEDs = MAX_NUMBER_OF_LEDS + MAX_TOUCHSCREEN_BUTTONS;

        ///
        /// \brief Array holding current LED status for all LEDs.
        ///
        uint8_t ledState[maxLEDs] = {};

        ///
        /// \brief Array holding time after which LEDs should blink.
        ///
        uint8_t blinkTimer[maxLEDs] = {};

        ///
        /// \brief Holds currently active LED blink type.
        ///
        blinkType_t ledBlinkType = blinkType_t::timer;

        ///
        /// \brief Pointer to array used to check if blinking LEDs should toggle state.
        ///
        const uint8_t* blinkResetArrayPtr = nullptr;

        ///
        /// \brief Array holding MIDI clock pulses after which LED state is toggled for all possible blink rates.
        ///
        const uint8_t blinkReset_midiClock[static_cast<uint8_t>(blinkSpeed_t::AMOUNT)] = {
            255,    //no blinking
            2,
            3,
            4,
            6,
            9,
            12,
            18,
            24,
            36,
            48
        };

        ///
        /// \brief Array holding time indexes (multipled by 100) after which LED state is toggled for all possible blink rates.
        ///
        const uint8_t blinkReset_timer[static_cast<uint8_t>(blinkSpeed_t::AMOUNT)] = {
            0,
            1,
            2,
            3,
            4,
            5,
            6,
            7,
            8,
            9,
            10
        };

        ///
        /// \brief Array used to determine when the blink state for specific blink rate should be changed.
        ///
        uint8_t blinkCounter[static_cast<uint8_t>(blinkSpeed_t::AMOUNT)] = {};

        ///
        /// \brief Holds last time in miliseconds when LED blinking has been updated.
        ///
        uint32_t lastLEDblinkUpdateTime = 0;

        ///
        // \brief Holds blink state for each blink speed so that leds are in sync.
        ///
        bool blinkState[static_cast<uint8_t>(blinkSpeed_t::AMOUNT)] = {};
    };
}    // namespace IO