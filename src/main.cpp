#include <Arduino.h>

#include "Config.h"
#include "boot/BootManager.h"
#include "display/DisplayManager.h"

namespace {
espion::boot::BootManager bootManager;
}

void setup() {
#if ESPION_ENABLE_SERIAL_DEBUG
    Serial.begin(115200);
    delay(200);
    Serial.println("[ESPION] Booting...");
#endif

    auto& display = espion::display::DisplayManager::getInstance();
    if (!display.begin()) {
#if ESPION_ENABLE_SERIAL_DEBUG
        Serial.println("[ESPION] FATAL: Display initialization failed.");
#endif
        return;
    }

    bootManager.begin();
}

void loop() {
    if (!bootManager.isFinished()) {
        bootManager.update();
        return;
    }

    // Main menu will be started here in the next milestone.
    delay(1);
}
