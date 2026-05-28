#include "camera.hpp"
#include "cube.hpp"
#include "material.hpp"
#include "ray.hpp"
#include "sphere.hpp"
#include "triangle.hpp"
#include "vec3.hpp"
//#include "point_light.hpp"
#include "random_prob.hpp"

#include <algorithm>
#include <ranges>
#include <stack>


using namespace cg_raytracing::scene;

Camera::Camera(uint32_t _sensor_size_width, uint32_t _focal_length,
               uint32_t _image_width, uint32_t _image_height,
               const float *_position, const float *_direction)
    : m_sensor_size_width(_sensor_size_width),
      m_sensor_size_height(_sensor_size_width / Config::ASPECT_RATIO),
      m_focal_length(_focal_length), m_image_width(_image_width),
      m_image_height(_image_height),
      m_position(_position[0], _position[1], _position[2]),
      m_direction(_direction[0], _direction[1], _direction[2]),
      m_threads{} {

    // get the top left corner from which calculate all the other rays direction
    math::Vec3 top_left =
        math::Vec3(-1 * this->m_sensor_size_width / 2,
                   -1 * this->m_sensor_size_height / 2, _focal_length);
    top_left += this->m_position;

    // get the offset for each ray for each pixel
    math::Vec3 horizontal_offset =
        math::Vec3(this->m_sensor_size_width / m_image_width, 0, 0);
    math::Vec3 vertical_offset =
        math::Vec3(0, this->m_sensor_size_height / m_image_height, 0);

    // calculate the direction of the rays for each pixel
    for (uint32_t y = 0; y < this->m_image_height; y++) {
        for (uint32_t x = 0; x < this->m_image_width; x++) {
            for (uint32_t ray_index = 0; ray_index < Config::RAY_PER_PIXEL; ray_index++) {
                auto ray_direction = math::GetRandomVecAroundPoint(math::Vec3(x, y, m_position.z),
                    top_left, horizontal_offset.x, vertical_offset.y);
                ray_direction.Rotate(this->m_direction);
                this->m_rays_matrix[(y * this->m_image_width + x) * Config::RAY_PER_PIXEL + ray_index].SetDirection(
                    ray_direction);
                this->m_rays_matrix[(y * this->m_image_width + x) * Config::RAY_PER_PIXEL + ray_index].SetOrigin(
                    this->m_position);
            }
            //math::Vec3 ray_direction =
            //    top_left + horizontal_offset * x + vertical_offset * y;
            //ray_direction.Rotate(this->m_direction);
            //this->m_rays_matrix[(y * this->m_image_width + x) * Config::RAY_PER_PIXEL].SetDirection(
            //    ray_direction);
            //this->m_rays_matrix[(y * this->m_image_width + x) * Config::RAY_PER_PIXEL].SetOrigin(
            //    this->m_position);
        }
    }

    auto num_threads = std::thread::hardware_concurrency() * 2;
    m_threads.reserve(num_threads);
    for (auto i : std::views::iota(0U, num_threads)) {
        RenderThreadData th_data{};
        th_data.th = std::jthread([this, i](std::stop_token tok) {
            this->RenderThreadMain(tok, i);
        });
        m_threads.push_back(std::move(th_data));
    }
}

void Camera::RenderThreadMain(std::stop_token _tok, uint32_t _index) {
    std::println(std::cout, "Thread {} started", _index);
    while (!_tok.stop_requested()) {
        m_threads[_index].start_sema.s.acquire();
        if (_tok.stop_requested()) {
            break;
        }
        RenderThreadRender(m_threads[_index]);
        m_threads[_index].finish_sema.s.release();
    }
    std::println(std::cout, "Thread {} stopped", _index);
}

void Camera::RenderThreadRender(RenderThreadData& _data) {
    for (auto const& param : _data.params) {
        RenderThreadRenderBlock(_data, param);
    }
    _data.params.clear();
}

