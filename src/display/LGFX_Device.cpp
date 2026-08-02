/**
 * ============================================================================
 *  ESPION Firmware — LovyanGFX Device Descriptor (Implementation)
 *  Engineered by Espada
 * ============================================================================
 */

#include "LGFX_Device.h"

namespace espion {
namespace display {

LGFX_Device::LGFX_Device() {
    // ------------------------------------------------------------------
    // SPI BUS CONFIGURATION
    // Shared bus with the microSD card (separate CS lines per device).
    // ------------------------------------------------------------------
    auto bus_cfg = _bus_instance.config();

    bus_cfg.spi_host    = SPI2_HOST;
    bus_cfg.spi_mode    = 0;                          // ILI9341 uses SPI mode 0
    bus_cfg.freq_write  = config::SPI_FREQ_DISPLAY;
    bus_cfg.freq_read   = config::SPI_FREQ_READ;
    bus_cfg.spi_3wire   = false;                       // Separate MISO line wired
    bus_cfg.use_lock    = true;                        // Bus is shared with SD card
    bus_cfg.dma_channel = SPI_DMA_CH_AUTO;

    bus_cfg.pin_sclk = pins::SPI_SCK;
    bus_cfg.pin_mosi = pins::SPI_MOSI;
    bus_cfg.pin_miso = pins::SPI_MISO;
    bus_cfg.pin_dc   = pins::TFT_DC;

    _bus_instance.config(bus_cfg);
    _panel_instance.setBus(&_bus_instance);

    // ------------------------------------------------------------------
    // PANEL CONFIGURATION
    // ------------------------------------------------------------------
    auto panel_cfg = _panel_instance.config();

    panel_cfg.pin_cs   = pins::TFT_CS;
    panel_cfg.pin_rst  = pins::TFT_RST;
    panel_cfg.pin_busy = -1;                           // ILI9341 has no BUSY line

    panel_cfg.panel_width    = config::DISPLAY_WIDTH;
    panel_cfg.panel_height   = config::DISPLAY_HEIGHT;
    panel_cfg.offset_x       = 0;
    panel_cfg.offset_y       = 0;
    panel_cfg.offset_rotation = 0;
    panel_cfg.readable   = true;
    panel_cfg.invert     = false;
    panel_cfg.rgb_order  = false;                       // BGR order (standard ILI9341) NOTE: IF SWAPPED CHANGE FALSE TO TRUE
    panel_cfg.dlen_16bit = false;
    panel_cfg.bus_shared = true;                        // CRITICAL: bus shared with SD card

    _panel_instance.config(panel_cfg);

    // ------------------------------------------------------------------
    // BACKLIGHT CONFIGURATION (PWM dimmable via LEDC)
    // ------------------------------------------------------------------
    auto light_cfg = _light_instance.config();

    light_cfg.pin_bl      = pins::TFT_BL;
    light_cfg.invert      = false;
    light_cfg.freq        = config::BACKLIGHT_PWM_FREQ;
    light_cfg.pwm_channel = config::BACKLIGHT_PWM_CHANNEL;

    _light_instance.config(light_cfg);
    _panel_instance.setLight(&_light_instance);

    // ------------------------------------------------------------------
    // Register panel with the LGFX_Device base class
    // ------------------------------------------------------------------
    setPanel(&_panel_instance);
}

} // namespace display
} // namespace espion