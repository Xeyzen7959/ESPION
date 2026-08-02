#pragma once

#include <cstdint>
#include "LGFX_Device.h"

namespace espion {
namespace display {

class DisplayManager {
public:
    DisplayManager(const DisplayManager&) = delete;
    DisplayManager& operator=(const DisplayManager&) = delete;

    static DisplayManager& getInstance();

    bool begin();
    void setBrightness(uint8_t value);
    void clear(uint16_t color);
    void fillRect(int32_t x, int32_t y, int32_t w, int32_t h, uint16_t color);
    void drawRect(int32_t x, int32_t y, int32_t w, int32_t h, uint16_t color);
    void drawLine(int32_t x0, int32_t y0, int32_t x1, int32_t y1, uint16_t color);
    void fillCircle(int32_t x, int32_t y, int32_t r, uint16_t color);
    void drawText(int32_t x, int32_t y, const char* text, uint16_t color, uint8_t textSize = 1);
    int32_t textWidth(const char* text, uint8_t textSize = 1);
    void fillScreenWithAlpha(uint16_t color, uint8_t alpha);

    // Draws a raw RGB565 image stored in flash/ROM.
    void drawBitmap(int32_t x, int32_t y, int32_t w, int32_t h, const uint16_t* pixels);

    void startWrite();
    void endWrite();
    LGFX_Device& raw();

    int32_t width() const;
    int32_t height() const;

private:
    DisplayManager() = default;

    LGFX_Device _lgfx;
    bool _initialized = false;
};

} // namespace display
} // namespace espion
