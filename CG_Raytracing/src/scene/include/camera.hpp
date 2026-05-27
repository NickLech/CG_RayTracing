#pragma once

#include "point_light.hpp"
#include "TextureFormats.hpp"
#include "config.hpp"
#include "ray.hpp"
#include "vec3.hpp"
#include "hittable.hpp"
#include "kd_tree.hpp"
#include "world.hpp"

#include <array>
#include <cstdint>
#include <iostream>
#include <vector>
#include <memory>
#include <thread>
#include <semaphore>

namespace cg_raytracing {
namespace scene {

class Camera {
  public:
    float m_sensor_size_width;
    float m_sensor_size_height;
    uint32_t m_focal_length;
    uint32_t m_image_width;
    uint32_t m_image_height;

    struct ZeroSema {
        std::binary_semaphore s = std::binary_semaphore{0};

        ZeroSema() : s{0} {}
        ZeroSema(ZeroSema const& _other) : s{0} {}
    };

    struct OneSema {
        std::binary_semaphore s = std::binary_semaphore{1};

        OneSema() : s{1} {}
        OneSema(OneSema const& _other) : s{1} {}
    };
    
    struct RenderParam {
        size_t pos_x, pos_y;
        size_t size_x, size_y;
        PointLight* light;
        World const* world;
    };

    struct RenderThreadData {
        std::jthread             th;
        ZeroSema                 start_sema;
        ZeroSema                 finish_sema;
        std::vector<RenderParam> params;

        RenderThreadData() :
            th{}, 
            start_sema{},
            finish_sema{},
            params{} {}

        RenderThreadData(RenderThreadData&& _prev) noexcept :
            th{std::move(_prev.th)},
            start_sema{},
            finish_sema{},
            params{std::move(_prev.params)} {}
    };

    // hardcoded the size for RGB
    std::array<uint8_t, Config::IMAGE_HEIGHT * Config::IMAGE_WIDTH * 3>
        m_img_buf;

    cg_raytracing::math::Vec3 m_position;
    cg_raytracing::math::Vec3 m_direction;

    // create the rays for each pixel so not to compute them every time we need
    // to render
    std::array<cg_raytracing::math::Ray,
               Config::IMAGE_HEIGHT * Config::IMAGE_WIDTH * Config::RAY_PER_PIXEL>
        m_rays_matrix;

    // vector that connects the camera focal center to the top left corner.
    cg_raytracing::math::Vec3 m_top_left;
    // vector that indicates the offset to move 1px to the right
    cg_raytracing::math::Vec3 m_horizontal_offset;
    // vector that indicates the offset to move 1px to the bottom
    cg_raytracing::math::Vec3 m_vertical_offset;

    std::vector<RenderThreadData> m_threads;

    Camera(uint32_t _sensor_size_width = Config::SENSOR_SIZE_WIDTH,
           uint32_t _focal_length = Config::FOCAL_LENGTH,
           uint32_t _image_width = Config::IMAGE_WIDTH,
           uint32_t _image_height = Config::IMAGE_HEIGHT,
           const float *_position = Config::CAMERA_POSITION,
           const float *_direction = Config::CAMERA_DIRECTION);

    void RenderThreadMain(std::stop_token _tok, uint32_t _index);
    void RenderThreadRender(RenderThreadData& _data);
    void RenderThreadRenderBlock(RenderThreadData const& _data, RenderParam _param);

    void BurstRays(PointLight& _light, 
        World const& _world);

    void Rotate(const math::Vec3 &_rotation_angles);
    void Translate(const math::Vec3 &_translation_vector);

    ~Camera();
};
} // namespace scene
} // namespace cg_raytracing
