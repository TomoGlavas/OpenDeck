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

#include "board/Board.h"
#include "board/Internal.h"
#include "common/OpenDeckMIDIformat/OpenDeckMIDIformat.h"
#include "core/src/general/Reset.h"
#include "core/src/general/Timing.h"

namespace
{
    ///
    /// \brief Placeholder variable used only to reserve space in linker section.
    ///
    const uint32_t appLength __attribute__((section(".applen"))) __attribute__((used)) = 0;
}    // namespace

namespace core
{
    namespace timing
    {
        namespace detail
        {
            ///
            /// \brief Implementation of core variable used to keep track of run time in milliseconds.
            ///
            volatile uint32_t rTime_ms;
        }    // namespace detail
    }        // namespace timing
}    // namespace core

namespace Board
{
    void reboot(rebootType_t type)
    {
        switch (type)
        {
        case rebootType_t::rebootApp:
            detail::bootloader::clearSWtrigger();
#ifndef USB_MIDI_SUPPORTED
            //signal to usb link to reboot as well
            //no need to do this for bootloader reboot - the bootloader already sends btldrReboot command to USB link
            MIDI::USBMIDIpacket_t USBMIDIpacket;

            USBMIDIpacket.Event = static_cast<uint8_t>(OpenDeckMIDIformat::command_t::appReboot);
            USBMIDIpacket.Data1 = 0x00;
            USBMIDIpacket.Data2 = 0x00;
            USBMIDIpacket.Data3 = 0x00;

            OpenDeckMIDIformat::write(UART_USB_LINK_CHANNEL, USBMIDIpacket, OpenDeckMIDIformat::packetType_t::internalCommand);
            while (!Board::UART::isTxEmpty(UART_USB_LINK_CHANNEL))
                ;

            //give some time to usb link to properly re-initialize so that everything is in sync
            core::timing::waitMs(50);
#endif
            break;

        case rebootType_t::rebootBtldr:
            detail::bootloader::enableSWtrigger();
            break;
        }

        core::reset::mcuReset();
    }

    namespace detail
    {
        void errorHandler()
        {
            while (1)
            {
            }
        }
    }    // namespace detail
}    // namespace Board
