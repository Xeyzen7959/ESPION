/**
 * ============================================================================
 *  ESPION Firmware — Input Manager (Implementation)
 *  Engineered by Espada
 * ============================================================================
 */

#include "InputManager.h"

#include <Arduino.h>

#include "Config.h"
#include "pins.h"

namespace espion::input {
namespace {

bool readActiveLow(uint8_t pin) {
    return digitalRead(pin) == LOW;
}

} // namespace

std::size_t InputManager::indexOf(Button button) {
    return static_cast<std::size_t>(button);
}

bool InputManager::isRepeatable(Button button) {
    return button == Button::Up || button == Button::Down ||
           button == Button::Left || button == Button::Right;
}

void InputManager::configureButton(Button button, uint8_t pin, uint32_t now) {
    pinMode(pin, INPUT_PULLUP);

    ButtonState& state = _buttons[indexOf(button)];
    state.pin = pin;
    state.rawDown = readActiveLow(pin);
    state.stableDown = state.rawDown;
    state.longPressSent = false;
    state.rawChangedMs = now;
    state.pressedMs = state.stableDown ? now : 0;
    state.nextRepeatMs = state.stableDown
        ? now + config::SCROLL_REPEAT_INITIAL_MS
        : 0;
}

void InputManager::begin() {
    const uint32_t now = millis();

    configureButton(Button::Up,    pins::BTN_UP, now);
    configureButton(Button::Down,  pins::BTN_DOWN, now);
    configureButton(Button::Left,  pins::BTN_LEFT, now);
    configureButton(Button::Right, pins::BTN_RIGHT, now);
    configureButton(Button::Ok,    pins::BTN_OK, now);

    clearEvents();
    _initialized = true;
}

void InputManager::update() {
    if (!_initialized) {
        return;
    }

    const uint32_t now = millis();
    pollButton(Button::Up, now);
    pollButton(Button::Down, now);
    pollButton(Button::Left, now);
    pollButton(Button::Right, now);
    pollButton(Button::Ok, now);
}

void InputManager::pollButton(Button button, uint32_t now) {
    ButtonState& state = _buttons[indexOf(button)];
    const bool sampledDown = readActiveLow(state.pin);

    if (sampledDown != state.rawDown) {
        state.rawDown = sampledDown;
        state.rawChangedMs = now;
    }

    // The raw signal must remain unchanged for the full debounce window before
    // it is accepted as the new stable state.
    if (state.rawDown != state.stableDown &&
        static_cast<uint32_t>(now - state.rawChangedMs) >= config::DEBOUNCE_MS) {

        state.stableDown = state.rawDown;

        if (state.stableDown) {
            state.pressedMs = now;
            state.nextRepeatMs = now + config::SCROLL_REPEAT_INITIAL_MS;
            state.longPressSent = false;
            pushEvent(button, EventType::Pressed, now);
        } else {
            pushEvent(button, EventType::Released, now);
            state.pressedMs = 0;
            state.nextRepeatMs = 0;
            state.longPressSent = false;
        }
    }

    if (!state.stableDown) {
        return;
    }

    if (!state.longPressSent &&
        static_cast<uint32_t>(now - state.pressedMs) >= config::LONG_PRESS_MS) {
        state.longPressSent = true;
        pushEvent(button, EventType::LongPressed, now);
    }

    if (isRepeatable(button) &&
        static_cast<int32_t>(now - state.nextRepeatMs) >= 0) {
        pushEvent(button, EventType::Repeat, now);
        state.nextRepeatMs = now + config::SCROLL_REPEAT_MS;
    }
}

void InputManager::pushEvent(Button button, EventType type, uint32_t now) {
    // If the consumer falls behind, discard the oldest event rather than
    // allocating memory or blocking the main loop.
    if (_eventCount == EVENT_QUEUE_CAPACITY) {
        _eventHead = (_eventHead + 1) % EVENT_QUEUE_CAPACITY;
        --_eventCount;
    }

    _events[_eventTail] = InputEvent{button, type, now};
    _eventTail = (_eventTail + 1) % EVENT_QUEUE_CAPACITY;
    ++_eventCount;
}

bool InputManager::popEvent(InputEvent& event) {
    if (_eventCount == 0) {
        return false;
    }

    event = _events[_eventHead];
    _eventHead = (_eventHead + 1) % EVENT_QUEUE_CAPACITY;
    --_eventCount;
    return true;
}

bool InputManager::isDown(Button button) const {
    const std::size_t index = indexOf(button);
    return index < BUTTON_COUNT && _buttons[index].stableDown;
}

void InputManager::clearEvents() {
    _eventHead = 0;
    _eventTail = 0;
    _eventCount = 0;
}

const char* InputManager::buttonName(Button button) {
    switch (button) {
        case Button::Up:    return "UP";
        case Button::Down:  return "DOWN";
        case Button::Left:  return "LEFT";
        case Button::Right: return "RIGHT";
        case Button::Ok:    return "OK";
        case Button::Count: break;
    }
    return "UNKNOWN";
}

const char* InputManager::eventName(EventType type) {
    switch (type) {
        case EventType::Pressed:     return "PRESSED";
        case EventType::Released:    return "RELEASED";
        case EventType::LongPressed: return "LONG_PRESSED";
        case EventType::Repeat:      return "REPEAT";
    }
    return "UNKNOWN";
}

} // namespace espion::input
