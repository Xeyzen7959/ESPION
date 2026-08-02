/**
 * ============================================================================
 *  ESPION Firmware — Desktop Simulator
 *  Engineered by Espada
 *  https://github.com/Xeyzen7959
 * ============================================================================
 *
 *  macOS desktop simulator for the ESPION boot sequence and menu UI.
 *  The visual language uses original rough-tech linework, asymmetrical marks,
 *  scan accents, and pixel-art styling. It does not copy third-party logos,
 *  symbols, or screen layouts.
 *
 *  Controls:
 *    Up / Down        Move menu selection
 *    Enter / Right    Open selected page
 *    Escape / Left    Go back (or exit from the main menu)
 *    R                Replay boot animation
 *    Space            Pause / resume boot animation
 *    F                Toggle fullscreen
 *
 *  Asset:
 *    simulator/assets/mascot.png
 * ============================================================================
 */

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <string>
#include <utility>

#ifndef ESPION_SIM_ASSETS_DIR
#define ESPION_SIM_ASSETS_DIR "."
#endif

namespace {

constexpr int LOGICAL_WIDTH = 320;
constexpr int LOGICAL_HEIGHT = 240;
constexpr int DEFAULT_WINDOW_WIDTH = 960;
constexpr int DEFAULT_WINDOW_HEIGHT = 720;
constexpr float PI = 3.14159265358979323846f;

constexpr SDL_Color BLACK{0x00, 0x00, 0x00, 0xFF};
constexpr SDL_Color WHITE{0xF4, 0xF1, 0xFA, 0xFF};
constexpr SDL_Color PURPLE{0xA8, 0x55, 0xF7, 0xFF};
constexpr SDL_Color BRIGHT_PURPLE{0xC1, 0x8A, 0xFF, 0xFF};
constexpr SDL_Color DIM_PURPLE{0x4E, 0x2A, 0x72, 0xFF};
constexpr SDL_Color DEEP_PURPLE{0x22, 0x12, 0x32, 0xFF};
constexpr SDL_Color PANEL{0x0A, 0x08, 0x0E, 0xFF};
constexpr SDL_Color GRAY{0x42, 0x42, 0x4C, 0xFF};
constexpr SDL_Color MUTED{0x9B, 0x95, 0xA5, 0xFF};
constexpr SDL_Color GREEN{0x7D, 0xF0, 0xB2, 0xFF};

const std::string MASCOT_PATH = std::string(ESPION_SIM_ASSETS_DIR) + "/mascot.png";
const std::string FONT_FALLBACK_PATH = std::string(ESPION_SIM_ASSETS_DIR) + "/font.ttf";

const char* const TITLE_FONT_CANDIDATES[] = {
    "/System/Library/Fonts/Avenir Next Condensed.ttc",
    "/System/Library/Fonts/Supplemental/Arial Narrow.ttf",
    "/System/Library/Fonts/Supplemental/Arial Bold Italic.ttf",
    "/System/Library/Fonts/Helvetica.ttc",
};

const char* const BODY_FONT_CANDIDATES[] = {
    "/System/Library/Fonts/SFNS.ttf",
    "/System/Library/Fonts/Helvetica.ttc",
    "/System/Library/Fonts/Supplemental/Arial.ttf",
};

enum class BootPhase {
    Black,
    Reveal,
    Branding,
    Loading,
    Ready,
};

enum class AppScreen {
    Boot,
    MainMenu,
    Home,
    Wifi,
    Bluetooth,
    Logs,
    Settings,
    About,
};

struct Durations {
    static constexpr uint64_t BLACK = 500;
    static constexpr uint64_t REVEAL = 1800;
    static constexpr uint64_t BRANDING = 4200;
    static constexpr uint64_t LOADING = 3200;
    static constexpr uint64_t READY = 1800;
};

constexpr std::array<const char*, 6> MENU_LABELS = {
    "HOME",
    "WI-FI ANALYZER",
    "BLUETOOTH SCANNER",
    "LOGS",
    "SETTINGS",
    "ABOUT",
};

constexpr std::array<AppScreen, 6> MENU_SCREENS = {
    AppScreen::Home,
    AppScreen::Wifi,
    AppScreen::Bluetooth,
    AppScreen::Logs,
    AppScreen::Settings,
    AppScreen::About,
};

float clamp01(float value) {
    return std::clamp(value, 0.0f, 1.0f);
}

float easeOutBack(float t) {
    t = clamp01(t);
    constexpr float c1 = 1.70158f;
    constexpr float c3 = c1 + 1.0f;
    return 1.0f + c3 * std::pow(t - 1.0f, 3.0f) + c1 * std::pow(t - 1.0f, 2.0f);
}

float easeInOutCubic(float t) {
    t = clamp01(t);
    return t < 0.5f
        ? 4.0f * t * t * t
        : 1.0f - std::pow(-2.0f * t + 2.0f, 3.0f) / 2.0f;
}

Uint8 toAlpha(float value) {
    return static_cast<Uint8>(std::clamp(value, 0.0f, 255.0f));
}

SDL_Color withAlpha(SDL_Color color, Uint8 alpha) {
    color.a = alpha;
    return color;
}

struct SDLContext {
    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;

    ~SDLContext() {
        if (renderer) SDL_DestroyRenderer(renderer);
        if (window) SDL_DestroyWindow(window);
        TTF_Quit();
        IMG_Quit();
        SDL_Quit();
    }
};

struct Texture {
    SDL_Texture* handle = nullptr;
    int width = 0;
    int height = 0;

    Texture() = default;
    Texture(const Texture&) = delete;
    Texture& operator=(const Texture&) = delete;

