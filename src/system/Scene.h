/**
 * ============================================================================
 *  ESPION Firmware — Scene Interface
 *  Engineered by Espada
 * ============================================================================
 */

#pragma once

#include <cstdint>
#include "input/InputManager.h"

namespace espion::system {

enum class SceneID : uint8_t {
    Home,
    Network,
    Wireless,
    Logs,
    System,
    About,
    Count
};

class Scene {
public:
    virtual ~Scene() = default;

    virtual SceneID id() const = 0;

    /// Called whenever this scene becomes active.
    virtual void onEnter() {}

    /// Called immediately before this scene stops being active.
    virtual void onExit() {}

    /// Receives input events while this scene is active.
    virtual void handleInput(const input::InputEvent& event) = 0;

    /// Non-blocking scene update.
    virtual void update(uint32_t nowMs) = 0;

    /// Draws the scene when it has become dirty.
    virtual void render() = 0;

    /// Returns whether the scene needs a redraw.
    virtual bool isDirty() const = 0;

    /// Marks the scene dirty so it will be rendered again.
    virtual void invalidate() = 0;
};

} // namespace espion::system
