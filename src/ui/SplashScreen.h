/**
 * ============================================================================
 *  ESPION Firmware — Splash Screen Renderer
 *  Engineered by Espada
 * ============================================================================
 *
 *  PURPOSE:
 *  SplashScreen draws a single frame of the boot experience for a given
 *  animation state (mascot scale, text alpha, loading progress). It has
 *  NO knowledge of timing, easing, or sequencing — it is purely "given
 *  these numbers, draw this frame."
 *
 *  BootAnimator computes the numbers (what scale/alpha/progress should be
 *  right now). BootManager decides which phase of the boot sequence is
 *  active and when to advance. SplashScreen only renders.
 *
 *  This separation means the visual design (mascot shape, layout, colors)
 *  can be reworked without touching any timing code, and vice versa.
 *
 *  All drawing goes through DisplayManager — never raw LovyanGFX calls.
 * ============================================================================
 */

#pragma once

#include <cstdint>
#include <cstddef>

namespace espion {
namespace ui {

/**
 * SplashScreen
 *
 * Stateless-per-call renderer for the ESPION boot visuals. Each draw*
 * method takes the display state it needs and paints exactly one frame.
 * Callers (BootAnimator/BootManager) are responsible for clearing the
 * screen and calling startWrite()/endWrite() around a frame as needed.
 */
class SplashScreen {
public:
    SplashScreen() = default;

    /// Draws the ESPION mascot centered on screen at the given scale.
    /// scale: 0.0 (invisible) to 1.0 (full size, 100%).
    /// color: mascot fill color, allowing fade-in-from-black by blending
    ///        the caller's chosen color toward black before passing it in.
    void drawMascot(float scale, uint16_t color) const;

    /// Draws the "ESPION" title and "Engineered by Espada" subtitle,
    /// vertically positioned below the mascot, using the given colors
    /// (callers pre-blend colors toward black/background for fade effects).
    void drawTitleText(uint16_t titleColor, uint16_t subtitleColor) const;

    /// Draws the loading bar frame (outline + fill) and status line beneath
    /// the mascot. progress: 0.0 - 1.0. statusText: e.g. "Initializing Display...".
    void drawLoadingBar(float progress, const char* statusText) const;

    /// Clears the entire screen to black. Convenience wrapper so callers
    /// don't need to know the background color constant.
    void clearBackground() const;

private:
    // --- Layout constants (screen-relative, computed from DisplayManager's
    //     reported width/height so this class never hardcodes 320x240). ---
    static constexpr float MASCOT_BASE_RADIUS_RATIO = 0.14f; // relative to screen height
    static constexpr uint8_t TITLE_FONT_SIZE = 3;
    static constexpr uint8_t SUBTITLE_FONT_SIZE = 1;
    static constexpr uint8_t LOADING_BAR_WIDTH_RATIO_PERCENT = 70; // % of screen width
    static constexpr uint8_t LOADING_BAR_HEIGHT = 6;
};

} // namespace ui
} // namespace espion