    Texture(Texture&& other) noexcept
        : handle(other.handle), width(other.width), height(other.height) {
        other.handle = nullptr;
        other.width = 0;
        other.height = 0;
    }

    Texture& operator=(Texture&& other) noexcept {
        if (this != &other) {
            if (handle) SDL_DestroyTexture(handle);
            handle = other.handle;
            width = other.width;
            height = other.height;
            other.handle = nullptr;
            other.width = 0;
            other.height = 0;
        }
        return *this;
    }

    ~Texture() {
        if (handle) SDL_DestroyTexture(handle);
    }

    bool valid() const { return handle != nullptr; }
};

struct Font {
    TTF_Font* handle = nullptr;

    Font() = default;
    Font(const Font&) = delete;
    Font& operator=(const Font&) = delete;

    Font(Font&& other) noexcept : handle(other.handle) {
        other.handle = nullptr;
    }

    Font& operator=(Font&& other) noexcept {
        if (this != &other) {
            if (handle) TTF_CloseFont(handle);
            handle = other.handle;
            other.handle = nullptr;
        }
        return *this;
    }

    ~Font() {
        if (handle) TTF_CloseFont(handle);
    }

    bool valid() const { return handle != nullptr; }
};

Font loadFontFromCandidates(const char* const* paths, std::size_t count, int size) {
    Font font;
    for (std::size_t i = 0; i < count; ++i) {
        font.handle = TTF_OpenFont(paths[i], size);
        if (font.handle) return font;
    }

    font.handle = TTF_OpenFont(FONT_FALLBACK_PATH.c_str(), size);
    if (!font.handle) {
        std::fprintf(stderr,
            "[ESPION-SIM] No font found. Put a TTF at '%s'. Error: %s\n",
            FONT_FALLBACK_PATH.c_str(), TTF_GetError());
    }
    return font;
}

Texture loadPixelatedTexture(SDL_Renderer* renderer, const std::string& path,
                             int pixelWidth = 88, int pixelHeight = 84) {
    Texture result;

    SDL_Surface* surface = IMG_Load(path.c_str());
    if (!surface) {
        std::fprintf(stderr, "[ESPION-SIM] Failed to load '%s': %s\n",
                     path.c_str(), IMG_GetError());
        return result;
    }

    SDL_Texture* source = SDL_CreateTextureFromSurface(renderer, surface);
    if (!source) {
        std::fprintf(stderr, "[ESPION-SIM] Failed to create source texture: %s\n",
                     SDL_GetError());
        SDL_FreeSurface(surface);
        return result;
    }

    SDL_Texture* pixelated = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_RGBA8888,
        SDL_TEXTUREACCESS_TARGET,
        pixelWidth,
        pixelHeight);

    if (!pixelated) {
        std::fprintf(stderr, "[ESPION-SIM] Failed to create pixel-art texture: %s\n",
                     SDL_GetError());
        SDL_DestroyTexture(source);
        SDL_FreeSurface(surface);
        return result;
    }

    SDL_SetTextureBlendMode(pixelated, SDL_BLENDMODE_BLEND);
#if SDL_VERSION_ATLEAST(2, 0, 12)
    SDL_SetTextureScaleMode(source, SDL_ScaleModeNearest);
    SDL_SetTextureScaleMode(pixelated, SDL_ScaleModeNearest);
#endif

    SDL_Texture* previousTarget = SDL_GetRenderTarget(renderer);
    SDL_SetRenderTarget(renderer, pixelated);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    SDL_RenderClear(renderer);

    SDL_Rect lowResDestination{0, 0, pixelWidth, pixelHeight};
    SDL_RenderCopy(renderer, source, nullptr, &lowResDestination);
    SDL_SetRenderTarget(renderer, previousTarget);

    result.handle = pixelated;
    result.width = surface->w;
    result.height = surface->h;

    SDL_DestroyTexture(source);
    SDL_FreeSurface(surface);
    return result;
}

Texture makeText(SDL_Renderer* renderer, TTF_Font* font, const char* text, SDL_Color color) {
    Texture texture;
    if (!font) return texture;

    SDL_Surface* surface = TTF_RenderUTF8_Blended(font, text, color);
    if (!surface) {
        std::fprintf(stderr, "[ESPION-SIM] Text render failed: %s\n", TTF_GetError());
        return texture;
    }

    texture.width = surface->w;
    texture.height = surface->h;
    texture.handle = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);

    if (texture.handle) SDL_SetTextureBlendMode(texture.handle, SDL_BLENDMODE_BLEND);
    return texture;
}

void drawTexture(SDL_Renderer* renderer, Texture& texture, int centerX, int centerY,
                 float scale, Uint8 alpha) {
    if (!texture.valid() || scale <= 0.0f || alpha == 0) return;

    SDL_SetTextureAlphaMod(texture.handle, alpha);
    const int width = static_cast<int>(texture.width * scale);
    const int height = static_cast<int>(texture.height * scale);
    SDL_Rect dst{centerX - width / 2, centerY - height / 2, width, height};
    SDL_RenderCopy(renderer, texture.handle, nullptr, &dst);
}

void drawTextAt(SDL_Renderer* renderer, TTF_Font* font, const char* text,
                int x, int y, SDL_Color color, bool centered = false) {
    Texture texture = makeText(renderer, font, text, color);
    if (!texture.valid()) return;
    const int centerX = centered ? x : x + texture.width / 2;
    drawTexture(renderer, texture, centerX, y + texture.height / 2, 1.0f, color.a);
}

