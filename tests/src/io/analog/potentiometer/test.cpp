#include "unity/src/unity.h"
#include "unity/Helpers.h"
#include "io/analog/Analog.h"
#include "io/leds/LEDs.h"
#include "io/common/CInfo.h"
#include "database/Database.h"
#include "midi/src/MIDI.h"
#include "core/src/general/Timing.h"
#include "core/src/general/Helpers.h"
#include "stubs/database/DB_ReadWrite.h"
#include <vector>

namespace
{
    uint32_t                           messageCounter = 0;
    std::vector<MIDI::USBMIDIpacket_t> midiPacket;

    void resetReceived()
    {
        midiPacket.clear();
        messageCounter = 0;
    }

    bool midiDataHandler(MIDI::USBMIDIpacket_t& USBMIDIpacket)
    {
        midiPacket.push_back(USBMIDIpacket);
        messageCounter++;

        return true;
    }

    class DBhandlers : public Database::Handlers
    {
        public:
        DBhandlers() {}

        void presetChange(uint8_t preset) override
        {
            if (presetChangeHandler != nullptr)
                presetChangeHandler(preset);
        }

        void factoryResetStart() override
        {
            if (factoryResetStartHandler != nullptr)
                factoryResetStartHandler();
        }

        void factoryResetDone() override
        {
            if (factoryResetDoneHandler != nullptr)
                factoryResetDoneHandler();
        }

        void initialized() override
        {
            if (initHandler != nullptr)
                initHandler();
        }

        //actions which these handlers should take depend on objects making
        //up the entire system to be initialized
        //therefore in interface we are calling these function pointers which
        // are set in application once we have all objects ready
        void (*presetChangeHandler)(uint8_t preset) = nullptr;
        void (*factoryResetStartHandler)()          = nullptr;
        void (*factoryResetDoneHandler)()           = nullptr;
        void (*initHandler)()                       = nullptr;
    } dbHandlers;

    class HWALEDs : public IO::LEDs::HWA
    {
        public:
        HWALEDs() {}

        void setState(size_t index, bool state) override
        {
        }

        size_t rgbSingleComponentIndex(size_t rgbIndex, IO::LEDs::rgbIndex_t rgbComponent) override
        {
            return 0;
        }

        size_t rgbIndex(size_t singleLEDindex) override
        {
            return 0;
        }

        void setFadeSpeed(size_t transitionSpeed) override
        {
        }
    } ledsHWA;

    class HWAAnalog : public IO::Analog::HWA
    {
        public:
        HWAAnalog() {}

        uint16_t state(size_t index) override
        {
            return adcReturnValue;
        }

        uint32_t adcReturnValue;
    } hwaAnalog;

    DBstorageMock dbStorageMock;
    Database      database = Database(dbHandlers, dbStorageMock);
    MIDI          midi;
    ComponentInfo cInfo;

    IO::LEDs leds(ledsHWA, database);

#ifdef DISPLAY_SUPPORTED
    class HWAU8X8 : public IO::U8X8::HWAI2C
    {
        public:
        HWAU8X8() {}

        void init() override
        {
        }

        bool transfer(uint8_t address, IO::U8X8::HWAI2C::transferType_t type) override
        {
            return true;
        }

        void stop() override
        {
        }

        bool write(uint8_t data) override
        {
            return true;
        }
    } hwaU8X8;

    IO::U8X8    u8x8(hwaU8X8);
    IO::Display display(u8x8, database);
#endif

#ifdef DISPLAY_SUPPORTED
#ifdef ADC_10_BIT
    IO::Analog analog(hwaAnalog, IO::Analog::adcType_t::adc10bit, database, midi, leds, display, cInfo);
#else
    IO::Analog analog(hwaAnalog, IO::Analog::adcType_t::adc12bit, database, midi, leds, display, cInfo);
#endif
#else
#ifdef ADC_10_BIT
    IO::Analog analog(hwaAnalog, IO::Analog::adcType_t::adc10bit, database, midi, leds, cInfo);
#else
    IO::Analog analog(hwaAnalog, IO::Analog::adcType_t::adc12bit, database, midi, leds, cInfo);
#endif
#endif
}    // namespace

TEST_SETUP()
{
    //init checks - no point in running further tests if these conditions fail
    TEST_ASSERT(database.init() == true);
    TEST_ASSERT(database.isSignatureValid() == true);
    TEST_ASSERT(database.factoryReset(LESSDB::factoryResetType_t::full) == true);
    midi.handleUSBwrite(midiDataHandler);

    for (int i = 0; i < MAX_NUMBER_OF_ANALOG; i++)
        analog.debounceReset(i);

    analog.disableExpFiltering();

    resetReceived();
}