void Camera::RenderThreadRenderBlock(RenderThreadData const& _data, RenderParam _param) {
    auto world = _param.world;
    //auto light = _param.light;

    auto start_x = _param.pos_x;
    auto start_y = _param.pos_y;
    auto end_x = start_x + _param.size_x;
    auto end_y = start_y + _param.size_y;

    //std::println(std::cout, "From X: {}, Y: {} to X: {}, Y: {}", start_x, start_y, end_x, end_y);

    for (uint32_t y = start_y; y < end_y; y++) {
        for (uint32_t x = start_x; x < end_x; x++) {

            uint32_t base_idx =
                (y * this->m_image_width + x) * 3;

            uint32_t r{}, g{}, b{};

            for (uint32_t ray_index = 0; ray_index < Config::RAY_PER_PIXEL; ray_index++) {
                

                const math::Ray& ray =
                    this->m_rays_matrix[
                        (y * this->m_image_width + x) * Config::RAY_PER_PIXEL + ray_index
                    ];

                float t =
                    static_cast<float>(y) /
                    this->m_image_height;

                for (auto curr_iteration : std::views::iota(0U, Config::RENDER_ITERATION)) {
                    // Stack of rays, together with the iteration number
                    std::stack<std::pair<math::Ray, size_t>> rays_to_follow{};
                    rays_to_follow.push({ ray, 0 });

                    auto final_color = math::Vec3(1.f, 1.f, 1.f);

                    while (!rays_to_follow.empty()) {
                        auto [curr_ray, iteration] = rays_to_follow.top();
                        rays_to_follow.pop();

                        std::optional<geometry::HitRecord> hit{};
                        hit = world->Hit(curr_ray);

                        if (hit) {
                            hit->m_point = math::Ray(hit->m_point, hit->m_normal).At(geometry::Hittable::TMIN * 1.1f);
                            auto scattered = hit->m_material->Scatter(curr_ray, hit.value());

                            if (!scattered.has_value()) {
                                // Check if the surface is emissive — if so, accumulate its emission.
                                // Otherwise treat as total absorption (e.g. a black hole material).
                                if (hit->m_material->IsEmissive()) {
                                    // Shade with no external light — emissive materials
                                    // return their own emission color from Shade()
                                    auto emission = hit->m_material->Shade(
                                        hit.value(), {}, {}, 0.0f, curr_ray
                                    );
                                    final_color = final_color * emission;
                                } else {
                                    final_color = math::Vec3(); // Total absorption
                                }
                                break;
                            }

                            auto [direction, albedo] = scattered.value();

                            final_color = final_color * albedo;

                            if (Config::MAX_DEPTH == iteration + 1) {
                                continue;
                            }

                            rays_to_follow.push({ direction, iteration + 1 });
                        }
                    }

                    // if(0 == num_accum) {
                    final_color = final_color * math::Vec3(0.f, ((1.0f - t) * 180 + t * 80) / 255.f, 1.f);
                    // }


                    // Clamp to [0,1]
                    final_color.x = std::clamp(final_color.x, 0.0f, 1.0f);
                    final_color.y = std::clamp(final_color.y, 0.0f, 1.0f);
                    final_color.z = std::clamp(final_color.z, 0.0f, 1.0f);

                    // RGB output
                    r += static_cast<uint8_t>(final_color.x * 255.0f);
                    g += static_cast<uint8_t>(final_color.y * 255.0f);
                    b += static_cast<uint8_t>(final_color.z * 255.0f);
                }

                
            }

            r /= Config::RAY_PER_PIXEL;
            g /= Config::RAY_PER_PIXEL;
            b /= Config::RAY_PER_PIXEL;

            this->m_img_buf[base_idx + 0] = r / Config::RENDER_ITERATION;
            this->m_img_buf[base_idx + 1] = g / Config::RENDER_ITERATION;
            this->m_img_buf[base_idx + 2] = b / Config::RENDER_ITERATION;
        }
    }
}

void Camera::Rotate(const math::Vec3 &_rotation_angles) {
    this->m_direction.Rotate(_rotation_angles);

    for (auto &Ray : this->m_rays_matrix) {
        Ray.Rotate(_rotation_angles);
    }
}

void Camera::Translate(const math::Vec3 &_translation_vector) {
    this->m_position += _translation_vector;
    for (auto &Ray : this->m_rays_matrix) {
        Ray.Translate(_translation_vector);
    }
}

void Camera::BurstRays(/*PointLight& _light,*/ World const& _world) {
    const auto NUM_ROWS_PER_THREAD = m_image_height / m_threads.size();
    const auto NUM_COLS_PER_THREAD = m_image_width / m_threads.size();

    std::vector<std::vector<RenderParam>> params{};

    for (auto i : std::views::iota(0ULL, m_threads.size())) {
        auto start_x = 0;
        auto end_x = m_image_width;
        auto size_x = end_x - start_x;

        auto start_y = NUM_ROWS_PER_THREAD * i;
        auto end_y = std::min<size_t>(start_y + NUM_ROWS_PER_THREAD, m_image_height);
        auto size_y = end_y - start_y;

        auto param = RenderParam{};
        //param.light = &_light;
        param.world = &_world;
        param.pos_x = start_x;
        param.size_x = size_x;
        param.pos_y = start_y;
        param.size_y = size_y;
        params.push_back({ param });
    }

    for (auto i : std::views::iota(0ULL, m_threads.size())) {
        m_threads[i].params = params[i];
        m_threads[i].start_sema.s.release();
    }

    for (auto i : std::views::iota(0ULL, m_threads.size())) {
        m_threads[i].finish_sema.s.acquire();
    }
}

Camera::~Camera() {
    for (auto& th : m_threads) {
        auto stop = th.th.get_stop_source();
        stop.request_stop();
        th.start_sema.s.release();
        th.th.join();
    }
}