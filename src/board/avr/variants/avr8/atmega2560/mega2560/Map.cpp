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

#include "Pins.h"
#include "board/Internal.h"

namespace Board
{
    namespace detail
    {
        namespace map
        {
            namespace
            {
                uint8_t aInChannels[MAX_NUMBER_OF_ANALOG] = {
                    0,     //AI_1_PIN, ADC0
                    1,     //AI_2_PIN, ADC1
                    2,     //AI_3_PIN, ADC2
                    3,     //AI_4_PIN, ADC3
                    4,     //AI_5_PIN, ADC4
                    5,     //AI_6_PIN, ADC5
                    6,     //AI_7_PIN, ADC6
                    7,     //AI_8_PIN, ADC7
                    8,     //AI_9_PIN, ADC8
                    9,     //AI_10_PIN, ADC9
                    10,    //AI_11_PIN, ADC10
                    11,    //AI_12_PIN, ADC11
                    12,    //AI_13_PIN, ADC12
                    13,    //AI_14_PIN, ADC13
                    14,    //AI_15_PIN, ADC14
                    15,    //AI_16_PIN, ADC15
                };

                ///
                /// \brief Array holding ports and bits for all digital input pins.
                ///
                const core::io::mcuPin_t dInPins[MAX_NUMBER_OF_BUTTONS] = {
                    CORE_IO_MCU_PIN_DEF(DI_1_PORT, DI_1_PIN),
                    CORE_IO_MCU_PIN_DEF(DI_2_PORT, DI_2_PIN),
                    CORE_IO_MCU_PIN_DEF(DI_3_PORT, DI_3_PIN),
                    CORE_IO_MCU_PIN_DEF(DI_4_PORT, DI_4_PIN),
                    CORE_IO_MCU_PIN_DEF(DI_5_PORT, DI_5_PIN),
                    CORE_IO_MCU_PIN_DEF(DI_6_PORT, DI_6_PIN),
                    CORE_IO_MCU_PIN_DEF(DI_7_PORT, DI_7_PIN),
                    CORE_IO_MCU_PIN_DEF(DI_8_PORT, DI_8_PIN),
                    CORE_IO_MCU_PIN_DEF(DI_9_PORT, DI_9_PIN),
                    CORE_IO_MCU_PIN_DEF(DI_10_PORT, DI_10_PIN),
                    CORE_IO_MCU_PIN_DEF(DI_11_PORT, DI_11_PIN),
                    CORE_IO_MCU_PIN_DEF(DI_12_PORT, DI_12_PIN),
                    CORE_IO_MCU_PIN_DEF(DI_13_PORT, DI_13_PIN),
                    CORE_IO_MCU_PIN_DEF(DI_14_PORT, DI_14_PIN),
                    CORE_IO_MCU_PIN_DEF(DI_15_PORT, DI_15_PIN),
                    CORE_IO_MCU_PIN_DEF(DI_16_PORT, DI_16_PIN),
                    CORE_IO_MCU_PIN_DEF(DI_17_PORT, DI_17_PIN),
                    CORE_IO_MCU_PIN_DEF(DI_18_PORT, DI_18_PIN),
                    CORE_IO_MCU_PIN_DEF(DI_19_PORT, DI_19_PIN),
                    CORE_IO_MCU_PIN_DEF(DI_20_PORT, DI_20_PIN),
                    CORE_IO_MCU_PIN_DEF(DI_21_PORT, DI_21_PIN),
                    CORE_IO_MCU_PIN_DEF(DI_22_PORT, DI_22_PIN),
                    CORE_IO_MCU_PIN_DEF(DI_23_PORT, DI_23_PIN),
                    CORE_IO_MCU_PIN_DEF(DI_24_PORT, DI_24_PIN),
                    CORE_IO_MCU_PIN_DEF(DI_25_PORT, DI_25_PIN),
                    CORE_IO_MCU_PIN_DEF(DI_26_PORT, DI_26_PIN),
                    CORE_IO_MCU_PIN_DEF(DI_27_PORT, DI_27_PIN),
                    CORE_IO_MCU_PIN_DEF(DI_28_PORT, DI_28_PIN),
                    CORE_IO_MCU_PIN_DEF(DI_29_PORT, DI_29_PIN),
                    CORE_IO_MCU_PIN_DEF(DI_30_PORT, DI_30_PIN),
                    CORE_IO_MCU_PIN_DEF(DI_31_PORT, DI_31_PIN),
                    CORE_IO_MCU_PIN_DEF(DI_32_PORT, DI_32_PIN)
                };

                ///
                /// \brief Array holding ports and bits for all digital output pins.
                ///
                const core::io::mcuPin_t dOutPins[MAX_NUMBER_OF_LEDS] = {
                    CORE_IO_MCU_PIN_DEF(DO_1_PORT, DO_1_PIN),
                    CORE_IO_MCU_PIN_DEF(DO_2_PORT, DO_2_PIN),
                    CORE_IO_MCU_PIN_DEF(DO_3_PORT, DO_3_PIN),
                    CORE_IO_MCU_PIN_DEF(DO_4_PORT, DO_4_PIN),
                    CORE_IO_MCU_PIN_DEF(DO_5_PORT, DO_5_PIN),
                    CORE_IO_MCU_PIN_DEF(DO_6_PORT, DO_6_PIN),
                    CORE_IO_MCU_PIN_DEF(DO_7_PORT, DO_7_PIN),
                    CORE_IO_MCU_PIN_DEF(DO_8_PORT, DO_8_PIN),
                    CORE_IO_MCU_PIN_DEF(DO_9_PORT, DO_9_PIN),
                    CORE_IO_MCU_PIN_DEF(DO_10_PORT, DO_10_PIN),
                    CORE_IO_MCU_PIN_DEF(DO_11_PORT, DO_11_PIN),
                    CORE_IO_MCU_PIN_DEF(DO_12_PORT, DO_12_PIN),
                    CORE_IO_MCU_PIN_DEF(DO_13_PORT, DO_13_PIN),
                    CORE_IO_MCU_PIN_DEF(DO_14_PORT, DO_14_PIN),
                    CORE_IO_MCU_PIN_DEF(DO_15_PORT, DO_15_PIN),
                    CORE_IO_MCU_PIN_DEF(DO_16_PORT, DO_16_PIN)
                };
            }    // namespace

            uint32_t adcChannel(uint8_t index)
            {
                return aInChannels[index];
            }

            core::io::mcuPin_t button(uint8_t index)
            {
                return dInPins[index];
            }

            core::io::mcuPin_t led(uint8_t index)
            {
                return dOutPins[index];
            }
        }    // namespace map
    }        // namespace detail
}    // namespace Board