void drawArc(SDL_Renderer* renderer, int cx, int cy, float radius,
             float startAngle, float endAngle, SDL_Color color, int thickness = 1) {
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);

    constexpr int segments = 80;
    for (int t = 0; t < thickness; ++t) {
        const float r = radius + static_cast<float>(t);
        int previousX = static_cast<int>(cx + std::cos(startAngle) * r);
        int previousY = static_cast<int>(cy + std::sin(startAngle) * r);

        for (int i = 1; i <= segments; ++i) {
            const float f = static_cast<float>(i) / segments;
            const float angle = startAngle + (endAngle - startAngle) * f;
            const int x = static_cast<int>(cx + std::cos(angle) * r);
            const int y = static_cast<int>(cy + std::sin(angle) * r);
            SDL_RenderDrawLine(renderer, previousX, previousY, x, y);
            previousX = x;
            previousY = y;
        }
    }
}

void drawOrbitDecoration(SDL_Renderer* renderer, uint64_t timeMs, Uint8 alpha,
                         int cx = 160, int cy = 86, float scale = 1.0f) {
    const float time = static_cast<float>(timeMs) / 1000.0f;
    SDL_Color faint = withAlpha(DIM_PURPLE, alpha);
    SDL_Color bright = withAlpha(PURPLE, alpha);

    const float rotation = time * 0.35f;
    drawArc(renderer, cx, cy, 64.0f * scale, rotation + 0.1f, rotation + 1.8f, faint, 1);
    drawArc(renderer, cx, cy, 64.0f * scale, rotation + 2.4f, rotation + 4.3f, bright, 1);
    drawArc(renderer, cx, cy, 70.0f * scale, -rotation + 0.6f, -rotation + 2.0f, faint, 1);
    drawArc(renderer, cx, cy, 70.0f * scale, -rotation + 3.0f, -rotation + 5.5f, faint, 1);
}

void drawRoughFrame(SDL_Renderer* renderer, uint64_t timeMs, Uint8 alpha = 155) {
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_Color line = withAlpha(DIM_PURPLE, alpha);
    SDL_Color hot = withAlpha(PURPLE, static_cast<Uint8>(std::min<int>(255, alpha + 40)));

    const int jitter = static_cast<int>((timeMs / 220) % 3) - 1;

    SDL_SetRenderDrawColor(renderer, line.r, line.g, line.b, line.a);
    SDL_RenderDrawLine(renderer, 13, 15 + jitter, 55, 15 + jitter);
    SDL_RenderDrawLine(renderer, 13, 15 + jitter, 13, 35 + jitter);
    SDL_RenderDrawLine(renderer, 307, 15 - jitter, 270, 15 - jitter);
    SDL_RenderDrawLine(renderer, 307, 15 - jitter, 307, 35 - jitter);
    SDL_RenderDrawLine(renderer, 13, 225 - jitter, 48, 225 - jitter);
    SDL_RenderDrawLine(renderer, 13, 225 - jitter, 13, 209 - jitter);
    SDL_RenderDrawLine(renderer, 307, 225 + jitter, 276, 225 + jitter);
    SDL_RenderDrawLine(renderer, 307, 225 + jitter, 307, 209 + jitter);

    SDL_SetRenderDrawColor(renderer, hot.r, hot.g, hot.b, hot.a);
    SDL_RenderDrawLine(renderer, 23, 43, 45, 43);
    SDL_RenderDrawLine(renderer, 275, 196, 297, 196);
    SDL_RenderDrawPoint(renderer, 27, 191);
    SDL_RenderDrawPoint(renderer, 291, 54);
    SDL_RenderDrawPoint(renderer, 38, 70);
    SDL_RenderDrawPoint(renderer, 279, 112);
}

void drawScanlines(SDL_Renderer* renderer, Uint8 alpha = 22) {
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, PURPLE.r, PURPLE.g, PURPLE.b, alpha);
    for (int y = 1; y < LOGICAL_HEIGHT; y += 4) {
        SDL_RenderDrawLine(renderer, 0, y, LOGICAL_WIDTH, y);
    }
}

void drawTopBar(SDL_Renderer* renderer, TTF_Font* font, const char* section,
                uint64_t timeMs) {
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, DEEP_PURPLE.r, DEEP_PURPLE.g, DEEP_PURPLE.b, 235);
    SDL_Rect bar{0, 0, LOGICAL_WIDTH, 27};
    SDL_RenderFillRect(renderer, &bar);

    SDL_SetRenderDrawColor(renderer, PURPLE.r, PURPLE.g, PURPLE.b, 220);
    SDL_RenderDrawLine(renderer, 0, 26, LOGICAL_WIDTH, 26);

    drawTextAt(renderer, font, "ESPION", 10, 6, PURPLE);
    drawTextAt(renderer, font, section, 82, 6, WHITE);

    const int batteryWidth = 22;
    const int batteryX = 286;
    const int batteryY = 9;
    SDL_SetRenderDrawColor(renderer, WHITE.r, WHITE.g, WHITE.b, 210);
    SDL_Rect battery{batteryX, batteryY, batteryWidth, 9};
    SDL_RenderDrawRect(renderer, &battery);
    SDL_Rect nub{batteryX + batteryWidth + 1, batteryY + 2, 2, 5};
    SDL_RenderFillRect(renderer, &nub);
    const int charge = 13 + static_cast<int>((std::sin(timeMs * 0.0004f) + 1.0f) * 2.0f);
    SDL_Rect fill{batteryX + 2, batteryY + 2, charge, 5};
    SDL_SetRenderDrawColor(renderer, PURPLE.r, PURPLE.g, PURPLE.b, 255);
    SDL_RenderFillRect(renderer, &fill);
}

