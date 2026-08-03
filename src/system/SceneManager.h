/**
 * ============================================================================
 *  ESPION Firmware — Scene Manager
 *  Engineered by Espada
 * ============================================================================
 */

#pragma once

#include <array>
#include <cstddef>
#include "system/Scene.h"

namespace espion::system {

class SceneManager {
public:
    SceneManager() = default;

    SceneManager(const SceneManager&) = delete;
    SceneManager& operator=(const SceneManager&) = delete;

    /// Registers a scene. Ownership remains with the caller.
    bool registerScene(Scene& scene);

    /// Activates the requested scene if it has been registered.
    bool changeScene(SceneID sceneID);

    void handleInput(const input::InputEvent& event);
    void update(uint32_t nowMs);
    void render();

    Scene* currentScene() { return _currentScene; }
    const Scene* currentScene() const { return _currentScene; }

    SceneID currentSceneID() const;

private:
    static constexpr std::size_t SCENE_COUNT =
        static_cast<std::size_t>(SceneID::Count);

    static constexpr std::size_t indexOf(SceneID sceneID) {
        return static_cast<std::size_t>(sceneID);
    }

    std::array<Scene*, SCENE_COUNT> _scenes{};
    Scene* _currentScene = nullptr;
};

} // namespace espion::system
