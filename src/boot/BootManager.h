#pragma once

#include <cstdint>
#include "ui/SplashScreen.h"

namespace espion::boot {

class BootManager {
public:
    void begin();
    void update();
    bool isFinished() const;

private:
    enum class Phase {
        IntroReveal,
        IntroHold,
        IntroFade,
        Loading,
        Ready,
        Complete
    };

    void setPhase(Phase next);

    Phase _phase = Phase::IntroReveal;
    uint32_t _phaseStartMs = 0;
    uint32_t _lastFrameMs = 0;
    ui::SplashScreen _screen;
};

} // namespace espion::boot