void drawFooter(SDL_Renderer* renderer, TTF_Font* font,
                const char* left, const char* right) {
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, DEEP_PURPLE.r, DEEP_PURPLE.g, DEEP_PURPLE.b, 225);
    SDL_Rect footer{0, 218, LOGICAL_WIDTH, 22};
    SDL_RenderFillRect(renderer, &footer);

    SDL_SetRenderDrawColor(renderer, DIM_PURPLE.r, DIM_PURPLE.g, DIM_PURPLE.b, 220);
    SDL_RenderDrawLine(renderer, 0, 218, LOGICAL_WIDTH, 218);
    drawTextAt(renderer, font, left, 10, 224, MUTED);

    Texture rightText = makeText(renderer, font, right, MUTED);
    if (rightText.valid()) {
        drawTexture(renderer, rightText, LOGICAL_WIDTH - 10 - rightText.width / 2,
                    224 + rightText.height / 2, 1.0f, 255);
    }
}

void drawLoadingBar(SDL_Renderer* renderer, float progress) {
    progress = clamp01(progress);
    constexpr int x = 72;
    constexpr int y = 215;
    constexpr int width = 176;
    constexpr int height = 5;

    SDL_SetRenderDrawColor(renderer, GRAY.r, GRAY.g, GRAY.b, 255);
    SDL_Rect outline{x, y, width, height};
    SDL_RenderDrawRect(renderer, &outline);

    const int fillWidth = static_cast<int>((width - 2) * progress);
    if (fillWidth > 0) {
        SDL_SetRenderDrawColor(renderer, PURPLE.r, PURPLE.g, PURPLE.b, 255);
        SDL_Rect fill{x + 1, y + 1, fillWidth, height - 2};
        SDL_RenderFillRect(renderer, &fill);
    }
}

void drawMenuIcon(SDL_Renderer* renderer, int x, int y, int index, SDL_Color color) {
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);

    switch (index) {
        case 0: { // Home
            SDL_RenderDrawLine(renderer, x, y + 5, x + 6, y);
            SDL_RenderDrawLine(renderer, x + 6, y, x + 12, y + 5);
            SDL_Rect rect{x + 2, y + 5, 8, 7};
            SDL_RenderDrawRect(renderer, &rect);
            break;
        }
        case 1: // Wi-Fi
            drawArc(renderer, x + 6, y + 7, 6.0f, PI + 0.35f, 2.0f * PI - 0.35f, color);
            drawArc(renderer, x + 6, y + 7, 3.5f, PI + 0.45f, 2.0f * PI - 0.45f, color);
            SDL_RenderDrawPoint(renderer, x + 6, y + 10);
            break;
        case 2: // Bluetooth-ish original rune
            SDL_RenderDrawLine(renderer, x + 6, y, x + 6, y + 13);
            SDL_RenderDrawLine(renderer, x + 6, y, x + 11, y + 4);
            SDL_RenderDrawLine(renderer, x + 11, y + 4, x + 2, y + 10);
            SDL_RenderDrawLine(renderer, x + 2, y + 3, x + 11, y + 9);
            SDL_RenderDrawLine(renderer, x + 11, y + 9, x + 6, y + 13);
            break;
        case 3: { // Logs
            SDL_Rect rect{x + 2, y, 9, 12};
            SDL_RenderDrawRect(renderer, &rect);
            SDL_RenderDrawLine(renderer, x + 4, y + 3, x + 9, y + 3);
            SDL_RenderDrawLine(renderer, x + 4, y + 6, x + 9, y + 6);
            SDL_RenderDrawLine(renderer, x + 4, y + 9, x + 8, y + 9);
            break;
        }
        case 4: { // Settings
            SDL_Rect rect{x + 3, y + 3, 7, 7};
            SDL_RenderDrawRect(renderer, &rect);
            SDL_RenderDrawLine(renderer, x + 6, y, x + 6, y + 3);
            SDL_RenderDrawLine(renderer, x + 6, y + 10, x + 6, y + 13);
            SDL_RenderDrawLine(renderer, x, y + 6, x + 3, y + 6);
            SDL_RenderDrawLine(renderer, x + 10, y + 6, x + 13, y + 6);
            break;
        }
        default: { // About
            SDL_Rect rect{x + 1, y + 1, 11, 11};
            SDL_RenderDrawRect(renderer, &rect);
            SDL_RenderDrawPoint(renderer, x + 6, y + 4);
            SDL_RenderDrawLine(renderer, x + 6, y + 6, x + 6, y + 10);
            break;
        }
    }
}

void drawMainMenu(SDL_Renderer* renderer, TTF_Font* titleFont, TTF_Font* bodyFont,
                  int selected, uint64_t timeMs) {
    drawTopBar(renderer, bodyFont, "MAIN MENU", timeMs);
    drawRoughFrame(renderer, timeMs, 100);

    const float pulse = 0.5f + 0.5f * std::sin(timeMs * 0.004f);
    SDL_Color marker = withAlpha(PURPLE, static_cast<Uint8>(175 + pulse * 80));

    constexpr int startY = 45;
    constexpr int rowHeight = 26;

    for (int i = 0; i < static_cast<int>(MENU_LABELS.size()); ++i) {
        const int y = startY + i * rowHeight;
        const bool active = i == selected;

        if (active) {
            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
            SDL_SetRenderDrawColor(renderer, DEEP_PURPLE.r, DEEP_PURPLE.g, DEEP_PURPLE.b, 245);
            SDL_Rect selection{23, y - 3, 274, 22};
            SDL_RenderFillRect(renderer, &selection);

            SDL_SetRenderDrawColor(renderer, marker.r, marker.g, marker.b, marker.a);
            SDL_RenderDrawLine(renderer, 23, y - 3, 23, y + 18);
            SDL_RenderDrawLine(renderer, 23, y - 3, 33, y - 3);
            SDL_RenderDrawLine(renderer, 23, y + 18, 33, y + 18);
            SDL_RenderDrawLine(renderer, 294, y - 3, 297, y + 8);
            SDL_RenderDrawLine(renderer, 297, y + 8, 294, y + 18);
        }

        drawMenuIcon(renderer, 36, y + 1, i, active ? PURPLE : MUTED);
        drawTextAt(renderer, titleFont, MENU_LABELS[i], 58, y,
                   active ? WHITE : MUTED);

        if (active) {
            drawTextAt(renderer, bodyFont, "OPEN  >", 244, y + 3, PURPLE);
        }
    }

    drawFooter(renderer, bodyFont, "UP/DOWN  NAVIGATE", "ENTER  SELECT");
    drawScanlines(renderer, 12);
}

