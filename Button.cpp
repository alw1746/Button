#include <Arduino.h>
#include "Button.h"

#define BIT_CURRENT         0
#define BIT_PREVIOUS        1
#define BIT_CHANGED         2
#define BIT_HOLD_TRIGGERED  3
#define BIT_HOLD_NOW        4
#define BIT_TEST_MODE       5
#define BIT_TEST_PRESSED    6

Button::Button(uint8_t buttonPin, uint8_t buttonMode, uint16_t _debounceDuration)
{
    m_handlers = 0;
    init(buttonPin, buttonMode, _debounceDuration);
}

bool Button::queryButtonDown() const
{
    int pinState = digitalRead(m_myPin);
    bool down = false;
#ifdef __STM32F1__
    if (m_mode == PULL_DOWN ||m_mode == INTERNAL_PULLDOWN ) {
#else
    if (m_mode == PULL_DOWN) {
#endif
        down = (pinState == HIGH);
    }
    else {
        // PULL_UP or INTERNAL_PULLUP
        down = (pinState == LOW);
    }
    return down;
}

void Button::init(uint8_t buttonPin, uint8_t buttonMode, uint16_t _debounceDuration)
{
    m_holdRepeats = false;
    m_nHolds = 0;
    m_myPin = buttonPin;
    m_mode = buttonMode;
    m_state = 0;
    m_holdEventThreshold = DEFAULT_HOLD_TIME;
    m_debounceDuration = _debounceDuration;
    m_debounceStartTime = 0;
    m_pressedStartTime = 0;

    if (m_myPin != 255) {
        if (m_mode == INTERNAL_PULLUP) {
            pinMode(m_myPin, INPUT_PULLUP);
        }
#ifdef __STM32F1__
        else if (m_mode == INTERNAL_PULLDOWN) {
            pinMode(m_myPin, INPUT_PULLDOWN);
        }
#endif
        else {
            pinMode(m_myPin, INPUT);
        }
        bitWrite(m_state, BIT_CURRENT, queryButtonDown());
    }
}

void Button::process(void)
{
    uint32_t currentMillis = millis();
    uint32_t interval = currentMillis - m_debounceStartTime;

    if(interval < uint32_t(m_debounceDuration)) {
        bitWrite(m_state, BIT_CHANGED, false);
        // not enough time has passed; ignore
        return;
    }

    // save the previous value
    bitWrite(m_state, BIT_PREVIOUS, bitRead(m_state, BIT_CURRENT));

    if (bitRead(m_state, BIT_TEST_MODE))
        bitWrite(m_state, BIT_CURRENT, bitRead(m_state, BIT_TEST_PRESSED));
    else
        bitWrite(m_state, BIT_CURRENT, queryButtonDown());

    // clear the hold, if it was set.
    bitWrite(m_state, BIT_HOLD_NOW, false);

    if (bitRead(m_state, BIT_CURRENT) != bitRead(m_state, BIT_PREVIOUS)) {
        m_debounceStartTime = currentMillis;

        if (bitRead(m_state, BIT_CURRENT)) {
            // Pressed.
            if (m_handlers && m_handlers->cb_onPress) {
                m_handlers->cb_onPress(*this);
            }
            m_pressedStartTime = currentMillis;        //start timing
            bitWrite(m_state, BIT_HOLD_TRIGGERED, false);
            bitWrite(m_state, BIT_HOLD_NOW, false);
        }
        else {
            // Released.
            if (m_handlers && m_handlers->cb_onRelease) {
                m_handlers->cb_onRelease(*this);
            }
            // Don't fire both hold and click.
            if (!bitRead(m_state, BIT_HOLD_TRIGGERED)) {
                if (m_handlers && m_handlers->cb_onClick) {
                    m_handlers->cb_onClick(*this);    //fire the onClick event AFTER the onRelease
                }
            }
            //reset m_states (for timing and for event triggering)
            m_pressedStartTime = 0;
            m_nHolds = 0;
        }
        bitWrite(m_state, BIT_CHANGED, true);
    }
    else {
        // m_state did NOT change.
        bitWrite(m_state, BIT_CHANGED, false);

        // should we trigger an onHold event? If so - trigger once unless hold repeating.
        if (isDown()) {
            uint32_t count = (currentMillis - m_pressedStartTime) / uint32_t(m_holdEventThreshold);
            if (count > m_nHolds) {
                int wasHoldTriggered = bitRead(m_state, BIT_HOLD_TRIGGERED);
                m_nHolds = count;

                bitWrite(m_state, BIT_HOLD_TRIGGERED, true);

                if (!wasHoldTriggered || m_holdRepeats) {
                    bitWrite(m_state, BIT_HOLD_NOW, true);
                    if (m_handlers && m_handlers->cb_onHold) {
                        m_handlers->cb_onHold(*this);
                    }
                }
            }
        }
    }
}



bool Button::isDown() const
{
    return bitRead(m_state, BIT_CURRENT);
}


bool Button::stateChanged() const
{
    return bitRead(m_state, BIT_CHANGED);
}


bool Button::press() const
{
    return (isDown() && stateChanged());
}


bool Button::held() const
{
    return isDown() && bitRead(m_state, BIT_HOLD_NOW);
}


uint32_t Button::holdTime() const
{
    if (!isDown())
        return 0;
    return millis() - m_pressedStartTime;
}


void Button::setHoldThreshold(uint32_t holdTime)
{
    m_holdEventThreshold = holdTime;
}

void Button::enableTestMode(bool testMode)
{
    m_state = 0;
    m_pressedStartTime = 0;
    m_debounceStartTime = 0;
    bitWrite(m_state, BIT_TEST_MODE, testMode);
    bitWrite(m_state, BIT_TEST_PRESSED, false);
}

void Button::testPress()
{
    bitWrite(m_state, BIT_TEST_PRESSED, true);
}


void Button::testRelease()
{
    bitWrite(m_state, BIT_TEST_PRESSED, false);
}


void ButtonCB::setPressHandler(buttonEventHandler handler)
{
    m_handlerData.cb_onPress = handler;
}


void ButtonCB::setReleaseHandler(buttonEventHandler handler)
{
    m_handlerData.cb_onRelease = handler;
}


void ButtonCB::setClickHandler(buttonEventHandler handler)
{
    m_handlerData.cb_onClick = handler;
}


void ButtonCB::setHoldHandler(buttonEventHandler handler)
{
    m_handlerData.cb_onHold = handler;
}
