/**
 * ============================================================================
 *  ESPION Firmware — Display Manager
 *  Engineered by Espada
 * ============================================================================
 *
 *  PURPOSE:
 *  DisplayManager is the ONLY class permitted to issue raw drawing calls.
 *  Every screen, boot animation, and UI widget draws through this class —
 *  never directly against LGFX_Device or LovyanGFX primitives.
 *
 *  This keeps the rendering backend swappable: if ESPION ever moves off
 *  LovyanGFX, only DisplayManager's implementation changes.
 *
 *  DESIGN NOTES:
 *  - Singleton-style access via getInstance(): the firmware has exactly
 *    one physical display, so a singleton is the honest representation
 *    rather than passing a reference through every constructor.
 *  - No dynamic allocation of the LGFX_Device — it's a value member,
 *    constructed once, lifetime = firmware lifetime (RAII).
 * ============================================================================
 */

#pragma once

#include <cstdint>
#include "LGFX_Device.h"

namespace espion {
namespace display {

/**
 * DisplayManager
 *
 * Thin, deliberate drawing API wrapping the LGFX_Device.
 * Coordinates are always in the panel's native rotation (portrait, 320x240
 * per Config.h — note: physical panel is 320x240 landscape-native; ESPION
 * runs it in the rotation configured by begin()).
 */
class DisplayManager {
public:
    // Non-copyable: there is exactly one display.
    DisplayManager(const DisplayManager&) = delete;
    DisplayManager& operator=(const DisplayManager&) = delete;

    static DisplayManager& getInstance();

    /// Initializes the underlying LGFX device, sets rotation, clears to black.
    /// Initializes the underlying LGFX device, sets rotation, and clears the screen.
    /// Returns false if the graphics device initialization reports failure.
    bool begin();

    /// Sets backlight brightness. value: 0 (off) - 255 (full brightness).
    void setBrightness(uint8_t value);

    /// Clears the entire screen to the given color.
    void clear(uint16_t color);

    /// Draws filled rectangle.
    void fillRect(int32_t x, int32_t y, int32_t w, int32_t h, uint16_t color);

    /// Draws rectangle outline.
    void drawRect(int32_t x, int32_t y, int32_t w, int32_t h, uint16_t color);

    /// Draws filled circle — used by boot mascot / icons.
    void fillCircle(int32_t x, int32_t y, int32_t r, uint16_t color);

    /// Draws a text string. Font size is a LovyanGFX text-size multiplier (1 = base size).
    void drawText(int32_t x, int32_t y, const char* text, uint16_t color, uint8_t textSize = 1);

    /// Returns text width in pixels for the given string at the given size —
    /// used by UI code to center text without duplicating font math.
    int32_t textWidth(const char* text, uint8_t textSize = 1);

    /// Fills the screen with a color scaled toward black using an alpha-like value.
    /// This is not framebuffer alpha compositing.
    void fillScreenWithAlpha(uint16_t color, uint8_t alpha);

    /// Must be called to begin a batch of draw operations for tear-free updates.
    void startWrite();

    /// Ends a batch started with startWrite().
    void endWrite();

    /// Direct access to the underlying LGFX device — INTERNAL USE ONLY.
    /// Reserved for BootAnimator's sprite/bitmap work where the thin wrapper
    /// above would be too limiting. Application/UI code must not call this.
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