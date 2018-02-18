/*
    OpenDeck MIDI platform firmware
    Copyright (C) 2015-2017 Igor Petrovic

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "SysExConf.h"
#include "../interface/Interface.h"

void SysExConfig::createLayout()
{
    //last db block is ID block which isn't needed
    addBlocks(DB_BLOCKS-1);

    sysExSection section;

    {
        //MIDI block

        //midi feature section
        section.numberOfParameters = MIDI_FEATURES;
        section.minValue = 0;
        section.maxValue = 1;

        addSection(DB_BLOCK_MIDI, section);

        //midi channel section
        section.numberOfParameters = MIDI_CHANNELS;
        section.minValue = 1;
        section.maxValue = 16;

        addSection(DB_BLOCK_MIDI, section);
    }

    {
        //button block

        //type section
        section.numberOfParameters = MAX_NUMBER_OF_BUTTONS+MAX_NUMBER_OF_ANALOG;
        section.minValue = 0;
        section.maxValue = BUTTON_TYPES-1;

        addSection(DB_BLOCK_BUTTON, section);

        //midi message type section
        section.numberOfParameters = MAX_NUMBER_OF_BUTTONS+MAX_NUMBER_OF_ANALOG;
        section.minValue = 0;
        section.maxValue = BUTTON_MESSAGE_TYPES-1;

        addSection(DB_BLOCK_BUTTON, section);

        //midi id section
        section.numberOfParameters = MAX_NUMBER_OF_BUTTONS+MAX_NUMBER_OF_ANALOG;
        section.minValue = 0;
        section.maxValue = 127;

        addSection(DB_BLOCK_BUTTON, section);

        //midi velocity section
        section.numberOfParameters = MAX_NUMBER_OF_BUTTONS+MAX_NUMBER_OF_ANALOG;
        section.minValue = 1;
        section.maxValue = 127;

        addSection(DB_BLOCK_BUTTON, section);
    }

    {
        //encoder block

        //encoder enabled section
        section.numberOfParameters = MAX_NUMBER_OF_ENCODERS;
        section.minValue = 0;
        section.maxValue = 1;

        addSection(DB_BLOCK_ENCODER, section);

        //encoder inverted section
        section.numberOfParameters = MAX_NUMBER_OF_ENCODERS;
        section.minValue = 0;
        section.maxValue = 1;

        addSection(DB_BLOCK_ENCODER, section);

        //encoding mode section
        section.numberOfParameters = MAX_NUMBER_OF_ENCODERS;
        section.minValue = 0;
        section.maxValue = ENCODING_MODES-1;

        addSection(DB_BLOCK_ENCODER, section);

        //midi id section
        section.numberOfParameters = MAX_NUMBER_OF_ENCODERS;
        section.minValue = 0;
        section.maxValue = 127;

        addSection(DB_BLOCK_ENCODER, section);
    }

    {
        //analog block

        //analog enabled section
        section.numberOfParameters = MAX_NUMBER_OF_ANALOG;
        section.minValue = 0;
        section.maxValue = 1;

        addSection(DB_BLOCK_ANALOG, section);

        //analog inverted section
        section.numberOfParameters = MAX_NUMBER_OF_ANALOG;
        section.minValue = 0;
        section.maxValue = 1;

        addSection(DB_BLOCK_ANALOG, section);

        //analog type section
        section.numberOfParameters = MAX_NUMBER_OF_ANALOG;
        section.minValue = 0;
        section.maxValue = ANALOG_TYPES-1;

        addSection(DB_BLOCK_ANALOG, section);

        //midi id section, lsb
        section.numberOfParameters = MAX_NUMBER_OF_ANALOG;
        section.minValue = 0;
        section.maxValue = 127;

        addSection(DB_BLOCK_ANALOG, section);

        //midi id section, msb
        section.numberOfParameters = MAX_NUMBER_OF_ANALOG;
        section.minValue = 0;
        section.maxValue = 127;

        addSection(DB_BLOCK_ANALOG, section);

        //lower cc limit, lsb
        section.numberOfParameters = MAX_NUMBER_OF_ANALOG;
        section.minValue = 0;
        section.maxValue = 127;

        addSection(DB_BLOCK_ANALOG, section);

        //lower cc limit, msb
        section.numberOfParameters = MAX_NUMBER_OF_ANALOG;
        section.minValue = 0;
        section.maxValue = 127;

        addSection(DB_BLOCK_ANALOG, section);

        //upper cc limit, lsb
        section.numberOfParameters = MAX_NUMBER_OF_ANALOG;
        section.minValue = 0;
        section.maxValue = 127;

        addSection(DB_BLOCK_ANALOG, section);

        //upper cc limit, msb
        section.numberOfParameters = MAX_NUMBER_OF_ANALOG;
        section.minValue = 0;
        section.maxValue = 127;

        addSection(DB_BLOCK_ANALOG, section);
    }

    {
        //led block

        //hardware parameters section
        section.numberOfParameters = LED_HARDWARE_PARAMETERS;
        section.minValue = 0;
        section.maxValue = 0;

        addSection(DB_BLOCK_LED, section);

        //activation note section
        section.numberOfParameters = MAX_NUMBER_OF_LEDS;
        section.minValue = 0;
        section.maxValue = 127;

        addSection(DB_BLOCK_LED, section);

        //rgb enabled section
        section.numberOfParameters = MAX_NUMBER_OF_LEDS;
        section.minValue = 0;
        section.maxValue = 1;

        addSection(DB_BLOCK_LED, section);

        //local led control enabled section
        section.numberOfParameters = MAX_NUMBER_OF_LEDS;
        section.minValue = 0;
        section.maxValue = 1;

        addSection(DB_BLOCK_LED, section);

        //single led velocity value section
        section.numberOfParameters = MAX_NUMBER_OF_LEDS;
        section.minValue = 1;
        section.maxValue = 127;

        addSection(DB_BLOCK_LED, section);

        //led color section
        section.numberOfParameters = MAX_NUMBER_OF_LEDS;
        section.minValue = 0;
        section.maxValue = LED_COLORS-1;

        addSection(DB_BLOCK_LED, section);

        //led blink section
        section.numberOfParameters = MAX_NUMBER_OF_LEDS;
        section.minValue = 0;
        section.maxValue = 1;

        addSection(DB_BLOCK_LED, section);
    }

    {
        //display block
        //features section
        section.numberOfParameters = DISPLAY_FEATURES;
        section.minValue = 0;
        section.maxValue = DISPLAY_FEATURES-1;

        addSection(DB_BLOCK_DISPLAY, section);

        //controller section
        section.numberOfParameters = DISPLAY_CONTROLLERS;
        section.minValue = 0;
        section.maxValue = DISPLAY_CONTROLLERS-1;

        addSection(DB_BLOCK_DISPLAY, section);

        //resolution section
        section.numberOfParameters = DISPLAY_RESOLUTIONS;
        section.minValue = 0;
        section.maxValue = DISPLAY_RESOLUTIONS-1;

        addSection(DB_BLOCK_DISPLAY, section);
    }
}