#pragma once

#include <cstdint>

namespace espion::boot {

class BootAnimator {
public:
    static float clamp01(float value);
    static float easeInOut(float value);
    static float phaseProgress(uint32_t elapsedMs, uint32_t durationMs);
};

} // namespace espion::boot
