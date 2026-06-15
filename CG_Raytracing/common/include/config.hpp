#pragma once

#include <cstdint>
namespace Config {
inline constexpr float ASPECT_RATIO = 4.0 / 3.0;
inline constexpr uint32_t IMAGE_WIDTH = 1200;
inline constexpr uint32_t IMAGE_HEIGHT =
    static_cast<uint32_t>(IMAGE_WIDTH / ASPECT_RATIO);

inline constexpr uint32_t FOCAL_LENGTH = 50; // focal length in m
inline constexpr float SENSOR_SIZE_WIDTH =
    36.0; // sensor size in mm (36 is full-frame)

inline constexpr float CAMERA_POSITION[3] = {0.0, 0.0, 0.0};
// x, y, z angle of rotation (in radiants)
inline constexpr float CAMERA_DIRECTION[3] = {0.0, 0.0, 0.0};

inline constexpr uint32_t RAY_PER_PIXEL = 32;
inline constexpr uint32_t RENDER_ITERATION = 300;
inline constexpr uint32_t MAX_DEPTH = 50;
} // namespace Config
