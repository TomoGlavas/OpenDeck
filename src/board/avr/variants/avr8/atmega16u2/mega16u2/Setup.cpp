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

#include "board/Internal.h"
#include "board/common/io/Helpers.h"
#include "Pins.h"
#include "core/src/general/IO.h"

namespace Board
{
    namespace detail
    {
        namespace setup
        {
            void io()
            {
                //bootloader/midi leds
                CORE_IO_CONFIG(LED_MIDI_IN_DIN_PORT, LED_MIDI_IN_DIN_PIN, core::io::pinMode_t::output);
                CORE_IO_CONFIG(LED_MIDI_OUT_DIN_PORT, LED_MIDI_OUT_DIN_PIN, core::io::pinMode_t::output);

                INT_LED_OFF(LED_MIDI_IN_DIN_PORT, LED_MIDI_IN_DIN_PIN);
                INT_LED_OFF(LED_MIDI_OUT_DIN_PORT, LED_MIDI_OUT_DIN_PIN);
            }

            void timers()
            {
                //set timer0 to ctc, used to show midi tx/rx status using on-board leds
                TCCR0A |= (1 << WGM01);                 //CTC mode
                TCCR0B |= (1 << CS01) | (1 << CS00);    //prescaler 64
                OCR0A = 249;                            //1ms
                TIMSK0 |= (1 << OCIE0A);                //compare match interrupt
            }
        }    // namespace setup
    }        // namespace detail
}    // namespace Board