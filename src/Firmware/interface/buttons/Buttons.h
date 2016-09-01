#ifndef BUTTONS_H_
#define BUTTONS_H_

#include <avr/io.h>
#include "../../hardware/core/Core.h"

class Buttons {

    public:
    Buttons();

    void update();

    private:
    //variables
    uint8_t     previousButtonState[MAX_NUMBER_OF_BUTTONS/8+1],
                buttonPressed[MAX_NUMBER_OF_BUTTONS/8+1],
                buttonDebounceCounter[MAX_NUMBER_OF_BUTTONS];

    //data processing
    bool getButtonPressed(uint8_t buttonID);
    void setButtonPressed(uint8_t buttonID, bool state);
    void processMomentaryButton(uint8_t buttonID, bool buttonState);
    void processLatchingButton(uint8_t buttonID, bool buttonState);
    void processProgramChange(uint8_t buttonID, bool buttonState);
    void updateButtonState(uint8_t buttonID, uint8_t buttonState);
    bool getPreviousButtonState(uint8_t buttonID);
    bool buttonDebounced(uint8_t buttonID, bool buttonState);

};

extern Buttons buttons;

#endif
