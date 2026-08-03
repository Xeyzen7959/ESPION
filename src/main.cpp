/**
 * ============================================================================
 *  ESPION Firmware — Entry Point
 *  Engineered by Espada
 *  https://github.com/Xeyzen7959
 * ============================================================================
 */

#include <Arduino.h>

#include "Config.h"
#include "boot/BootManager.h"
#include "display/DisplayManager.h"
#include "input/InputManager.h"
#include "system/Application.h"

namespace {
espion::boot::BootManager bootManager;
espion::input::InputManager inputManager;
espion::system::Application application(inputManager);

bool applicationStarted = false;
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

    inputManager.begin();
    bootManager.begin();
}

void loop() {
    if (!bootManager.isFinished()) {
        bootManager.update();
        return;
    }

    if (!applicationStarted) {
        application.begin();
        applicationStarted = true;

#if ESPION_ENABLE_SERIAL_DEBUG
        Serial.println("[ESPION] Application started.");
        Serial.println("[ESPION] Home scene active.");
#endif
    }

    application.update();
    delay(1);
}