TEST_CASE(CCtest)
{
    using namespace IO;

    //set known state
    for (int i = 0; i < MAX_NUMBER_OF_ANALOG; i++)
    {
        //enable all analog components
        TEST_ASSERT(database.update(Database::Section::analog_t::enable, i, 1) == true);

        //disable invert state
        TEST_ASSERT(database.update(Database::Section::analog_t::invert, i, 0) == true);

        //configure all analog components as potentiometers with CC MIDI message
        TEST_ASSERT(database.update(Database::Section::analog_t::type, i, static_cast<int32_t>(Analog::type_t::potentiometerControlChange)) == true);

        //set all lower limits to 0
        TEST_ASSERT(database.update(Database::Section::analog_t::lowerLimit, i, 0) == true);

        //set all upper limits to MIDI_7_BIT_VALUE_MAX
        TEST_ASSERT(database.update(Database::Section::analog_t::upperLimit, i, MIDI_7_BIT_VALUE_MAX) == true);

        //midi channel
        TEST_ASSERT(database.update(Database::Section::analog_t::midiChannel, i, 1) == true);
    }

    //feed all the values from minimum to maximum
    //expected result should be all MIDI values received (0-127)
    auto     adcConfig        = analog.config();
    uint32_t expectedMessages = MAX_NUMBER_OF_ANALOG * 128;

    for (int i = 0; i <= adcConfig.adcMaxValue; i++)
    {
        hwaAnalog.adcReturnValue = i;
        analog.update();
    }

    TEST_ASSERT_EQUAL_UINT32(messageCounter, expectedMessages);

    //all received messages should be control change
    for (int i = 0; i < midiPacket.size(); i++)
        TEST_ASSERT_EQUAL_UINT32(static_cast<uint8_t>(MIDI::messageType_t::controlChange), midiPacket.at(i).Event << 4);

    //verify that all values have been sent in order (forward)
    for (int i = 0; i < 128; i++)
    {
        for (int j = 0; j < MAX_NUMBER_OF_ANALOG; j++)
        {
            size_t index = i * MAX_NUMBER_OF_ANALOG + j;
            TEST_ASSERT_EQUAL_UINT32(i, midiPacket.at(index).Data3);
        }
    }

    //try to update it again without changing values, nothing should change
    analog.update();
    TEST_ASSERT_EQUAL_UINT32(expectedMessages, messageCounter);

    //now go backward

    resetReceived();

    for (int i = adcConfig.adcMaxValue; i >= 0; i--)
    {
        hwaAnalog.adcReturnValue = i;
        analog.update();
    }

    //one message less (max/127 is already sent)
    expectedMessages -= MAX_NUMBER_OF_ANALOG;

    TEST_ASSERT_EQUAL_UINT32(expectedMessages, messageCounter);

    //verify that all values have been sent in order (backward)
    for (int i = 0; i < 127; i++)
    {
        for (int j = 0; j < MAX_NUMBER_OF_ANALOG; j++)
        {
            size_t index = i * MAX_NUMBER_OF_ANALOG + j;
            TEST_ASSERT_EQUAL_UINT32(126 - i, midiPacket.at(index).Data3);
        }
    }

    //try to update it again without changing values, nothing should change
    analog.update();
    TEST_ASSERT_EQUAL_UINT32(expectedMessages, messageCounter);
}

