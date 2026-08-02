/**
 * ============================================================================
 *  ESPION Firmware — Entry Point
 *  Engineered by Espada
 *  https://github.com/Xeyzen7959
 * ============================================================================
 */

#include <Arduino.h>

#include "Config.h"
#include "display/DisplayManager.h"

using espion::display::DisplayManager;

void setup() {
#if ESPION_ENABLE_SERIAL_DEBUG
    Serial.begin(115200);
    delay(200);
    Serial.println("[ESPION] Booting...");
#endif

    DisplayManager& display = DisplayManager::getInstance();

    if (!display.begin()) {
#if ESPION_ENABLE_SERIAL_DEBUG
        Serial.println("[ESPION] FATAL: Display initialization failed.");
#endif
        return;
    }

#if ESPION_ENABLE_SERIAL_DEBUG
    Serial.println("[ESPION] Display initialized OK.");
#endif

    // Temporary display test
    display.clear(TFT_BLACK);
    display.fillRect(20, 20, 80, 80, 0xA254);
    display.drawRect(10, 10, 300, 220, TFT_WHITE);
    display.drawText(30, 110, "ESPION", TFT_WHITE, 3);
    display.drawText(30, 150, "Engineered by Espada", TFT_WHITE, 1);
}

void loop() {
    // Intentionally empty for the first display test.
}