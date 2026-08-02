/**
 * ============================================================================
 *  ESPION Firmware — LovyanGFX Device Descriptor
 *  Engineered by Espada
 * ============================================================================
 *
 *  PURPOSE:
 *  Describes the physical display hardware to LovyanGFX: panel driver,
 *  SPI bus wiring, and backlight control. Contains no application logic
 *  and no drawing calls — DisplayManager owns all of that.
 *
 *  All GPIOs come from pins.h. All clock speeds come from Config.h.
 *  No GPIO number or frequency is hardcoded in this file.
 * ============================================================================
 */

#pragma once

#ifndef LGFX_USE_V1
#define LGFX_USE_V1
#endif
#include <LovyanGFX.hpp>
#include <lgfx/v1/panel/Panel_ILI9341.hpp>
#include <lgfx/v1/platforms/esp32/Bus_SPI.hpp>
#include <lgfx/v1/platforms/esp32/Light_PWM.hpp>

#include "Config.h"
#include "pins.h"

namespace espion {
namespace display {

/**
 * LGFX_Device
 *
 * Concrete LovyanGFX device for the ESPION handheld:
 *   - Panel:     ILI9341, 320x240
 *   - Bus:       SPI, shared with the microSD card (separate CS lines)
 *   - Backlight: PWM-dimmable via LEDC
 *
 * Instantiated once by DisplayManager. Never accessed directly by
 * application/UI code.
 */
class LGFX_Device : public lgfx::LGFX_Device {
public:
    LGFX_Device();

private:
    lgfx::Panel_ILI9341 _panel_instance;
    lgfx::Bus_SPI       _bus_instance;
    lgfx::Light_PWM     _light_instance;
};

} // namespace display
} // namespace espion