void drawPanel(SDL_Renderer* renderer, int x, int y, int width, int height,
               SDL_Color border = DIM_PURPLE) {
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, PANEL.r, PANEL.g, PANEL.b, 235);
    SDL_Rect fill{x, y, width, height};
    SDL_RenderFillRect(renderer, &fill);
    SDL_SetRenderDrawColor(renderer, border.r, border.g, border.b, 220);
    SDL_RenderDrawRect(renderer, &fill);
}

void drawSignalBars(SDL_Renderer* renderer, int x, int y, int strength, SDL_Color color) {
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    for (int i = 0; i < 4; ++i) {
        const int h = 3 + i * 3;
        SDL_Rect bar{x + i * 4, y + 12 - h, 2, h};
        if (i < strength) SDL_RenderFillRect(renderer, &bar);
        else SDL_RenderDrawRect(renderer, &bar);
    }
}

void drawHomePage(SDL_Renderer* renderer, Texture& mascot,
                  TTF_Font* titleFont, TTF_Font* bodyFont, uint64_t timeMs) {
    drawTopBar(renderer, bodyFont, "HOME", timeMs);
    drawRoughFrame(renderer, timeMs, 85);

    drawOrbitDecoration(renderer, timeMs, 90, 78, 90, 0.62f);
    drawTexture(renderer, mascot, 78, 90, 0.23f, 255);

    drawTextAt(renderer, titleFont, "SYSTEM ONLINE", 142, 49, WHITE);
    drawTextAt(renderer, bodyFont, "ESP32-S3 UI SIMULATOR", 143, 77, PURPLE);

    drawPanel(renderer, 136, 100, 162, 80);
    drawTextAt(renderer, bodyFont, "BATTERY", 148, 111, MUTED);
    drawTextAt(renderer, titleFont, "SIM  87%", 148, 126, WHITE);
    drawTextAt(renderer, bodyFont, "STORAGE", 148, 153, MUTED);
    drawTextAt(renderer, bodyFont, "READY / MOCK MODE", 213, 153, GREEN);

    drawPanel(renderer, 22, 177, 276, 31, PURPLE);
    drawTextAt(renderer, bodyFont, "STATUS", 31, 186, PURPLE);
    drawTextAt(renderer, bodyFont, "ALL CORE MODULES RESPONDING", 84, 186, WHITE);

    drawFooter(renderer, bodyFont, "ESC  BACK", "R  REPLAY BOOT");
    drawScanlines(renderer, 10);
}

void drawWifiPage(SDL_Renderer* renderer, TTF_Font* titleFont,
                  TTF_Font* bodyFont, uint64_t timeMs) {
    drawTopBar(renderer, bodyFont, "WI-FI ANALYZER", timeMs);
    drawRoughFrame(renderer, timeMs, 75);

    drawTextAt(renderer, titleFont, "PASSIVE SCAN", 22, 38, WHITE);
    drawTextAt(renderer, bodyFont, "MOCK DATA / NO RADIO ACCESS", 22, 65, PURPLE);

    struct NetworkRow { const char* ssid; const char* channel; int strength; };
    constexpr std::array<NetworkRow, 4> rows{{
        {"ESPADA-LAB", "CH 06", 4},
        {"CAMPUS-NET", "CH 11", 3},
        {"NODE-7F2A", "CH 01", 2},
        {"HIDDEN", "CH 09", 1},
    }};

    for (int i = 0; i < static_cast<int>(rows.size()); ++i) {
        const int y = 91 + i * 27;
        drawPanel(renderer, 22, y, 276, 22, i == 0 ? PURPLE : DIM_PURPLE);
        drawTextAt(renderer, bodyFont, rows[i].ssid, 32, y + 5, i == 0 ? WHITE : MUTED);
        drawTextAt(renderer, bodyFont, rows[i].channel, 203, y + 5, MUTED);
        drawSignalBars(renderer, 269, y + 5, rows[i].strength,
                       i == 0 ? PURPLE : MUTED);
    }

    drawFooter(renderer, bodyFont, "ESC  BACK", "PASSIVE DISPLAY ONLY");
    drawScanlines(renderer, 10);
}