TEST_CASE(PitchBendTest)
{
    using namespace IO;

    //set known state
    for (int i = 0; i < MAX_NUMBER_OF_ANALOG; i++)
    {
        //enable all analog components
        TEST_ASSERT(database.update(Database::Section::analog_t::enable, i, 1) == true);

        //disable invert state
        TEST_ASSERT(database.update(Database::Section::analog_t::invert, i, 0) == true);

        //configure all analog components as potentiometers with Pitch Bend MIDI message
        TEST_ASSERT(database.update(Database::Section::analog_t::type, i, static_cast<int32_t>(Analog::type_t::pitchBend)) == true);

        //set all lower limits to 0
        TEST_ASSERT(database.update(Database::Section::analog_t::lowerLimit, i, 0) == true);

        //set all upper limits to MIDI_14_BIT_VALUE_MAX
        TEST_ASSERT(database.update(Database::Section::analog_t::upperLimit, i, MIDI_14_BIT_VALUE_MAX) == true);

        //midi channel
        TEST_ASSERT(database.update(Database::Section::analog_t::midiChannel, i, 1) == true);
    }

    //feed all the values from minimum to maximum
    //here, ADC resolution should be taken into account:
    //ADC resolution is smaller than the 14-bit bit Pitch Bend values
    //application may use some debouncing techniques so perform only best-effort checks here

    auto     adcConfig        = analog.config();
    uint32_t valueDiff        = 16384 / (adcConfig.adcMaxValue + 1);
    uint32_t totalMIDIvalues  = 16384 / valueDiff / 2;    //don't expect the value to change on every change of raw value
    uint32_t expectedMessages = MAX_NUMBER_OF_ANALOG * totalMIDIvalues;

    for (int i = 0; i <= adcConfig.adcMaxValue; i++)
    {
        hwaAnalog.adcReturnValue = i;
        analog.update();
    }

    TEST_ASSERT(messageCounter >= expectedMessages);

    //all received messages should be pitch bend
    for (int i = 0; i < midiPacket.size(); i++)
        TEST_ASSERT_EQUAL_UINT32(midiPacket.at(i).Event << 4, static_cast<uint8_t>(MIDI::messageType_t::pitchBend));

    //since numbers in this case are scaled from lower to upper range,
    //verify that
    //1) first value is 0
    //2) each next value is larger from previous
    //3) last value is 16383
    uint32_t previousPitchBendValue = 0;

    uint32_t receivedMessages = midiPacket.size() / MAX_NUMBER_OF_ANALOG;

    for (int i = 0; i < receivedMessages; i++)
    {
        MIDI::encDec_14bit_t pitchBendValue;

        for (int j = 0; j < MAX_NUMBER_OF_ANALOG; j++)
        {
            size_t index = i * MAX_NUMBER_OF_ANALOG + j;

            pitchBendValue.low  = midiPacket.at(index).Data2;
            pitchBendValue.high = midiPacket.at(index).Data3;
            pitchBendValue.mergeTo14bit();

            if (!i)
                TEST_ASSERT_EQUAL_UINT32(0, pitchBendValue.value);
            else
                TEST_ASSERT(pitchBendValue.value > previousPitchBendValue);

            if (i == (receivedMessages - 1))
                TEST_ASSERT_EQUAL_UINT32(16383, pitchBendValue.value);
        }

        previousPitchBendValue = pitchBendValue.value;
    }

    // try to update it again without changing values, nothing should change
    uint32_t previousCount = messageCounter;
    analog.update();
    TEST_ASSERT_EQUAL_UINT32(previousCount, messageCounter);

    //now go backward
    resetReceived();

    for (int i = adcConfig.adcMaxValue; i >= 0; i--)
    {
        hwaAnalog.adcReturnValue = i;
        analog.update();
    }

    //similar to previous case, verify that:
    //every next value is smaller than previous
    //last value is 0

    receivedMessages       = midiPacket.size() / MAX_NUMBER_OF_ANALOG;
    previousPitchBendValue = 16383;

    for (int i = 0; i < receivedMessages; i++)
    {
        MIDI::encDec_14bit_t pitchBendValue;

        for (int j = 0; j < MAX_NUMBER_OF_ANALOG; j++)
        {
            volatile size_t index = i * MAX_NUMBER_OF_ANALOG + j;

            pitchBendValue.low  = midiPacket.at(index).Data2;
            pitchBendValue.high = midiPacket.at(index).Data3;
            pitchBendValue.mergeTo14bit();

            TEST_ASSERT(pitchBendValue.value < previousPitchBendValue);

            if (i == (receivedMessages - 1))
                TEST_ASSERT_EQUAL_UINT32(0, pitchBendValue.value);
        }

        previousPitchBendValue = pitchBendValue.value;
    }

    // try to update it again without changing values, nothing should change
    previousCount = messageCounter;
    analog.update();
    TEST_ASSERT_EQUAL_UINT32(previousCount, messageCounter);
}

