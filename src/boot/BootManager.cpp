#include "BootManager.h"

#include <Arduino.h>
#include "BootAnimator.h"

namespace espion::boot {
namespace {
constexpr uint32_t FRAME_INTERVAL_MS = 33; // ~30 FPS
constexpr uint32_t INTRO_REVEAL_MS = 1800;
constexpr uint32_t INTRO_HOLD_MS = 3200;
constexpr uint32_t INTRO_FADE_MS = 900;
constexpr uint32_t LOADING_MS = 4000;
constexpr uint32_t READY_MS = 1800;

const char* loadingStatus(float progress) {
    if (progress < 0.18f) return "INITIALIZING DISPLAY...";
    if (progress < 0.38f) return "INITIALIZING INPUT...";
    if (progress < 0.58f) return "INITIALIZING STORAGE...";
    if (progress < 0.78f) return "INITIALIZING BATTERY...";
    return "INITIALIZING UI...";
}
}

void BootManager::begin() {
    _phase = Phase::IntroReveal;
    _phaseStartMs = millis();
    _lastFrameMs = 0;
}

void BootManager::setPhase(Phase next) {
    _phase = next;
    _phaseStartMs = millis();
}

void BootManager::update() {
    if (_phase == Phase::Complete) return;

    const uint32_t now = millis();
    if (now - _lastFrameMs < FRAME_INTERVAL_MS) return;
    _lastFrameMs = now;

    const uint32_t elapsed = now - _phaseStartMs;
    const float ringPhase = static_cast<float>(now) * 0.0014f;

    switch (_phase) {
        case Phase::IntroReveal: {
            const float t = BootAnimator::easeInOut(BootAnimator::phaseProgress(elapsed, INTRO_REVEAL_MS));
            _screen.drawIntro(t, t, ringPhase);
            if (elapsed >= INTRO_REVEAL_MS) setPhase(Phase::IntroHold);
            break;
        }
        case Phase::IntroHold:
            _screen.drawIntro(1.0f, 1.0f, ringPhase);
            if (elapsed >= INTRO_HOLD_MS) setPhase(Phase::IntroFade);
            break;
        case Phase::IntroFade: {
            const float t = BootAnimator::easeInOut(BootAnimator::phaseProgress(elapsed, INTRO_FADE_MS));
            _screen.drawIntro(1.0f, 1.0f - t, ringPhase);
            if (elapsed >= INTRO_FADE_MS) setPhase(Phase::Loading);
            break;
        }
        case Phase::Loading: {
            const float p = BootAnimator::phaseProgress(elapsed, LOADING_MS);
            _screen.drawLoading(p, loadingStatus(p));
            if (elapsed >= LOADING_MS) setPhase(Phase::Ready);
            break;
        }
        case Phase::Ready: {
            const float a = BootAnimator::easeInOut(BootAnimator::phaseProgress(elapsed, 450));
            _screen.drawSystemReady(a);
            if (elapsed >= READY_MS) setPhase(Phase::Complete);
            break;
        }
        case Phase::Complete:
            break;
    }
}

bool BootManager::isFinished() const {
    return _phase == Phase::Complete;
}

} // namespace espion::boot