void drawBluetoothPage(SDL_Renderer* renderer, TTF_Font* titleFont,
                       TTF_Font* bodyFont, uint64_t timeMs) {
    drawTopBar(renderer, bodyFont, "BLUETOOTH SCANNER", timeMs);
    drawRoughFrame(renderer, timeMs, 75);

    drawTextAt(renderer, titleFont, "BLE PROXIMITY", 22, 38, WHITE);
    drawTextAt(renderer, bodyFont, "MOCK DISCOVERY FEED", 22, 65, PURPLE);

    constexpr std::array<const char*, 4> names = {
        "ESPION-BEACON",
        "HEADPHONES-22",
        "UNKNOWN-C4",
        "FITNESS-BAND",
    };
    constexpr std::array<const char*, 4> values = {
        "-38 dBm", "-61 dBm", "-74 dBm", "-82 dBm"
    };

    for (int i = 0; i < 4; ++i) {
        const int y = 91 + i * 27;
        drawPanel(renderer, 22, y, 276, 22, i == 0 ? PURPLE : DIM_PURPLE);
        drawMenuIcon(renderer, 32, y + 4, 2, i == 0 ? PURPLE : MUTED);
        drawTextAt(renderer, bodyFont, names[i], 52, y + 5, i == 0 ? WHITE : MUTED);
        drawTextAt(renderer, bodyFont, values[i], 240, y + 5, MUTED);
    }

    drawFooter(renderer, bodyFont, "ESC  BACK", "NO PAIRING / MOCK MODE");
    drawScanlines(renderer, 10);
}

void drawLogsPage(SDL_Renderer* renderer, TTF_Font* titleFont,
                  TTF_Font* bodyFont, uint64_t timeMs) {
    drawTopBar(renderer, bodyFont, "LOGS", timeMs);
    drawRoughFrame(renderer, timeMs, 70);

    drawTextAt(renderer, titleFont, "SYSTEM TRACE", 22, 38, WHITE);
    drawPanel(renderer, 22, 72, 276, 130);

    constexpr std::array<const char*, 6> logs = {
        "00:00:01  DISPLAY ........ OK",
        "00:00:02  INPUT .......... OK",
        "00:00:03  STORAGE ........ SIM",
        "00:00:04  BATTERY ........ SIM",
        "00:00:05  UI ENGINE ...... OK",
        "00:00:06  MENU READY ..... OK",
    };

    for (int i = 0; i < static_cast<int>(logs.size()); ++i) {
        drawTextAt(renderer, bodyFont, logs[i], 32, 82 + i * 18,
                   i == static_cast<int>(logs.size()) - 1 ? PURPLE : MUTED);
    }

    drawFooter(renderer, bodyFont, "ESC  BACK", "READ-ONLY SIMULATION");
    drawScanlines(renderer, 10);
}

void drawSettingsPage(SDL_Renderer* renderer, TTF_Font* titleFont,
                      TTF_Font* bodyFont, uint64_t timeMs) {
    drawTopBar(renderer, bodyFont, "SETTINGS", timeMs);
    drawRoughFrame(renderer, timeMs, 70);

    drawTextAt(renderer, titleFont, "INTERFACE", 22, 38, WHITE);

    struct SettingRow { const char* name; const char* value; };
    constexpr std::array<SettingRow, 5> settings{{
        {"THEME", "UMBRA PURPLE"},
        {"BRIGHTNESS", "85%"},
        {"ANIMATIONS", "ENABLED"},
        {"SOUND", "OFF"},
        {"SIMULATION", "ACTIVE"},
    }};

    for (int i = 0; i < static_cast<int>(settings.size()); ++i) {
        const int y = 75 + i * 27;
        drawPanel(renderer, 22, y, 276, 22, i == 0 ? PURPLE : DIM_PURPLE);
        drawTextAt(renderer, bodyFont, settings[i].name, 32, y + 5,
                   i == 0 ? WHITE : MUTED);
        Texture value = makeText(renderer, bodyFont, settings[i].value,
                                 i == 0 ? PURPLE : MUTED);
        if (value.valid()) {
            drawTexture(renderer, value, 288 - value.width / 2,
                        y + 5 + value.height / 2, 1.0f, 255);
        }
    }

    drawFooter(renderer, bodyFont, "ESC  BACK", "SETTINGS ARE VISUAL ONLY");
    drawScanlines(renderer, 10);
}

void drawAboutPage(SDL_Renderer* renderer, Texture& mascot,
                   TTF_Font* titleFont, TTF_Font* bodyFont, uint64_t timeMs) {
    drawTopBar(renderer, bodyFont, "ABOUT", timeMs);
    drawRoughFrame(renderer, timeMs, 80);

    drawOrbitDecoration(renderer, timeMs, 90, 77, 92, 0.58f);
    drawTexture(renderer, mascot, 77, 92, 0.21f, 255);

    drawTextAt(renderer, titleFont, "ESPION", 141, 45, WHITE);
    drawTextAt(renderer, bodyFont, "VERSION 0.1.0-ALPHA", 142, 74, PURPLE);
    drawTextAt(renderer, bodyFont, "ENGINEERED BY ESPADA", 142, 97, WHITE);
    drawTextAt(renderer, bodyFont, "GITHUB  @XEYZEN7959", 142, 118, MUTED);
    drawTextAt(renderer, bodyFont, "TARGET  ESP32-S3", 142, 139, MUTED);

    drawPanel(renderer, 22, 174, 276, 34, PURPLE);
    drawTextAt(renderer, bodyFont, "ORIGINAL EMBEDDED UI RESEARCH PROJECT", 34, 184, WHITE);

    drawFooter(renderer, bodyFont, "ESC  BACK", "MIT LICENSE");
    drawScanlines(renderer, 10);
}

