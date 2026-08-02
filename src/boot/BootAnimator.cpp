#include "BootAnimator.h"

#include <algorithm>

namespace espion::boot {

float BootAnimator::clamp01(float value) {
    return std::max(0.0f, std::min(1.0f, value));
}

float BootAnimator::easeInOut(float value) {
    const float t = clamp01(value);
    return t < 0.5f ? 4.0f * t * t * t
                    : 1.0f - ((-2.0f * t + 2.0f) * (-2.0f * t + 2.0f) * (-2.0f * t + 2.0f)) / 2.0f;
}

float BootAnimator::phaseProgress(uint32_t elapsedMs, uint32_t durationMs) {
    if (durationMs == 0) return 1.0f;
    return clamp01(static_cast<float>(elapsedMs) / static_cast<float>(durationMs));
}

} // namespace espion::boot
