#include "DisplayManager.h"
#include "Config.h"

namespace espion {
namespace display {

DisplayManager& DisplayManager::getInstance() {
    static DisplayManager instance;
    return instance;
}

bool DisplayManager::begin() {
    if (_initialized) return true;
    if (!_lgfx.init()) return false;

    _lgfx.setRotation(config::DISPLAY_ROTATION);
    _lgfx.setColorDepth(16);
    _lgfx.setBrightness(255);
    _lgfx.fillScreen(TFT_BLACK);
    _initialized = true;
    return true;
}

void DisplayManager::setBrightness(uint8_t value) { _lgfx.setBrightness(value); }
void DisplayManager::clear(uint16_t color) { if (_initialized) _lgfx.fillScreen(color); }
void DisplayManager::fillRect(int32_t x, int32_t y, int32_t w, int32_t h, uint16_t color) { if (_initialized) _lgfx.fillRect(x,y,w,h,color); }
void DisplayManager::drawRect(int32_t x, int32_t y, int32_t w, int32_t h, uint16_t color) { if (_initialized) _lgfx.drawRect(x,y,w,h,color); }
void DisplayManager::drawLine(int32_t x0, int32_t y0, int32_t x1, int32_t y1, uint16_t color) { if (_initialized) _lgfx.drawLine(x0,y0,x1,y1,color); }
void DisplayManager::fillCircle(int32_t x, int32_t y, int32_t r, uint16_t color) { if (_initialized) _lgfx.fillCircle(x,y,r,color); }

void DisplayManager::drawText(int32_t x, int32_t y, const char* text, uint16_t color, uint8_t textSize) {
    if (!_initialized || text == nullptr) return;
    _lgfx.setTextColor(color);
    _lgfx.setTextSize(textSize);
    _lgfx.setCursor(x, y);
    _lgfx.print(text);
}

int32_t DisplayManager::textWidth(const char* text, uint8_t textSize) {
    if (!_initialized || text == nullptr) return 0;
    _lgfx.setTextSize(textSize);
    return _lgfx.textWidth(text);
}

void DisplayManager::fillScreenWithAlpha(uint16_t color, uint8_t alpha) {
    uint8_t r = ((color >> 11) & 0x1F) * alpha / 255;
    uint8_t g = ((color >> 5) & 0x3F) * alpha / 255;
    uint8_t b = (color & 0x1F) * alpha / 255;
    _lgfx.fillScreen((r << 11) | (g << 5) | b);
}

void DisplayManager::drawBitmap(int32_t x, int32_t y, int32_t w, int32_t h, const uint16_t* pixels) {
    if (!_initialized || pixels == nullptr || w <= 0 || h <= 0) return;
    _lgfx.pushImage(x, y, w, h, pixels);
}

void DisplayManager::startWrite() { _lgfx.startWrite(); }
void DisplayManager::endWrite() { _lgfx.endWrite(); }
LGFX_Device& DisplayManager::raw() { return _lgfx; }
int32_t DisplayManager::width() const { return _lgfx.width(); }
int32_t DisplayManager::height() const { return _lgfx.height(); }

} // namespace display
} // namespace espion
