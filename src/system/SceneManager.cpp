/**
 * ============================================================================
 *  ESPION Firmware — Scene Manager (Implementation)
 *  Engineered by Espada
 * ============================================================================
 */

#include "system/SceneManager.h"

namespace espion::system {

bool SceneManager::registerScene(Scene& scene) {
    const std::size_t index = indexOf(scene.id());
    if (index >= _scenes.size()) {
        return false;
    }

    _scenes[index] = &scene;
    return true;
}

bool SceneManager::changeScene(SceneID sceneID) {
    const std::size_t index = indexOf(sceneID);
    if (index >= _scenes.size() || _scenes[index] == nullptr) {
        return false;
    }

    Scene* nextScene = _scenes[index];
    if (nextScene == _currentScene) {
        nextScene->invalidate();
        return true;
    }

    if (_currentScene != nullptr) {
        _currentScene->onExit();
    }

    _currentScene = nextScene;
    _currentScene->onEnter();
    _currentScene->invalidate();
    return true;
}

void SceneManager::handleInput(const input::InputEvent& event) {
    if (_currentScene != nullptr) {
        _currentScene->handleInput(event);
    }
}

void SceneManager::update(uint32_t nowMs) {
    if (_currentScene != nullptr) {
        _currentScene->update(nowMs);
    }
}

void SceneManager::render() {
    if (_currentScene != nullptr && _currentScene->isDirty()) {
        _currentScene->render();
    }
}

SceneID SceneManager::currentSceneID() const {
    return _currentScene != nullptr ? _currentScene->id() : SceneID::Count;
}

} // namespace espion::system
