/**
 * ============================================================================
 *  ESPION Firmware — Home Scene (Implementation)
 *  Engineered by Espada
 * ============================================================================
 */

#include "scenes/HomeScene.h"

#include <Arduino.h>
#include "display/DisplayManager.h"

namespace espion::scenes {

namespace {
constexpr uint16_t COLOR_BLACK       = 0x0000;
constexpr uint16_t COLOR_WHITE       = 0xFFFF;
constexpr uint16_t COLOR_PURPLE      = 0xA2BE;
constexpr uint16_t COLOR_DARK_PURPLE = 0x3008;
constexpr uint16_t COLOR_MUTED       = 0x7BEF;

constexpr int32_t HEADER_HEIGHT = 30;
constexpr int32_t FOOTER_HEIGHT = 22;
constexpr int32_t MENU_START_Y  = 48;
constexpr int32_t ITEM_HEIGHT   = 31;
}

void HomeScene::onEnter() {
    _dirty = true;
}

void HomeScene::handleInput(const input::InputEvent& event) {
    const bool actionable =
        event.type == input::EventType::Pressed ||
        event.type == input::EventType::Repeat;

    if (!actionable) {
        return;
    }

    switch (event.button) {
        case input::Button::Up:
            moveSelection(-1);
            break;

        case input::Button::Down:
            moveSelection(1);
            break;

        case input::Button::Ok:
        case input::Button::Right:
            // Destination scenes are added in later milestones.
            // For now, briefly redraw the selected row as confirmation.
            _dirty = true;
            break;

        case input::Button::Left:
        case input::Button::Count:
            break;
    }
}

void HomeScene::update(uint32_t nowMs) {
    _lastUpdateMs = nowMs;
}

void HomeScene::render() {
    auto& display = display::DisplayManager::getInstance();

    display.startWrite();
    display.clear(COLOR_BLACK);

    drawHeader();
    drawMenu();
    drawFooter();

    display.endWrite();
    _dirty = false;
}

void HomeScene::moveSelection(int direction) {
    const int count = static_cast<int>(MENU_ITEMS.size());
    int next = static_cast<int>(_selectedIndex) + direction;

    if (next < 0) {
        next = count - 1;
    } else if (next >= count) {
        next = 0;
    }

    _selectedIndex = static_cast<std::size_t>(next);
    _dirty = true;
}

void HomeScene::drawHeader() {
    auto& display = display::DisplayManager::getInstance();
    const int32_t width = display.width();

    display.fillRect(0, 0, width, HEADER_HEIGHT, COLOR_DARK_PURPLE);
    display.drawText(10, 8, "ESPION", COLOR_WHITE, 2);
    display.drawText(width - 67, 10, "v0.1.0", COLOR_MUTED, 1);
    display.drawLine(0, HEADER_HEIGHT - 1, width, HEADER_HEIGHT - 1, COLOR_PURPLE);
}

void HomeScene::drawMenu() {
    auto& display = display::DisplayManager::getInstance();
    const int32_t width = display.width();

    for (std::size_t i = 0; i < MENU_ITEMS.size(); ++i) {
        const int32_t y = MENU_START_Y + static_cast<int32_t>(i) * ITEM_HEIGHT;
        const bool selected = i == _selectedIndex;

        if (selected) {
            display.fillRect(10, y - 5, width - 20, ITEM_HEIGHT - 3, COLOR_DARK_PURPLE);
            display.drawRect(10, y - 5, width - 20, ITEM_HEIGHT - 3, COLOR_PURPLE);
            display.drawText(20, y + 3, ">", COLOR_PURPLE, 2);
            display.drawText(43, y + 4, MENU_ITEMS[i].label, COLOR_WHITE, 2);
        } else {
            display.drawText(43, y + 4, MENU_ITEMS[i].label, COLOR_MUTED, 2);
        }
    }
}

void HomeScene::drawFooter() {
    auto& display = display::DisplayManager::getInstance();
    const int32_t width = display.width();
    const int32_t height = display.height();
    const int32_t y = height - FOOTER_HEIGHT;

    display.drawLine(0, y, width, y, COLOR_DARK_PURPLE);
    display.drawText(10, y + 7, "UP/DOWN NAVIGATE", COLOR_MUTED, 1);
    display.drawText(width - 74, y + 7, "OK SELECT", COLOR_PURPLE, 1);
}

} // namespace espion::scenes
