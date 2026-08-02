# ESPION Firmware — Hardware Notes

## Battery Monitoring

ESPION's `BatteryManager` supports two modes, controlled at compile time
via the `ESPION_BATTERY_USE_REAL_ADC` flag in `include/Config.h`:

- **`0` (default, v0.1.0):** Simulated battery mode. Returns a plausible,
  slowly-varying battery percentage so the status bar icon can be fully
  developed and tested before battery hardware exists.
- **`1`:** Real ADC mode. Reads `pins::BATT_ADC` and converts the sampled
  voltage into a percentage using the constants below.

### Required Circuit — Voltage Divider

A single-cell LiPo (nominal 3.7V, ~4.2V full charge) exceeds the ESP32-S3's
safe ADC input range (0–3.3V), so a resistive divider is required:

```
 LiPo+ (up to 4.2V)
    |
   [R1]
    |
    +----------> BATT_ADC (GPIO 1)
    |
   [R2]
    |
   GND
```

- With **R1 = R2** (equal resistors, e.g. 100kΩ each), the divider ratio is
  **2.0**, matching `config::BATTERY_DIVIDER_RATIO`.
- This halves the battery voltage before it reaches the ADC pin:
  4.2V → 2.1V (safely within the 3.3V ADC range).
- Recommended resistor value: 100kΩ–220kΩ range to minimize idle current
  draw from the battery while still giving a stable ADC reading.
- Add a 0.1µF ceramic capacitor across R2 (ADC input to GND) to smooth
  noise for more stable readings.

### Expected Voltage Range

| State          | Battery Voltage | Divided Voltage (÷2) |
|----------------|------------------|------------------------|
| Full charge    | 4.20V            | 2.10V                  |
| Nominal        | 3.70V            | 1.85V                  |
| Cutoff (0%)    | 3.30V            | 1.65V                  |

These map to `config::BATTERY_VOLTAGE_MAX` / `BATTERY_VOLTAGE_MIN`.
When real hardware is wired, `BatteryManager` reads raw ADC counts,
converts to voltage using the ESP32-S3's ADC calibration curve, multiplies
by `BATTERY_DIVIDER_RATIO` to reconstruct actual battery voltage, then maps
that linearly between MIN/MAX into a 0–100% value.

### Charging Module

A TP4056-based module is planned for USB-C charging. The TP4056 itself
does not need a firmware-visible signal for v0.1.0; charge-status GPIO
(CHRG/STDBY pins) can be wired to a future firmware input once hardware
is finalized. Not implemented yet — no pin currently reserved for it.
If needed later, choose a spare pin and document it in `pins.h` + `PINOUT.md`.

## SPI Bus Sharing (Display + SD)

Both the ILI9341 display and microSD card sit on the same physical SPI
bus (`SPI_SCK` / `SPI_MOSI` / `SPI_MISO`), differentiated only by their
Chip Select line (`TFT_CS` vs `SD_CS`). This is standard practice for
ESP32 handhelds and keeps GPIO usage low.

**Firmware responsibility:** `SPIManager` must ensure only one CS line is
ever active (LOW) at a time, and must wrap each device access in
`SPI.beginTransaction()` / `SPI.endTransaction()` with the appropriate
clock speed for that device (`SPI_FREQ_DISPLAY` vs `SPI_FREQ_SD`).

## Future Expansion Hardware

RFID (MFRC522), CC1101, and NRF24L01 are all SPI devices and are planned
to share the same physical bus, each with a dedicated CS pin (see
`PINOUT.md`). GPS is UART-based and uses its own RX/TX pair. None of this
hardware is initialized in v0.1.0 — pins are reserved only.
