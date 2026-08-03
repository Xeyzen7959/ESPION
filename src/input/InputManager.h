/**
 * ============================================================================
 *  ESPION Firmware — Input Manager
 *  Engineered by Espada
 * ============================================================================
 *
 *  Reads the five active-low navigation buttons and converts raw GPIO changes
 *  into debounced press, release, long-press, and hold-repeat events.
 *
 *  No dynamic allocation is used. Events are stored in a small fixed queue.
 * ============================================================================
 */

#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

namespace espion::input {

enum class Button : uint8_t {
    Up,
    Down,
    Left,
    Right,
    Ok,
    Count
};

enum class EventType : uint8_t {
    Pressed,
    Released,
    LongPressed,
    Repeat
};

struct InputEvent {
    Button button;
    EventType type;
    uint32_t timestampMs;
};

class InputManager {
public:
    InputManager() = default;

    InputManager(const InputManager&) = delete;
    InputManager& operator=(const InputManager&) = delete;

    /// Configures all five inputs as active-low INPUT_PULLUP buttons.
    void begin();

    /// Poll this frequently from loop(). It never blocks.
    void update();

    /// Removes the oldest pending event. Returns false when the queue is empty.
    bool popEvent(InputEvent& event);

    /// Returns the latest stable debounced state of a button.
    bool isDown(Button button) const;

    /// Discards all pending events without changing current button states.
    void clearEvents();

    bool isInitialized() const { return _initialized; }

    static const char* buttonName(Button button);
    static const char* eventName(EventType type);

private:
    struct ButtonState {
        uint8_t pin = 0;
        bool rawDown = false;
        bool stableDown = false;
        bool longPressSent = false;
        uint32_t rawChangedMs = 0;
        uint32_t pressedMs = 0;
        uint32_t nextRepeatMs = 0;
    };

    static constexpr std::size_t BUTTON_COUNT = static_cast<std::size_t>(Button::Count);
    static constexpr std::size_t EVENT_QUEUE_CAPACITY = 16;

    static std::size_t indexOf(Button button);
    static bool isRepeatable(Button button);

    void configureButton(Button button, uint8_t pin, uint32_t now);
    void pollButton(Button button, uint32_t now);
    void pushEvent(Button button, EventType type, uint32_t now);

    std::array<ButtonState, BUTTON_COUNT> _buttons{};
    std::array<InputEvent, EVENT_QUEUE_CAPACITY> _events{};
    std::size_t _eventHead = 0;
    std::size_t _eventTail = 0;
    std::size_t _eventCount = 0;
    bool _initialized = false;
};

} // namespace espion::input