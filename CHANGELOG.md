# Changelog

All notable changes to ESPION will be documented in this file.

The format is inspired by Keep a Changelog and follows Semantic Versioning where practical.

---

## [0.4.0] - 2026-08-03

### Added
- Introduced Application architecture.
- Added Scene interface for screen abstraction.
- Added SceneManager responsible for scene registration and switching.
- Added HomeScene as the initial firmware scene.
- Replaced the temporary serial input test with the Application lifecycle.
- Integrated HomeScene into the firmware after boot completion.

### Changed
- Firmware flow is now:

```
Boot
→ Application
→ SceneManager
→ HomeScene
```

### Notes
This marks the transition from a collection of firmware modules into a structured operating system architecture.

---

## [0.3.0] - 2026-08-03

### Added
- Implemented InputManager.
- Active-low button handling.
- Button debounce.
- Long press detection.
- Hold-repeat events.
- Fixed-size event queue.
- Button event abstraction.

### Notes
Input is now fully event-driven instead of directly polling GPIOs throughout the firmware.

---

## [0.2.0] - 2026-08-03

### Added
- BootManager.
- BootAnimator.
- SplashScreen renderer.
- Embedded ESPION boot animation.
- Pixel mascot rendering.
- Loading animation.
- "System Ready" state.

### Changed
- Replaced the original hardware test pattern with the complete ESPION boot experience.

---

## [0.1.0] - 2026-08-02

### Added
- Initial repository structure.
- PlatformIO project.
- ESP32-S3 firmware.
- DisplayManager.
- LovyanGFX integration.
- LGFX_Device abstraction.
- Project documentation.
- Desktop simulator.
- ESPION branding.
- GitHub repository.

### Notes
Initial public version of the ESPION firmware architecture.