TEST_CASE(Inversion)
{
    using namespace IO;

    //set known state
    for (int i = 0; i < MAX_NUMBER_OF_ANALOG; i++)
    {
        //enable all analog components
        TEST_ASSERT(database.update(Database::Section::analog_t::enable, i, 1) == true);

        //disable invert state
        TEST_ASSERT(database.update(Database::Section::analog_t::invert, i, 0) == true);

        //configure all analog components as potentiometers with CC MIDI message
        TEST_ASSERT(database.update(Database::Section::analog_t::type, i, static_cast<int32_t>(Analog::type_t::potentiometerControlChange)) == true);

        //set all lower limits to 0
        TEST_ASSERT(database.update(Database::Section::analog_t::lowerLimit, i, 0) == true);

        //set all upper limits to MIDI_14_BIT_VALUE_MAX
        TEST_ASSERT(database.update(Database::Section::analog_t::upperLimit, i, MIDI_14_BIT_VALUE_MAX) == true);

        //midi channel
        TEST_ASSERT(database.update(Database::Section::analog_t::midiChannel, i, 1) == true);
    }

    auto adcConfig           = analog.config();
    hwaAnalog.adcReturnValue = adcConfig.adcMaxValue;
    uint32_t previousValue;

    for (int i = 0; i <= adcConfig.adcMaxValue; i++)
    {
        hwaAnalog.adcReturnValue = i;
        analog.update();
    }

    previousValue = 0;

    //verify that every received value is larger than the previous
    //first value should be 0
    //last value should be 127
    for (int i = 0; i < 128; i++)
    {
        for (int j = 0; j < MAX_NUMBER_OF_ANALOG; j++)
        {
            size_t index = i * MAX_NUMBER_OF_ANALOG + j;

            if (!i)
                TEST_ASSERT_EQUAL_UINT32(0, midiPacket.at(index).Data3);
            else
                TEST_ASSERT(midiPacket.at(index).Data3 > previousValue);

            if (i == 127)
                TEST_ASSERT_EQUAL_UINT32(127, midiPacket.at(index).Data3);
        }

        previousValue = i;
    }

    //now enable inversion
    for (int i = 0; i < MAX_NUMBER_OF_ANALOG; i++)
    {
        TEST_ASSERT(database.update(Database::Section::analog_t::invert, i, 1) == true);
        analog.debounceReset(i);
    }

    resetReceived();

    //feed all the values again
    for (int i = 0; i <= adcConfig.adcMaxValue; i++)
    {
        hwaAnalog.adcReturnValue = i;
        analog.update();
    }

    //verify that every received value is smaller than the previous
    //first value should be 127
    //last value should be 0
    for (int i = 0; i < 128; i++)
    {
        size_t index;

        for (int j = 0; j < MAX_NUMBER_OF_ANALOG; j++)
        {
            index = i * MAX_NUMBER_OF_ANALOG + j;

            if (!i)
                TEST_ASSERT_EQUAL_UINT32(127, midiPacket.at(index).Data3);
            else
                TEST_ASSERT(midiPacket.at(index).Data3 < previousValue);

            if (i == 127)
                TEST_ASSERT_EQUAL_UINT32(0, midiPacket.at(index).Data3);
        }

        previousValue = midiPacket.at(index).Data3;
    }

    resetReceived();

    //funky setup: set lower limit to 127, upper to 0 while inversion is enabled
    //result should be the same as when default setup is used (no inversion/ 0 as lower limit, 127 as upper limit)

    for (int i = 0; i < MAX_NUMBER_OF_ANALOG; i++)
    {
        TEST_ASSERT(database.update(Database::Section::analog_t::invert, i, 1) == true);
        TEST_ASSERT(database.update(Database::Section::analog_t::lowerLimit, i, 127) == true);
        TEST_ASSERT(database.update(Database::Section::analog_t::upperLimit, i, 0) == true);
        analog.debounceReset(i);
    }

    //feed all the values again
    for (int i = 0; i <= adcConfig.adcMaxValue; i++)
    {
        hwaAnalog.adcReturnValue = i;
        analog.update();
    }

    previousValue = 0;

    //verify that every received value is larger than the previous
    //first value should be 0
    //last value should be 127
    for (int i = 0; i < 128; i++)
    {
        for (int j = 0; j < MAX_NUMBER_OF_ANALOG; j++)
        {
            size_t index = i * MAX_NUMBER_OF_ANALOG + j;

            if (!i)
                TEST_ASSERT_EQUAL_UINT32(0, midiPacket.at(index).Data3);
            else
                TEST_ASSERT(midiPacket.at(index).Data3 > previousValue);

            if (i == 127)
                TEST_ASSERT_EQUAL_UINT32(127, midiPacket.at(index).Data3);
        }

        previousValue = i;
    }

    //now disable inversion
    resetReceived();

    for (int i = 0; i < MAX_NUMBER_OF_ANALOG; i++)
    {
        TEST_ASSERT(database.update(Database::Section::analog_t::invert, i, 0) == true);
        analog.debounceReset(i);
    }

    //feed all the values again
    for (int i = 0; i <= adcConfig.adcMaxValue; i++)
    {
        hwaAnalog.adcReturnValue = i;
        analog.update();
    }

    previousValue = 0;

    //verify that every received value is smaller than the previous
    //first value should be 127
    //last value should be 0
    for (int i = 0; i < 128; i++)
    {
        size_t index;

        for (int j = 0; j < MAX_NUMBER_OF_ANALOG; j++)
        {
            index = i * MAX_NUMBER_OF_ANALOG + j;

            if (!i)
                TEST_ASSERT_EQUAL_UINT32(127, midiPacket.at(index).Data3);
            else
                TEST_ASSERT(midiPacket.at(index).Data3 < previousValue);

            if (i == 127)
                TEST_ASSERT_EQUAL_UINT32(0, midiPacket.at(index).Data3);
        }

        previousValue = midiPacket.at(index).Data3;
    }
}

