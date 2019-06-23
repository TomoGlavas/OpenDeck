/*

Copyright 2015-2019 Igor Petrovic

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

#include <util/crc16.h>
#include <avr/cpufunc.h>
#include "BootloaderHID.h"
#include "core/src/avr/PinManipulation.h"
#include "core/src/avr/Misc.h"
#include "core/src/general/BitManipulation.h"
#include "board/common/constants/LEDs.h"
#include "board/common/constants/Reboot.h"
#include "Pins.h"
#include "board/Board.h"
#include "Redef.h"
#include "../../common/OpenDeckMIDIformat/OpenDeckMIDIformat.h"

namespace
{
    ///
    /// \brief Initializes all pins needed in bootloader mode.
    ///
    void pins()
    {
//indicate that we're in bootloader mode by turning on specific leds
//if the board uses indicator leds for midi traffic, turn them both on
#ifdef BOARD_KODAMA
        CORE_AVR_PIN_SET_LOW(SR_OUT_LATCH_PORT, SR_OUT_LATCH_PIN);

        for (int i = 0; i < 16; i++)
        {
            EXT_LED_ON(SR_OUT_DATA_PORT, SR_OUT_DATA_PIN);
            CORE_AVR_PIN_SET_HIGH(SR_OUT_CLK_PORT, SR_OUT_CLK_PIN);
            _NOP();
            _NOP();
            CORE_AVR_PIN_SET_LOW(SR_OUT_CLK_PORT, SR_OUT_CLK_PIN);
        }

        CORE_AVR_PIN_SET_HIGH(SR_OUT_LATCH_PORT, SR_OUT_LATCH_PIN);
//make sure internal led is turned off for mega/uno
#elif defined(BOARD_A_MEGA)
        CORE_AVR_PIN_SET_OUTPUT(PORTB, 7);
        CORE_AVR_PIN_SET_LOW(PORTB, 7);
#elif defined(BOARD_A_UNO)
        CORE_AVR_PIN_SET_OUTPUT(PORTB, 5);
        CORE_AVR_PIN_SET_LOW(PORTB, 5);
#elif defined(BOARD_T_2PP)
        //only one led
        CORE_AVR_PIN_SET_OUTPUT(LED_IN_PORT, LED_IN_PIN);
        INT_LED_ON(LED_IN_PORT, LED_IN_PIN);
#elif defined(LED_INDICATORS)
        CORE_AVR_PIN_SET_OUTPUT(LED_IN_PORT, LED_IN_PIN);
        CORE_AVR_PIN_SET_OUTPUT(LED_OUT_PORT, LED_OUT_PIN);
        INT_LED_ON(LED_IN_PORT, LED_IN_PIN);
        INT_LED_ON(LED_OUT_PORT, LED_OUT_PIN);
#endif

//configure bootloader entry pins
#if defined(BOARD_KODAMA)
        CORE_AVR_PIN_SET_INPUT(SR_DIN_DATA_PORT, SR_DIN_DATA_PIN);
        CORE_AVR_PIN_SET_OUTPUT(SR_DIN_CLK_PORT, SR_DIN_CLK_PIN);
        CORE_AVR_PIN_SET_OUTPUT(SR_DIN_LATCH_PORT, SR_DIN_LATCH_PIN);
#elif defined(BTLDR_BUTTON_PORT)
        CORE_AVR_PIN_SET_INPUT(BTLDR_BUTTON_PORT, BTLDR_BUTTON_PIN);
        CORE_AVR_PIN_SET_HIGH(BTLDR_BUTTON_PORT, BTLDR_BUTTON_PIN);
#endif
    }

    ///
    /// \brief Configures all hardware required for the bootloader.
    ///
    void setupHardware(void)
    {
        //disable clock division
        clock_prescale_set(clock_div_1);

        //relocate the interrupt vector table to the bootloader section
        MCUCR = (1 << IVCE);
        MCUCR = (1 << IVSEL);

#ifdef USB_SUPPORTED
        //initialize USB subsystem
        USB_Init();
#endif

#if defined(BOARD_A_xu2) || !defined(USB_SUPPORTED)
        Board::UART::init(UART_BAUDRATE_MIDI_OD, UART_USB_LINK_CHANNEL);

#ifndef USB_SUPPORTED
        // make sure USB link goes to bootloader mode as well
        MIDI::USBMIDIpacket_t packet;

        packet.Event = 0x00;
        packet.Data1 = 0x00;
        packet.Data2 = 0x00;
        packet.Data3 = 0x00;

        OpenDeckMIDIformat::write(UART_USB_LINK_CHANNEL, packet, OpenDeckMIDIformat::packetType_t::internalCommand);
#endif
#endif
    }

    ///
    /// \brief Calculates CRC of entire flash.
    /// \return True if CRC is valid, that is, if it matches CRC written in last flash address.
    ///
    bool appCRCvalid()
    {
        return true;
        uint16_t crc = 0x0000;

#if (FLASHEND > 0xFFFF)
        uint32_t lastAddress = pgm_read_word(core::CORE_ARCH::pgmGetFarAddress(APP_LENGTH_LOCATION));
#else
        uint32_t lastAddress = pgm_read_word(APP_LENGTH_LOCATION);
#endif

        for (uint32_t i = 0; i < lastAddress; i++)
        {
#if (FLASHEND > 0xFFFF)
            crc = _crc_xmodem_update(crc, pgm_read_byte_far(core::CORE_ARCH::pgmGetFarAddress(i)));
#else
            crc = _crc_xmodem_update(crc, pgm_read_byte(i));
#endif
        }

#if (FLASHEND > 0xFFFF)
        return (crc == pgm_read_word_far(core::CORE_ARCH::pgmGetFarAddress(lastAddress)));
#else
        return (crc == pgm_read_word(lastAddress));
#endif
    }

    ///
    /// \brief Checks if application should be run.
    /// This function performs two checks: hardware and software bootloader entry.
    /// Hardware bootloader entry is possible if the specific board has defined button
    /// which should be pressed before the MCU is turned on. If it is, bootloader is
    /// entered.
    /// Software bootloader entry is possible by writing special value to special EEPROM
    /// address before the application is rebooted.
    /// \returns True if application should be run, false if bootloader should be run.
    ///
    bool checkApplicationRun()
    {
        bool jumpToApplication = false;

        //add some delay before reading the pins to avoid incorrect state detection
        _delay_ms(100);

#if defined(BOARD_KODAMA)
        uint16_t dInData = 0;
        CORE_AVR_PIN_SET_LOW(SR_DIN_CLK_PORT, SR_DIN_CLK_PIN);
        CORE_AVR_PIN_SET_LOW(SR_DIN_LATCH_PORT, SR_DIN_LATCH_PIN);
        _NOP();
        _NOP();

        CORE_AVR_PIN_SET_HIGH(SR_DIN_LATCH_PORT, SR_DIN_LATCH_PIN);

        //this board has two shift registers - 16 inputs in total
        for (int i = 0; i < 16; i++)
        {
            CORE_AVR_PIN_SET_LOW(SR_DIN_CLK_PORT, SR_DIN_CLK_PIN);
            _NOP();
            BIT_WRITE(dInData, i, !CORE_AVR_PIN_READ(SR_DIN_DATA_PORT, SR_DIN_DATA_PIN));
            CORE_AVR_PIN_SET_HIGH(SR_DIN_CLK_PORT, SR_DIN_CLK_PIN);
        }

        bool hardwareTrigger = BIT_READ(dInData, BTLDR_BUTTON_INDEX);
#else
#ifdef BTLDR_BUTTON_PORT
        bool           hardwareTrigger = !CORE_AVR_PIN_READ(BTLDR_BUTTON_PORT, BTLDR_BUTTON_PIN);
#else
        //no hardware entry possible in this case
        bool hardwareTrigger = false;
#endif
#endif

        //check if user wants to enter bootloader
        bool softwareTrigger = eeprom_read_byte((uint8_t*)REBOOT_VALUE_EEPROM_LOCATION) == BTLDR_REBOOT_VALUE;

        //reset value in eeprom after reading
        eeprom_write_byte((uint8_t*)REBOOT_VALUE_EEPROM_LOCATION, APP_REBOOT_VALUE);

        //jump to app only if both software and hardware triggers aren't activated
        if (!hardwareTrigger && !softwareTrigger)
            jumpToApplication = true;

        //don't run the user application if the reset vector is blank (no app loaded)
        bool applicationValid = (pgm_read_word(0) != 0xFFFF) && appCRCvalid();

        return (jumpToApplication && applicationValid);
    }

    ///
    /// \brief Run user application.
    ///
    void runApplication()
    {
        //run app
        // ((void (*)(void))0x0000)();
        __asm__ __volatile__(
            // Jump to RST vector
            "clr r30\n"
            "clr r31\n"
            "ijmp\n");
    }
}    // namespace

namespace bootloader
{
    bool RunBootloader = true;
}

///
/// \brief Main program entry point.
/// This routine configures the hardware required by the bootloader, then continuously
/// runs the bootloader processing routine until instructed to soft-exit.
///
int main(void)
{
    //clear reset source
    MCUSR &= ~(1 << EXTRF);

    //disable watchdog
    MCUSR &= ~(1 << WDRF);
    wdt_disable();

    pins();

    if (checkApplicationRun())
        runApplication();

    //setup hardware required for the bootloader
    setupHardware();

    //enable global interrupts so that the USB stack can function
    GlobalInterruptEnable();

    while (bootloader::RunBootloader)
    {
#ifdef USB_SUPPORTED
        USB_USBTask();
#else
        static uint8_t pageStartCnt = 0;
        uint8_t        data;
        while (!Board::UART::read(UART_USB_LINK_CHANNEL, data))
            ;

        if (pageStartCnt < 6)
        {
            if (data == bootloader::hidUploadStart[pageStartCnt])
            {
                pageStartCnt++;

                if (pageStartCnt == 6)
                {
                    EVENT_UART_Device_ControlRequest();
                    pageStartCnt = 0;
                }
            }
            else
            {
                pageStartCnt = 0;
            }
        }
#endif
    }

#ifdef USB_SUPPORTED
    //disconnect from the host - USB interface will be reset later along with the AVR
    USB_Detach();
#endif

    //enable the watchdog and force a timeout to reset the AVR
    wdt_enable(WDTO_250MS);
    while (1)
        ;
}

#ifdef USB_SUPPORTED
///
/// \brief Event handler for the USB_ConfigurationChanged event.
/// This configures the device's endpoints ready to relay data to and from the attached USB host.
///
void EVENT_USB_Device_ConfigurationChanged(void)
{
    //setup HID Report Endpoint
    Endpoint_ConfigureEndpoint(HID_IN_EPADDR, EP_TYPE_INTERRUPT, HID_IN_EPSIZE, 1);
}
#endif