/**
 * ============================================================================
 *  ESPION Firmware — Application
 *  Engineered by Espada
 * ============================================================================
 */

#pragma once

#include <cstdint>

#include "input/InputManager.h"
#include "scenes/HomeScene.h"
#include "system/SceneManager.h"

namespace espion::system {

class Application {
public:
    explicit Application(input::InputManager& inputManager);

    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;

    void begin();
    void update();

    SceneManager& scenes() { return _sceneManager; }
    const SceneManager& scenes() const { return _sceneManager; }

private:
    input::InputManager& _inputManager;
    SceneManager _sceneManager;
    scenes::HomeScene _homeScene;

    bool _initialized = false;
};

} // namespace espion::system
