#include "SplashScreen.h"

#include <algorithm>
#include <cmath>

#include "assets/MascotBitmap.h"
#include "display/DisplayManager.h"

namespace espion::ui {
namespace {
constexpr uint16_t BLACK = 0x0000;
constexpr uint16_t WHITE = 0xFFFF;
constexpr uint16_t PURPLE = 0xA2BE; // approx #A855F7 in RGB565
constexpr float kPi = 3.14159265f;

float clamp01(float v) { return std::max(0.0f, std::min(1.0f, v)); }
}

uint16_t SplashScreen::blendTowardBlack(uint16_t color, float alpha) {
    alpha = clamp01(alpha);
    const uint16_t r = static_cast<uint16_t>(((color >> 11) & 0x1F) * alpha);
    const uint16_t g = static_cast<uint16_t>(((color >> 5) & 0x3F) * alpha);
    const uint16_t b = static_cast<uint16_t>((color & 0x1F) * alpha);
    return static_cast<uint16_t>((r << 11) | (g << 5) | b);
}

void SplashScreen::drawSegmentedRing(float phase, int radius, uint16_t color) {
    auto& d = display::DisplayManager::getInstance();
    const int cx = d.width() / 2;
    const int cy = 78;

    constexpr int SEGMENTS = 14;
    constexpr float SEGMENT_SPAN = 0.22f;
    for (int i = 0; i < SEGMENTS; ++i) {
        const float base = phase + (2.0f * PI * i / SEGMENTS);
        const float a0 = base;
        const float a1 = base + SEGMENT_SPAN;
        const int x0 = cx + static_cast<int>(std::cos(a0) * radius);
        const int y0 = cy + static_cast<int>(std::sin(a0) * radius);
        const int x1 = cx + static_cast<int>(std::cos(a1) * radius);
        const int y1 = cy + static_cast<int>(std::sin(a1) * radius);
        d.drawLine(x0, y0, x1, y1, color);
    }
}

void SplashScreen::drawIntro(float reveal, float titleAlpha, float ringPhase) const {
    auto& d = display::DisplayManager::getInstance();
    reveal = clamp01(reveal);
    titleAlpha = clamp01(titleAlpha);

    d.startWrite();
    d.clear(BLACK);

    const uint16_t ringColor = blendTowardBlack(PURPLE, reveal * 0.75f);
    drawSegmentedRing(ringPhase, 64, ringColor);
    drawSegmentedRing(-ringPhase * 0.7f, 72, blendTowardBlack(PURPLE, reveal * 0.35f));

    const int mascotX = (d.width() - assets::MASCOT_WIDTH) / 2;
    const int mascotY = 28;
    if (reveal > 0.08f) {
        d.drawBitmap(mascotX, mascotY, assets::MASCOT_WIDTH, assets::MASCOT_HEIGHT,
                     assets::MASCOT_RGB565);
    }

    const uint16_t titleColor = blendTowardBlack(WHITE, titleAlpha);
    const uint16_t subColor = blendTowardBlack(PURPLE, titleAlpha);

    const char* title = "ESPION";
    const char* subtitle = "ENGINEERED BY ESPADA";
    const int titleX = (d.width() - d.textWidth(title, 3)) / 2;
    const int subtitleX = (d.width() - d.textWidth(subtitle, 1)) / 2;

    // Title overlaps the lower ring, but stays below the mascot face.
    d.drawText(titleX, 136, title, titleColor, 3);
    d.drawText(subtitleX, 172, subtitle, subColor, 1);

    d.endWrite();
}

void SplashScreen::drawLoading(float progress, const char* statusText) const {
    auto& d = display::DisplayManager::getInstance();
    progress = clamp01(progress);

    d.startWrite();
    d.clear(BLACK);

    const int mascotX = (d.width() - assets::MASCOT_WIDTH) / 2;
    d.drawBitmap(mascotX, 18, assets::MASCOT_WIDTH, assets::MASCOT_HEIGHT,
                 assets::MASCOT_RGB565);

    const int barW = 210;
    const int barH = 8;
    const int barX = (d.width() - barW) / 2;
    const int barY = 172;
    d.drawRect(barX, barY, barW, barH, blendTowardBlack(PURPLE, 0.45f));
    d.fillRect(barX + 2, barY + 2,
               static_cast<int>((barW - 4) * progress), barH - 4, PURPLE);

    if (statusText != nullptr) {
        const int textX = (d.width() - d.textWidth(statusText, 1)) / 2;
        d.drawText(textX, 194, statusText, WHITE, 1);
    }
    d.endWrite();
}

void SplashScreen::drawSystemReady(float alpha) const {
    auto& d = display::DisplayManager::getInstance();
    alpha = clamp01(alpha);
    d.startWrite();
    d.clear(BLACK);

    const int mascotX = (d.width() - assets::MASCOT_WIDTH) / 2;
    d.drawBitmap(mascotX, 28, assets::MASCOT_WIDTH, assets::MASCOT_HEIGHT,
                 assets::MASCOT_RGB565);

    const char* text = "SYSTEM READY";
    const int textX = (d.width() - d.textWidth(text, 2)) / 2;
    d.drawText(textX, 170, text, blendTowardBlack(PURPLE, alpha), 2);
    d.endWrite();
}

} // namespace espion::ui