void drawPage(SDL_Renderer* renderer, AppScreen screen, Texture& mascot,
              TTF_Font* titleFont, TTF_Font* bodyFont, uint64_t timeMs) {
    switch (screen) {
        case AppScreen::Home:
            drawHomePage(renderer, mascot, titleFont, bodyFont, timeMs);
            break;
        case AppScreen::Wifi:
            drawWifiPage(renderer, titleFont, bodyFont, timeMs);
            break;
        case AppScreen::Bluetooth:
            drawBluetoothPage(renderer, titleFont, bodyFont, timeMs);
            break;
        case AppScreen::Logs:
            drawLogsPage(renderer, titleFont, bodyFont, timeMs);
            break;
        case AppScreen::Settings:
            drawSettingsPage(renderer, titleFont, bodyFont, timeMs);
            break;
        case AppScreen::About:
            drawAboutPage(renderer, mascot, titleFont, bodyFont, timeMs);
            break;
        default:
            break;
    }
}

void toggleFullscreen(SDL_Window* window) {
    const Uint32 flags = SDL_GetWindowFlags(window);
    const bool fullscreen = (flags & SDL_WINDOW_FULLSCREEN_DESKTOP) != 0;
    SDL_SetWindowFullscreen(window, fullscreen ? 0 : SDL_WINDOW_FULLSCREEN_DESKTOP);
}

} // namespace

