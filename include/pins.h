/**
 * ============================================================================
 *  ESPION Firmware — Centralized Pin Configuration
 *  Engineered by Espada
 *  https://github.com/Xeyzen7959
 * ============================================================================
 *
 *  RULE: No GPIO number may appear anywhere else in the codebase.
 *  Every module includes this header and references these constants only.
 *  When real hardware is finalized, this is the ONLY file that changes.
 *
 *  Target board: ESP32-S3 DevKitC-1 (N16R8)
 *  See /PINOUT.md for the full human-readable pin map and wiring notes.
 * ============================================================================
 */

#pragma once

#include <cstdint>

namespace espion {
namespace pins {

// ============================================================================
// SHARED SPI BUS  (Display + microSD)
// ----------------------------------------------------------------------------
// The display and SD card share one physical SPI bus. They are never
// active at the same time — SPIManager owns bus arbitration and issues
// SPI::beginTransaction()/endTransaction() around every access.
// ============================================================================
constexpr uint8_t SPI_SCK   = 12;   // Shared SPI clock
constexpr uint8_t SPI_MOSI  = 11;   // Shared SPI MOSI
constexpr uint8_t SPI_MISO  = 13;   // Shared SPI MISO

// --- Display (ILI9341) chip select + control lines -------------------------
constexpr uint8_t TFT_CS    = 10;   // Display chip select
constexpr uint8_t TFT_DC    = 9;    // Data/Command
constexpr uint8_t TFT_RST   = 8;    // Hardware reset
constexpr uint8_t TFT_BL    = 14;   // Backlight (PWM-capable, LEDC channel)

// --- microSD card chip select -----------------------------------------------
constexpr uint8_t SD_CS     = 21;   // SD card chip select (separate from TFT_CS)

// ============================================================================
// 5-WAY NAVIGATION SWITCH
// ----------------------------------------------------------------------------
// All inputs use internal pull-ups; switch pulls line LOW when pressed.
// Avoids strapping pins (0, 3, 45, 46) and USB-JTAG pins (19, 20).
// ============================================================================
constexpr uint8_t BTN_UP     = 4;
constexpr uint8_t BTN_DOWN   = 5;
constexpr uint8_t BTN_LEFT   = 6;
constexpr uint8_t BTN_RIGHT  = 7;
constexpr uint8_t BTN_OK     = 15;   // Center press (select / confirm)

// ============================================================================
// POWER / BATTERY MONITORING
// ----------------------------------------------------------------------------
// Reserved ADC1 pin (ADC1 remains usable even when WiFi radio is active,
// unlike ADC2). Real hardware: voltage divider from LiPo+ into this pin.
// See HARDWARE.md for divider ratio and expected voltage range.
// ============================================================================
constexpr uint8_t BATT_ADC  = 1;    // ADC1_CH0 — battery voltage sense

// ============================================================================
// FUTURE EXPANSION — RESERVED, NOT YET WIRED OR INITIALIZED
// ----------------------------------------------------------------------------
// These pins are documented here to prevent accidental reuse by future
// features. They share the existing SPI bus (SCK/MOSI/MISO) where possible;
// each peripheral gets its own dedicated chip-select line.
// ============================================================================

// --- RFID (MFRC522, SPI) ----------------------------------------------------
constexpr uint8_t RFID_CS   = 38;   // MFRC522 chip select (shared SPI bus)
constexpr uint8_t RFID_RST  = 39;   // MFRC522 reset

// --- CC1101 (Sub-GHz transceiver, SPI) --------------------------------------
constexpr uint8_t CC1101_CS  = 40;  // CC1101 chip select (shared SPI bus)
constexpr uint8_t CC1101_GDO0 = 41; // CC1101 GDO0 interrupt/data line

// --- NRF24L01 (2.4GHz transceiver, SPI) -------------------------------------
constexpr uint8_t NRF24_CS  = 42;   // NRF24 chip select (shared SPI bus)
constexpr uint8_t NRF24_CE  = 2;    // NRF24 chip enable

// --- GPS (UART) --------------------------------------------------------------
constexpr uint8_t GPS_RX    = 17;   // ESP32 RX  <- GPS TX
constexpr uint8_t GPS_TX    = 18;   // ESP32 TX  -> GPS RX

} // namespace pins
} // namespace espion
