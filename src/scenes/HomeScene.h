/**
 * ============================================================================
 *  ESPION Firmware — Home Scene
 *  Engineered by Espada
 * ============================================================================
 */

#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

#include "system/Scene.h"

namespace espion::scenes {

class HomeScene final : public system::Scene {
public:
    HomeScene() = default;

    system::SceneID id() const override { return system::SceneID::Home; }

    void onEnter() override;
    void handleInput(const input::InputEvent& event) override;
    void update(uint32_t nowMs) override;
    void render() override;

    bool isDirty() const override { return _dirty; }
    void invalidate() override { _dirty = true; }

private:
    struct MenuItem {
        const char* label;
        system::SceneID destination;
    };

    static constexpr std::array<MenuItem, 5> MENU_ITEMS{{
        {"NETWORK",  system::SceneID::Network},
        {"WIRELESS", system::SceneID::Wireless},
        {"LOGS",     system::SceneID::Logs},
        {"SYSTEM",   system::SceneID::System},
        {"ABOUT",    system::SceneID::About},
    }};

    void moveSelection(int direction);
    void drawHeader();
    void drawMenu();
    void drawFooter();

    std::size_t _selectedIndex = 0;
    bool _dirty = true;
    uint32_t _lastUpdateMs = 0;
};

} // namespace espion::scenes