int main(int, char**) {
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::fprintf(stderr, "[ESPION-SIM] SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }

    const int imageFlags = IMG_INIT_PNG | IMG_INIT_JPG;
    if ((IMG_Init(imageFlags) & imageFlags) != imageFlags) {
        std::fprintf(stderr, "[ESPION-SIM] IMG_Init warning: %s\n", IMG_GetError());
    }

    if (TTF_Init() != 0) {
        std::fprintf(stderr, "[ESPION-SIM] TTF_Init failed: %s\n", TTF_GetError());
        SDL_Quit();
        return 1;
    }

    SDLContext context;
    context.window = SDL_CreateWindow(
        "ESPION Firmware Simulator",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        DEFAULT_WINDOW_WIDTH,
        DEFAULT_WINDOW_HEIGHT,
        SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);

    if (!context.window) {
        std::fprintf(stderr, "[ESPION-SIM] Window creation failed: %s\n", SDL_GetError());
        return 1;
    }

    context.renderer = SDL_CreateRenderer(
        context.window,
        -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    if (!context.renderer) {
        std::fprintf(stderr, "[ESPION-SIM] Renderer creation failed: %s\n", SDL_GetError());
        return 1;
    }

    SDL_RenderSetLogicalSize(context.renderer, LOGICAL_WIDTH, LOGICAL_HEIGHT);
    SDL_RenderSetIntegerScale(context.renderer, SDL_TRUE);

    Texture mascot = loadPixelatedTexture(context.renderer, MASCOT_PATH);
    if (!mascot.valid()) return 1;

    Font bootTitleFont = loadFontFromCandidates(
        TITLE_FONT_CANDIDATES,
        sizeof(TITLE_FONT_CANDIDATES) / sizeof(TITLE_FONT_CANDIDATES[0]),
        40);
    Font menuTitleFont = loadFontFromCandidates(
        TITLE_FONT_CANDIDATES,
        sizeof(TITLE_FONT_CANDIDATES) / sizeof(TITLE_FONT_CANDIDATES[0]),
        16);
    Font subtitleFont = loadFontFromCandidates(
        BODY_FONT_CANDIDATES,
        sizeof(BODY_FONT_CANDIDATES) / sizeof(BODY_FONT_CANDIDATES[0]),
        13);
    Font bodyFont = loadFontFromCandidates(
        BODY_FONT_CANDIDATES,
        sizeof(BODY_FONT_CANDIDATES) / sizeof(BODY_FONT_CANDIDATES[0]),
        11);

    if (!bootTitleFont.valid() || !menuTitleFont.valid() ||
        !subtitleFont.valid() || !bodyFont.valid()) {
        return 1;
    }

    Texture bootTitle = makeText(context.renderer, bootTitleFont.handle, "ESPION", WHITE);
    Texture bootSubtitle = makeText(context.renderer, subtitleFont.handle,
                                    "ENGINEERED BY ESPADA", PURPLE);
    Texture ready = makeText(context.renderer, subtitleFont.handle, "SYSTEM READY", PURPLE);

    const std::array<const char*, 5> statuses = {
        "DISPLAY  OK",
        "INPUT    OK",
        "STORAGE  OK",
        "BATTERY  SIM",
        "UI       OK",
    };

    AppScreen screen = AppScreen::Boot;
    BootPhase bootPhase = BootPhase::Black;
    uint64_t phaseStarted = SDL_GetTicks64();
    uint64_t bootStarted = phaseStarted;
    bool paused = false;
    uint64_t pauseStarted = 0;
    bool running = true;
    int selectedMenu = 0;

    auto restartBoot = [&]() {
        screen = AppScreen::Boot;
        bootPhase = BootPhase::Black;
        phaseStarted = SDL_GetTicks64();
        bootStarted = phaseStarted;
        paused = false;
    };

    SDL_Event event{};
    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            } else if (event.type == SDL_KEYDOWN && event.key.repeat == 0) {
                const SDL_Keycode key = event.key.keysym.sym;

                if (key == SDLK_f) {
                    toggleFullscreen(context.window);
                    continue;
                }

                if (key == SDLK_r) {
                    restartBoot();
                    continue;
                }

                if (screen == AppScreen::Boot) {
                    if (key == SDLK_ESCAPE) {
                        running = false;
                    } else if (key == SDLK_SPACE) {
                        if (paused) {
                            const uint64_t pauseDuration = SDL_GetTicks64() - pauseStarted;
                            phaseStarted += pauseDuration;
                            bootStarted += pauseDuration;
                            paused = false;
                        } else {
                            paused = true;
                            pauseStarted = SDL_GetTicks64();
                        }
                    }
                    continue;
                }

                if (screen == AppScreen::MainMenu) {
                    switch (key) {
                        case SDLK_UP:
                        case SDLK_w:
                            selectedMenu = (selectedMenu - 1 + static_cast<int>(MENU_LABELS.size()))
                                         % static_cast<int>(MENU_LABELS.size());
                            break;
                        case SDLK_DOWN:
                        case SDLK_s:
                            selectedMenu = (selectedMenu + 1)
                                         % static_cast<int>(MENU_LABELS.size());
                            break;
                        case SDLK_RETURN:
                        case SDLK_KP_ENTER:
                        case SDLK_RIGHT:
                            screen = MENU_SCREENS[selectedMenu];
                            break;
                        case SDLK_ESCAPE:
                        case SDLK_LEFT:
                            running = false;
                            break;
                        default:
                            break;
                    }
                } else {
                    if (key == SDLK_ESCAPE || key == SDLK_LEFT || key == SDLK_BACKSPACE) {
                        screen = AppScreen::MainMenu;
                    }
                }
            }
        }

        const uint64_t realNow = SDL_GetTicks64();
        const uint64_t now = (screen == AppScreen::Boot && paused) ? pauseStarted : realNow;

        if (screen == AppScreen::Boot) {
            uint64_t elapsed = now - phaseStarted;

            if (!paused) {
                switch (bootPhase) {
                    case BootPhase::Black:
                        if (elapsed >= Durations::BLACK) {
                            bootPhase = BootPhase::Reveal;
                            phaseStarted = now;
                        }
                        break;
                    case BootPhase::Reveal:
                        if (elapsed >= Durations::REVEAL) {
                            bootPhase = BootPhase::Branding;
                            phaseStarted = now;
                        }
                        break;
                    case BootPhase::Branding:
                        if (elapsed >= Durations::BRANDING) {
                            bootPhase = BootPhase::Loading;
                            phaseStarted = now;
                        }
                        break;
                    case BootPhase::Loading:
                        if (elapsed >= Durations::LOADING) {
                            bootPhase = BootPhase::Ready;
                            phaseStarted = now;
                        }
                        break;
                    case BootPhase::Ready:
                        if (elapsed >= Durations::READY) {
                            screen = AppScreen::MainMenu;
                            selectedMenu = 0;
                        }
                        break;
                }
            }
        }

        SDL_SetRenderDrawColor(context.renderer, BLACK.r, BLACK.g, BLACK.b, 255);
        SDL_RenderClear(context.renderer);

        if (screen == AppScreen::Boot) {
            const uint64_t elapsed = now - phaseStarted;
            const uint64_t animationTime = now - bootStarted;

            if (bootPhase != BootPhase::Black) {
                float reveal = 1.0f;
                if (bootPhase == BootPhase::Reveal) {
                    reveal = easeOutBack(static_cast<float>(elapsed) / Durations::REVEAL);
                }

                float brandingAlpha = 0.0f;
                if (bootPhase == BootPhase::Branding) {
                    const float t = clamp01(static_cast<float>(elapsed) / Durations::BRANDING);
                    if (t < 0.22f) {
                        brandingAlpha = easeInOutCubic(t / 0.22f);
                    } else if (t < 0.78f) {
                        brandingAlpha = 1.0f;
                    } else {
                        brandingAlpha = 1.0f - easeInOutCubic((t - 0.78f) / 0.22f);
                    }
                }

                const float drift = 0.5f + 0.5f * std::sin(animationTime * 0.0012f);
                const float logoScale = (0.34f + 0.003f * drift) * reveal;
                const Uint8 logoAlpha = toAlpha(255.0f * clamp01(reveal));
                const Uint8 orbitAlpha = toAlpha(130.0f * clamp01(reveal));

                drawRoughFrame(context.renderer, animationTime, 85);
                drawOrbitDecoration(context.renderer, animationTime, orbitAlpha);
                drawTexture(context.renderer, mascot, 160, 82, logoScale, logoAlpha);

                if (brandingAlpha > 0.0f && bootTitle.valid()) {
                    drawTexture(context.renderer, bootTitle, 160, 158, 0.72f,
                                toAlpha(255.0f * brandingAlpha));
                }

                if (brandingAlpha > 0.0f && bootSubtitle.valid()) {
                    drawTexture(context.renderer, bootSubtitle, 160, 184, 0.78f,
                                toAlpha(255.0f * brandingAlpha));
                }

                if (bootPhase == BootPhase::Loading) {
                    const float progress = clamp01(
                        static_cast<float>(elapsed) / Durations::LOADING);
                    drawLoadingBar(context.renderer, progress);

                    const int index = std::min(
                        static_cast<int>(progress * statuses.size()),
                        static_cast<int>(statuses.size()) - 1);
                    Texture status = makeText(context.renderer, bodyFont.handle,
                                              statuses[index], WHITE);
                    drawTexture(context.renderer, status, 160, 202, 0.90f, 255);
                }

                if (bootPhase == BootPhase::Ready && ready.valid()) {
                    drawTexture(context.renderer, ready, 160, 202, 0.92f, 255);
                }

                drawScanlines(context.renderer, 10);
            }
        } else if (screen == AppScreen::MainMenu) {
            drawMainMenu(context.renderer, menuTitleFont.handle, bodyFont.handle,
                         selectedMenu, realNow);
        } else {
            drawPage(context.renderer, screen, mascot, menuTitleFont.handle,
                     bodyFont.handle, realNow);
        }

        SDL_RenderPresent(context.renderer);
        SDL_Delay(1);
    }

    return 0;
}
