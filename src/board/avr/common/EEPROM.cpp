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

#include <avr/eeprom.h>
#include "board/Board.h"

namespace Board
{
    namespace eeprom
    {
        bool read(uint32_t address, LESSDB::sectionParameterType_t type, int32_t& value)
        {
            switch (type)
            {
            case LESSDB::sectionParameterType_t::bit:
            case LESSDB::sectionParameterType_t::byte:
            case LESSDB::sectionParameterType_t::halfByte:
                value = eeprom_read_byte(reinterpret_cast<uint8_t*>(address));
                break;

            case LESSDB::sectionParameterType_t::word:
                value = eeprom_read_word(reinterpret_cast<uint16_t*>(address));
                break;

            default:
                // case LESSDB::sectionParameterType_t::dword:
                value = eeprom_read_dword(reinterpret_cast<uint32_t*>(address));
                break;
            }

            return true;
        }

        bool write(uint32_t address, int32_t value, LESSDB::sectionParameterType_t type)
        {
            switch (type)
            {
            case LESSDB::sectionParameterType_t::bit:
            case LESSDB::sectionParameterType_t::byte:
            case LESSDB::sectionParameterType_t::halfByte:
                eeprom_update_byte(reinterpret_cast<uint8_t*>(address), value);
                break;

            case LESSDB::sectionParameterType_t::word:
                eeprom_update_word(reinterpret_cast<uint16_t*>(address), value);
                break;

            default:
                // case LESSDB::sectionParameterType_t::dword:
                eeprom_update_dword(reinterpret_cast<uint32_t*>(address), value);
                break;
            }

            return true;
        }
    }    // namespace eeprom
}    // namespace Board
