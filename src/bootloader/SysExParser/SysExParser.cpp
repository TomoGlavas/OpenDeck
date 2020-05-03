#include "SysExParser.h"
#include "application/OpenDeck/sysconfig/Constants.h"
#include "bootloader/Config.h"

bool SysExParser::isValidMessage(MIDI::USBMIDIpacket_t& packet)
{
    if (parse(packet))
        return verify();

    return false;
}

bool SysExParser::parse(MIDI::USBMIDIpacket_t& packet)
{
    //MIDIEvent.Event is CIN, see midi10.pdf
    //shift cin four bytes left to get message type
    uint8_t midiMessage = packet.Event << 4;

    switch (midiMessage)
    {
    case static_cast<uint8_t>(usbMIDIsystemCin_t::sysCommon1byteCin):
    case static_cast<uint8_t>(usbMIDIsystemCin_t::singleByte):
        if (packet.Data1 == 0xF7)
        {
            //end of sysex
            sysexArray[sysExArrayLength] = packet.Data1;
            sysExArrayLength++;
            return true;
        }
        break;

    case static_cast<uint8_t>(usbMIDIsystemCin_t::sysExStartCin):
        //the message can be any length between 3 and MIDI_SYSEX_ARRAY_SIZE
        if (packet.Data1 == 0xF0)
            sysExArrayLength = 0;    //this is a new sysex message, reset length

        sysexArray[sysExArrayLength] = packet.Data1;
        sysExArrayLength++;
        sysexArray[sysExArrayLength] = packet.Data2;
        sysExArrayLength++;
        sysexArray[sysExArrayLength] = packet.Data3;
        sysExArrayLength++;
        return false;
        break;

    case static_cast<uint8_t>(usbMIDIsystemCin_t::sysExStop2byteCin):
        sysexArray[sysExArrayLength] = packet.Data1;
        sysExArrayLength++;
        sysexArray[sysExArrayLength] = packet.Data2;
        sysExArrayLength++;
        return true;
        break;

    case static_cast<uint8_t>(usbMIDIsystemCin_t::sysExStop3byteCin):
        if (packet.Data1 == 0xF0)
            sysExArrayLength = 0;    //sysex message with 1 byte of payload
        sysexArray[sysExArrayLength] = packet.Data1;
        sysExArrayLength++;
        sysexArray[sysExArrayLength] = packet.Data2;
        sysExArrayLength++;
        sysexArray[sysExArrayLength] = packet.Data3;
        sysExArrayLength++;
        return true;
        break;

    default:
        return false;
        break;
    }

    return false;
}

size_t SysExParser::dataBytes()
{
    if (!verify())
        return 0;

    return (sysExArrayLength - 2 - 3) / 2;
}

bool SysExParser::value(size_t index, uint8_t& data)
{
    size_t arrayIndex = SYSEX_FW_DATA_START_BYTE + index * 2;

    if ((arrayIndex + 1) >= sysExArrayLength)
        return false;

    uint16_t data16;

    mergeTo14bit(data16, sysexArray[arrayIndex], sysexArray[arrayIndex + 1]);

    data = data16 & 0xFF;

    return true;
}

bool SysExParser::verify()
{
    if (sysexArray[1] != SYSEX_MANUFACTURER_ID_0)
        return false;

    if (sysexArray[2] != SYSEX_MANUFACTURER_ID_1)
        return false;

    if (sysexArray[3] != SYSEX_MANUFACTURER_ID_2)
        return false;

    //firmware sysex message should contain at least:
    //start byte
    //three ID bytes
    //two data bytes
    //stop byte
    if (sysExArrayLength < 7)
        return false;

    return true;
}

///
/// \brief Convert 7-bit high and low bytes to single 14-bit value.
/// @param [in,out] value Resulting 14-bit value.
/// @param [in,out] high    Higher 7 bits.
/// @param [in,out] low     Lower 7 bits.
///
void SysExParser::mergeTo14bit(uint16_t& value, uint8_t high, uint8_t low)
{
    if (high & 0x01)
        low |= (1 << 7);
    else
        low &= ~(1 << 7);

    high >>= 1;

    uint16_t joined;

    joined = high;
    joined <<= 8;
    joined |= low;

    value = joined;
}