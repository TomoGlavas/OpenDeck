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

#include "Analog.h"
#include "board/Board.h"
#include "core/src/general/Helpers.h"

using namespace IO;

void Analog::update()
{
    //check values
    for (int i = 0; i < MAX_NUMBER_OF_ANALOG; i++)
    {
        //don't process component if it's not enabled
        if (!database.read(Database::Section::analog_t::enable, i))
            continue;

        uint16_t analogData = Board::io::getAnalogValue(i);
        auto     type       = static_cast<type_t>(database.read(Database::Section::analog_t::type, i));

        if (expFilterUsed)
        {
            //normally use exponential filter (factor 0.5 for easier bitwise math), but not around the edges
            if (analogData <= adcConfig.stepMinDiff7bit)
                analogData = 0;
            else if (analogData >= (adcConfig.adcMaxValue - adcConfig.stepMinDiff7bit))
                analogData = adcConfig.adcMaxValue;
            else
                analogData = (analogData >> 1) + (lastAnalogueValue[i] >> 1);
        }

        if (type != type_t::button)
        {
            switch (type)
            {
            case type_t::potentiometerControlChange:
            case type_t::potentiometerNote:
            case type_t::nrpn7b:
            case type_t::nrpn14b:
            case type_t::pitchBend:
            case type_t::cc14bit:
                checkPotentiometerValue(type, i, analogData);
                break;

            case type_t::fsr:
                checkFSRvalue(i, analogData);
                break;

            default:
                break;
            }
        }
        else
        {
            if (buttonHandler != nullptr)
                (*buttonHandler)(i, digitalStateFromAnalogValue(analogData));
        }
    }
}

void Analog::debounceReset(uint16_t index)
{
    lastDirection[index]     = potDirection_t::initial;
    lastAnalogueValue[index] = 0;
    fsrPressed[index]        = false;
}

///
/// \param fptr [in]    Pointer to function.
///
void Analog::setButtonHandler(void (*fptr)(uint8_t adcIndex, bool state))
{
    buttonHandler = fptr;
}

bool Analog::digitalStateFromAnalogValue(uint16_t adcValue)
{
    //set state to released only if value is below digitalValueThresholdOff
    if (adcValue < adcConfig.digitalValueThresholdOff)
        return false;
    else if (adcValue > adcConfig.digitalValueThresholdOn)
        return true;
    else
        return false;
}

void Analog::enableExpFiltering()
{
    expFilterUsed = true;
}

void Analog::disableExpFiltering()
{
    expFilterUsed = false;
}

IO::Analog::adcConfig_t& Analog::config()
{
    return adcConfig;
}