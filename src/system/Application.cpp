/**
 * ============================================================================
 *  ESPION Firmware — Application (Implementation)
 *  Engineered by Espada
 * ============================================================================
 */

#include "system/Application.h"

#include <Arduino.h>

namespace espion::system {

Application::Application(input::InputManager& inputManager)
    : _inputManager(inputManager) {
}

void Application::begin() {
    if (_initialized) {
        return;
    }

    _sceneManager.registerScene(_homeScene);
    _sceneManager.changeScene(SceneID::Home);

    _inputManager.clearEvents();
    _initialized = true;
}

void Application::update() {
    if (!_initialized) {
        return;
    }

    _inputManager.update();

    input::InputEvent event{};
    while (_inputManager.popEvent(event)) {
        _sceneManager.handleInput(event);
    }

    _sceneManager.update(millis());
    _sceneManager.render();
}

} // namespace espion::system
