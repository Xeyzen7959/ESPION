#pragma once

#include <cstdint>

namespace espion::ui {

class SplashScreen {
public:
    void drawIntro(float reveal, float titleAlpha, float ringPhase) const;
    void drawLoading(float progress, const char* statusText) const;
    void drawSystemReady(float alpha) const;

private:
    static uint16_t blendTowardBlack(uint16_t color, float alpha);
    static void drawSegmentedRing(float phase, int radius, uint16_t color);
};

} // namespace espion::ui
