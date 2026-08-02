# ==============================================================================
#  ESPION Firmware — Desktop Simulator (macOS)
#  Engineered by Espada
#  https://github.com/Xeyzen7959
# ==============================================================================
#
#  This is a fully independent build target from the ESP32-S3 PlatformIO
#  firmware in the repository root. It shares no code, no build system, and
#  no toolchain with the firmware. Its sole purpose is to let the boot
#  sequence and UI design be iterated on quickly on a desktop, without
#  requiring physical ESP32-S3 hardware.
#
#  Target: macOS, Apple Silicon (arm64)
#  Dependencies (via Homebrew): sdl2, sdl2_image, sdl2_ttf
# ==============================================================================

cmake_minimum_required(VERSION 3.20)

project(espion_simulator
    VERSION 0.1.0
    DESCRIPTION "ESPION Firmware — Desktop Boot/UI Simulator"
    LANGUAGES CXX
)

# ------------------------------------------------------------------------------
# Language standard
# ------------------------------------------------------------------------------
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ------------------------------------------------------------------------------
# Apple Silicon / Homebrew path hints
# ------------------------------------------------------------------------------
# Homebrew on Apple Silicon installs to /opt/homebrew (Intel Macs use
# /usr/local). We hint CMAKE_PREFIX_PATH so find_package() locates SDL2 and
# friends without requiring the user to pass -D flags manually.
if(APPLE)
    list(APPEND CMAKE_PREFIX_PATH
        "/opt/homebrew"
        "/opt/homebrew/opt/sdl2"
        "/opt/homebrew/opt/sdl2_image"
        "/opt/homebrew/opt/sdl2_ttf"
    )
endif()

# ------------------------------------------------------------------------------
# Dependencies
# ------------------------------------------------------------------------------
find_package(SDL2 REQUIRED)
find_package(SDL2_image REQUIRED)
find_package(SDL2_ttf REQUIRED)

# ------------------------------------------------------------------------------
# Executable
# ------------------------------------------------------------------------------
add_executable(espion_simulator
    src/main.cpp
)

target_compile_options(espion_simulator PRIVATE
    -Wall
    -Wextra
)

# ------------------------------------------------------------------------------
# Include directories
# ------------------------------------------------------------------------------
# SDL2's CMake config target names vary slightly by Homebrew formula version
# (SDL2::SDL2 vs SDL2::SDL2-static, SDL2_image::SDL2_image, etc). We link the
# imported targets when available and fall back to variables otherwise.
target_include_directories(espion_simulator PRIVATE
    ${SDL2_INCLUDE_DIRS}
    ${SDL2_IMAGE_INCLUDE_DIRS}
    ${SDL2_TTF_INCLUDE_DIRS}
)

# ------------------------------------------------------------------------------
# Link libraries
# ------------------------------------------------------------------------------
if(TARGET SDL2::SDL2)
    target_link_libraries(espion_simulator PRIVATE SDL2::SDL2)
else()
    target_link_libraries(espion_simulator PRIVATE ${SDL2_LIBRARIES})
endif()

if(TARGET SDL2_image::SDL2_image)
    target_link_libraries(espion_simulator PRIVATE SDL2_image::SDL2_image)
else()
    target_link_libraries(espion_simulator PRIVATE ${SDL2_IMAGE_LIBRARIES})
endif()

if(TARGET SDL2_ttf::SDL2_ttf)
    target_link_libraries(espion_simulator PRIVATE SDL2_ttf::SDL2_ttf)
else()
    target_link_libraries(espion_simulator PRIVATE ${SDL2_TTF_LIBRARIES})
endif()

# ------------------------------------------------------------------------------
# Asset path propagation
# ------------------------------------------------------------------------------
# The simulator locates its assets relative to this source directory rather
# than the build directory, so `assets/` does not need to be copied into the
# build tree on every rebuild during development.
target_compile_definitions(espion_simulator PRIVATE
    ESPION_SIM_ASSETS_DIR="${CMAKE_CURRENT_SOURCE_DIR}/assets"
)

# ------------------------------------------------------------------------------
# Output location
# ------------------------------------------------------------------------------
set_target_properties(espion_simulator PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)