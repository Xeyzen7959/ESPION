/**
 * ============================================================================
 *  ESPION Firmware — Display Manager (Implementation)
 *  Engineered by Espada
 * ============================================================================
 */

#include "DisplayManager.h"
#include "Config.h"

namespace espion {
namespace display {

DisplayManager& DisplayManager::getInstance() {
    static DisplayManager instance;
    return instance;
}

bool DisplayManager::begin() {
    if (_initialized) {
        return true; // Idempotent — safe to call more than once.
    }

    if (!_lgfx.init()) {
        return false;
    }

    _lgfx.setRotation(config::DISPLAY_ROTATION);
    _lgfx.setColorDepth(16); // RGB565 per spec
    _lgfx.setBrightness(255);
    _lgfx.fillScreen(TFT_BLACK);

    _initialized = true;
    return true;
}

void DisplayManager::setBrightness(uint8_t value) {
    _lgfx.setBrightness(value);
}

void DisplayManager::clear(uint16_t color) {
    _lgfx.fillScreen(color);
}

void DisplayManager::fillRect(int32_t x, int32_t y, int32_t w, int32_t h, uint16_t color) {
    _lgfx.fillRect(x, y, w, h, color);
}

void DisplayManager::drawRect(int32_t x, int32_t y, int32_t w, int32_t h, uint16_t color) {
    _lgfx.drawRect(x, y, w, h, color);
}

void DisplayManager::fillCircle(int32_t x, int32_t y, int32_t r, uint16_t color) {
    _lgfx.fillCircle(x, y, r, color);
}

void DisplayManager::drawText(int32_t x, int32_t y, const char* text, uint16_t color, uint8_t textSize) {
    _lgfx.setTextColor(color);
    _lgfx.setTextSize(textSize);
    _lgfx.setCursor(x, y);
    _lgfx.print(text);
}

int32_t DisplayManager::textWidth(const char* text, uint8_t textSize) {
    _lgfx.setTextSize(textSize);
    return _lgfx.textWidth(text);
}

void DisplayManager::fillScreenWithAlpha(uint16_t color, uint8_t alpha) {
    // LovyanGFX's Panel_ILI9341 does not composite true alpha against
    // existing framebuffer contents on this simple fill path. For
    // ESPION's fade effects (boot text fade, screen transitions), the
    // practical approach used by BootAnimator is to interpolate the
    // color value itself toward the background color and redraw the
    // affected region — that logic lives in BootAnimator, which calls
    // drawText()/fillRect() repeatedly with interpolated colors.
    //
    // This method is kept as a simple direct fill for cases where a
    // flat alpha-blended rectangle (not text) is needed, blending
    // `color` toward black by `alpha` (0 = black, 255 = full color).
    uint8_t r = ((color >> 11) & 0x1F) * alpha / 255;
    uint8_t g = ((color >> 5) & 0x3F) * alpha / 255;
    uint8_t b = (color & 0x1F) * alpha / 255;
    uint16_t blended = (r << 11) | (g << 5) | b;
    _lgfx.fillScreen(blended);
}

void DisplayManager::startWrite() {
    _lgfx.startWrite();
}

void DisplayManager::endWrite() {
    _lgfx.endWrite();
}

LGFX_Device& DisplayManager::raw() {
    return _lgfx;
}

int32_t DisplayManager::width() const {
    return _lgfx.width();
}

int32_t DisplayManager::height() const {
    return _lgfx.height();
}

} // namespace display
} // namespace espion