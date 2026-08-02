# ESPION Firmware — Pinout Reference

**Board:** ESP32-S3 DevKitC-1 (N16R8 — 16MB Flash / 8MB PSRAM)
**Source of truth:** `include/pins.h` — this document is generated to match it.
If you rewire the hardware, update `pins.h` first, then reflect the change here.

---

## Shared SPI Bus (Display + microSD)

The ILI9341 display and microSD card share a single physical SPI bus.
Each device has its own dedicated Chip Select line. `SPIManager` guarantees
the two devices are never active on the bus simultaneously.

| Signal      | GPIO | Notes                              |
|-------------|------|-------------------------------------|
| SPI_SCK     | 12   | Shared clock                        |
| SPI_MOSI    | 11   | Shared data out                     |
| SPI_MISO    | 13   | Shared data in                      |
| TFT_CS      | 10   | Display chip select                 |
| TFT_DC      | 9    | Display data/command select         |
| TFT_RST     | 8    | Display hardware reset              |
| TFT_BL      | 14   | Backlight, PWM (LEDC) dimmable      |
| SD_CS       | 21   | microSD card chip select            |

## 5-Way Navigation Switch

All inputs configured `INPUT_PULLUP`; active-low (pressed = LOW).

| Signal    | GPIO | Notes            |
|-----------|------|------------------|
| BTN_UP    | 4    |                  |
| BTN_DOWN  | 5    |                  |
| BTN_LEFT  | 6    |                  |
| BTN_RIGHT | 7    |                  |
| BTN_OK    | 15   | Center / confirm |

## Power / Battery Monitoring

| Signal    | GPIO | Notes                                              |
|-----------|------|-----------------------------------------------------|
| BATT_ADC  | 1    | ADC1_CH0 — reserved for voltage divider (see HARDWARE.md) |

> ADC1 is used deliberately — it remains functional even when the WiFi
> radio is active. ADC2 does not, and WiFi Analyzer is a planned feature.

## Reserved for Future Expansion (NOT wired or initialized yet)

These pins are reserved to prevent future collisions. They are documented
but unused in v0.1.0.

### RFID (MFRC522 — SPI)
| Signal    | GPIO | Notes                        |
|-----------|------|-------------------------------|
| RFID_CS   | 38   | Shares existing SPI bus       |
| RFID_RST  | 39   | Reset line                    |

### CC1101 (Sub-GHz Transceiver — SPI)
| Signal      | GPIO | Notes                          |
|-------------|------|---------------------------------|
| CC1101_CS   | 40   | Shares existing SPI bus         |
| CC1101_GDO0 | 41   | Interrupt / data ready line     |

### NRF24L01 (2.4GHz Transceiver — SPI)
| Signal    | GPIO | Notes                      |
|-----------|------|-----------------------------|
| NRF24_CS  | 42   | Shares existing SPI bus     |
| NRF24_CE  | 2    | Chip enable                 |

### GPS (UART)
| Signal | GPIO | Notes                    |
|--------|------|---------------------------|
| GPS_RX | 17   | ESP32 RX ← GPS TX module  |
| GPS_TX | 18   | ESP32 TX → GPS RX module  |

---

## Pins Deliberately Avoided

- **GPIO 0, 3, 45, 46** — strapping pins, avoided to prevent boot-mode issues.
- **GPIO 19, 20** — reserved for USB-JTAG (used for flashing/debugging over USB-C).

---

## Changing Pin Assignments

1. Edit only `include/pins.h`.
2. No other source file should ever contain a raw GPIO number — every
   module (`DisplayManager`, `InputManager`, `StorageManager`, etc.)
   includes `pins.h` and references `espion::pins::*` constants.
3. Update this document to match.