TEST_CASE(Scaling)
{
    using namespace IO;

    const uint32_t scaledUpperLower = 11;
    const uint32_t scaledUpperValue = 100;

    //set known state
    for (int i = 0; i < MAX_NUMBER_OF_ANALOG; i++)
    {
        //enable all analog components
        TEST_ASSERT(database.update(Database::Section::analog_t::enable, i, 1) == true);

        //disable invert state
        TEST_ASSERT(database.update(Database::Section::analog_t::invert, i, 0) == true);

        //configure all analog components as potentiometers with CC MIDI message
        TEST_ASSERT(database.update(Database::Section::analog_t::type, i, static_cast<int32_t>(Analog::type_t::potentiometerControlChange)) == true);

        //set all lower limits to 0
        TEST_ASSERT(database.update(Database::Section::analog_t::lowerLimit, i, 0) == true);

        //set all upper limits to 100
        TEST_ASSERT(database.update(Database::Section::analog_t::upperLimit, i, scaledUpperValue) == true);

        //midi channel
        TEST_ASSERT(database.update(Database::Section::analog_t::midiChannel, i, 1) == true);
    }

    auto adcConfig           = analog.config();
    hwaAnalog.adcReturnValue = adcConfig.adcMaxValue;
    uint32_t previousValue;

    for (int i = 0; i <= adcConfig.adcMaxValue; i++)
    {
        hwaAnalog.adcReturnValue = i;
        analog.update();
    }

    previousValue = 0;

    //first values should be 0
    //last value should match the configured scaled value (scaledUpperValue)
    for (int i = 0; i < MAX_NUMBER_OF_ANALOG; i++)
    {
        TEST_ASSERT_EQUAL_UINT32(0, midiPacket.at(i).Data3);
        TEST_ASSERT_EQUAL_UINT32(scaledUpperValue, midiPacket.at(midiPacket.size() - MAX_NUMBER_OF_ANALOG + i).Data3);
    }

    //now scale minimum value as well
    for (int i = 0; i < MAX_NUMBER_OF_ANALOG; i++)
        TEST_ASSERT(database.update(Database::Section::analog_t::lowerLimit, i, scaledUpperLower) == true);

    resetReceived();

    for (int i = 0; i <= adcConfig.adcMaxValue; i++)
    {
        hwaAnalog.adcReturnValue = i;
        analog.update();
    }

    for (int i = 0; i < MAX_NUMBER_OF_ANALOG; i++)
    {
        TEST_ASSERT_EQUAL_UINT32(scaledUpperLower, midiPacket.at(i).Data3);
        TEST_ASSERT_EQUAL_UINT32(scaledUpperValue, midiPacket.at(midiPacket.size() - MAX_NUMBER_OF_ANALOG + i).Data3);
    }

    //now enable inversion
    for (int i = 0; i < MAX_NUMBER_OF_ANALOG; i++)
    {
        TEST_ASSERT(database.update(Database::Section::analog_t::invert, i, 1) == true);
        analog.debounceReset(i);
    }

    resetReceived();

    for (int i = 0; i <= adcConfig.adcMaxValue; i++)
    {
        hwaAnalog.adcReturnValue = i;
        analog.update();
    }

    //first values should be scaledUpperValue
    //last value should match the configured scaled value (scaledUpperLower)
    for (int i = 0; i < MAX_NUMBER_OF_ANALOG; i++)
    {
        TEST_ASSERT_EQUAL_UINT32(scaledUpperValue, midiPacket.at(i).Data3);
        TEST_ASSERT_EQUAL_UINT32(scaledUpperLower, midiPacket.at(midiPacket.size() - MAX_NUMBER_OF_ANALOG + i).Data3);
    }
}