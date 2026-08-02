/**
 * ============================================================================
 *  ESPION Firmware — General Hardware & Build Configuration
 *  Engineered by Espada
 * ============================================================================
 *  Non-pin hardware constants live here (display geometry, SPI speeds,
 *  timing constants, feature toggles). GPIO numbers belong in pins.h only.
 * ============================================================================
 */

#pragma once

#include <cstdint>

namespace espion {
namespace config {

// ============================================================================
// DISPLAY
// ============================================================================
constexpr uint16_t DISPLAY_WIDTH     = 320;
constexpr uint16_t DISPLAY_HEIGHT    = 240;
constexpr uint8_t  DISPLAY_ROTATION  = 0;      // Portrait per spec
constexpr uint32_t SPI_FREQ_DISPLAY  = 40'000'000; // 40 MHz write
constexpr uint32_t SPI_FREQ_READ     = 16'000'000; // ILI9341 read speed
constexpr uint8_t  BACKLIGHT_PWM_CHANNEL = 0;
constexpr uint32_t BACKLIGHT_PWM_FREQ    = 5000;
constexpr uint8_t  BACKLIGHT_PWM_RES_BITS = 8;

// ============================================================================
// SD CARD
// ============================================================================
constexpr uint32_t SPI_FREQ_SD = 20'000'000;   // Conservative, shared-bus safe

// ============================================================================
// INPUT / DEBOUNCE
// ============================================================================
constexpr uint16_t DEBOUNCE_MS       = 25;     // Ignore bounce noise window
constexpr uint16_t LONG_PRESS_MS     = 600;    // Hold duration to trigger long-press
constexpr uint16_t INPUT_POLL_MS     = 10;     // Input task polling interval
constexpr uint16_t SCROLL_REPEAT_MS  = 120;    // Repeat interval while held (smooth scroll)
constexpr uint16_t SCROLL_REPEAT_INITIAL_MS = 400; // Delay before repeat kicks in

// ============================================================================
// BATTERY MONITORING
// ============================================================================
// Set to 1 to read real ADC hardware, 0 to use simulated values.
// Flip this once the voltage divider is wired — no other code changes needed.
#ifndef ESPION_BATTERY_USE_REAL_ADC
#define ESPION_BATTERY_USE_REAL_ADC 0
#endif

constexpr float BATTERY_VOLTAGE_MIN = 3.30f;  // 0% — LiPo cutoff
constexpr float BATTERY_VOLTAGE_MAX = 4.20f;  // 100% — full charge
constexpr float BATTERY_DIVIDER_RATIO = 2.0f; // R1=R2 divider, see HARDWARE.md

// ============================================================================
// BOOT SEQUENCE TIMING (ms)
// ============================================================================
constexpr uint16_t BOOT_MASCOT_SCALE_DURATION   = 900;
constexpr uint16_t BOOT_TEXT_HOLD_DURATION      = 1000;
constexpr uint16_t BOOT_TEXT_FADE_DURATION      = 500;
constexpr uint16_t BOOT_LOADER_DURATION         = 1200;
constexpr uint16_t BOOT_FINAL_FADE_DURATION     = 400;

// ============================================================================
// FEATURE FLAGS
// ============================================================================
#ifndef ESPION_ENABLE_SERIAL_DEBUG
#define ESPION_ENABLE_SERIAL_DEBUG 1
#endif

} // namespace config
} // namespace espion
