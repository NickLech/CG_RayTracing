// CG_Raytracing.cpp : Defines the entry point for the application.
//

#include "CG_Raytracing.h"

#include <GLContext.hpp>
#include <IndexBuffer.hpp>
#include <camera.hpp>
#include <config.hpp>
#include <cube.hpp>
#include <hittable.hpp>
#include <kd_tree.hpp>
#include <mesh.hpp>
#include <sphere.hpp>
#include <triangle.hpp>
#include <vec3.hpp>
// #include <point_light.hpp>
#include <Shader.hpp>
#include <Texture2D.hpp>
#include <VertexBuffer.hpp>
#include <material.hpp>
#include <world.hpp>

#include <GL/glew.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_assert.h>
#include <bit>
#include <chrono>
#include <ctime>
#include <filesystem>
#include <format>
#include <iostream>
#include <memory>
#include <print>

void DebugCallback(GLenum source, GLenum type, GLuint id, GLenum severity,
                   GLsizei length, const GLchar *message,
                   const void *userParam) {
    std::string msg{message, (size_t)length};
    std::println(std::cout, "{}", message);
}

struct Vertex2D {
#pragma pack(push, 1)
    float x, y;
    float u, v;
#pragma pack(pop)

    std::vector<cg_raytracing::VertexAttribute> attributes() const {
        return {cg_raytracing::VertexAttribute{
                    cg_raytracing::VertexAttributeType::FLOAT,
                    (char *)&x - (char *)this},
                cg_raytracing::VertexAttribute{
                    cg_raytracing::VertexAttributeType::FLOAT,
                    (char *)&y - (char *)this},
                cg_raytracing::VertexAttribute{
                    cg_raytracing::VertexAttributeType::FLOAT,
                    (char *)&u - (char *)this},
                cg_raytracing::VertexAttribute{
                    cg_raytracing::VertexAttributeType::FLOAT,
                    (char *)&v - (char *)this}};
    }
};

cg_raytracing::scene::World BuildScene1() {
    using Hittable = cg_raytracing::geometry::Hittable;
    using World = cg_raytracing::scene::World;
    using StandardMaterial = cg_raytracing::geometry::StandardMaterial;
    using DielectricMaterial = cg_raytracing::geometry::DielectricMaterial;
    using EmissiveMaterial = cg_raytracing::geometry::EmissiveMaterial;
    using Sphere = cg_raytracing::geometry::Sphere;
    using Cube = cg_raytracing::geometry::Cube;
    using Mesh = cg_raytracing::geometry::Mesh;
    using Vec3 = cg_raytracing::math::Vec3;

    auto mat_sphere = std::make_shared<StandardMaterial>(
        StandardMaterial::Diffuse({.4f, .4f, .8f}, {.5f, .5f, .5f}));
    auto mat_cube = std::make_shared<StandardMaterial>(
        StandardMaterial::Diffuse({.4f, .4f, .8f}, {.5f, .5f, .5f}));
    auto mat_train = std::make_shared<StandardMaterial>(
        StandardMaterial::Diffuse({0.7f, 0.2f, 0.2f}));

    auto world = World::CreateEmpty(1000.f);

    // Emissive sphere acting as a light source in the scene
    auto mat_light1 = std::make_shared<EmissiveMaterial>(
        EmissiveMaterial::Create({1.0f, 1.0f, 0.0f}, 10000.0f));
    auto mat_light2 = std::make_shared<EmissiveMaterial>(
        EmissiveMaterial::Create({1.0f, 1.0f, 0.0f}, 100.0f));
    // world.AddObject(std::make_shared<Sphere>(
    //     cg_raytracing::math::Vec3(50.0f, -45.0f, 180.0f), 15.f, mat_light1));
    // Transparent sphere acting as a glass ball in the scene

    auto mat_glass = std::make_shared<DielectricMaterial>(
        DielectricMaterial::Create(1.5f, {1.0f, 0.0f, 0.0f}, 0.1f) // glass
    );

    std::shared_ptr<Mesh> table =
        std::make_shared<Mesh>(Vec3(0.0f, 20.0f, 100.0f));
    auto loader_status = table->LoadFromObj("./assets/meshes/Table.obj", 3.0);
    world.AddObject(table);

    world.AddObject(std::make_shared<Sphere>(Vec3(-25.0f, 0.0f, 180.0f), 25.f, mat_glass));

    world.AddObject(std::make_shared<Sphere>(Vec3(0.0f, -45.0f, 180.0f), 15.f, mat_light1));
    world.AddObject(std::make_shared<Sphere>(Vec3(-40.f, 10.0f, 170.0f), 15.f, mat_light2));

    world.AddObject(std::make_shared<Sphere>(Vec3(0.0f, 0.0f, 200.0f), 30.f, mat_sphere));
    world.AddObject(std::make_shared<Cube>(Vec3(0.0f, 85.0f, 200.0f), 50.f, mat_cube));

    world.UpdateTree();
    return world;
}

cg_raytracing::scene::World BuildScene2() {
    // Demo scene: one sphere per material, equally spaced in a row
    using World = cg_raytracing::scene::World;
    using StandardMaterial = cg_raytracing::geometry::StandardMaterial;
    using DielectricMaterial = cg_raytracing::geometry::DielectricMaterial;
    using EmissiveMaterial = cg_raytracing::geometry::EmissiveMaterial;
    using Sphere = cg_raytracing::geometry::Sphere;
    using Vec3 = cg_raytracing::math::Vec3;
    using Cube = cg_raytracing::geometry::Cube;

    auto world = World::CreateEmpty(2000.f);

    const float radius = 20.0f;
    const float spacing = 41.0f; // distance between sphere centers
    const float z = 100.0f;
    const float y = 0.0f;
    // Center the row: 5 spheres, total width = 4 * spacing
    const float x_start = -2.0f * spacing;

    // 1. Diffuse (Lambert) — blue
    auto mat_diffuse = std::make_shared<StandardMaterial>(
        StandardMaterial::Diffuse({0.2f, 0.3f, 0.9f}));

    // 2. Metal, low roughness (sharp reflection) — silver
    auto mat_metal_sharp = std::make_shared<StandardMaterial>(
        StandardMaterial::Metal({0.8f, 0.8f, 0.8f}, {0.95f, 0.1f, 0.1f}, 500.0f));

    // 3. Metal, high roughness (blurry reflection) — gold tint
    auto mat_metal_rough = std::make_shared<StandardMaterial>(
        StandardMaterial::Metal({0.8f, 0.8f, 0.8f}, {0.9f, 0.7f, 0.3f}, 5.0f));

    // 4. Dielectric (glass)
    auto mat_glass = std::make_shared<DielectricMaterial>(
        DielectricMaterial::Create(1.5f, {1.f, 1.f, 1.f}, 0.5f));

    // 5. Emissive (light source)
    auto mat_emissive = std::make_shared<EmissiveMaterial>(
        EmissiveMaterial::Create({1.0f, 0.4f, 0.1f}, 5.0f));

    auto mat_cube = std::make_shared<StandardMaterial>(
        StandardMaterial::Diffuse({ .3f, .9f, .3f }, { .5f, .5f, .5f }));

    // world.AddObject(std::make_shared<Sphere>(Vec3(x_start + 0 * spacing, y, z), radius, mat_diffuse));
    world.AddObject(std::make_shared<Sphere>(Vec3(x_start + 1 * spacing, y, z), radius, mat_metal_sharp));
    world.AddObject(std::make_shared<Sphere>(Vec3(x_start + 2 * spacing, y, z), radius, mat_diffuse));
    world.AddObject(std::make_shared<Sphere>(Vec3(x_start + 3 * spacing, y, z), radius, mat_metal_rough));

    world.AddObject(std::make_shared<Cube>(Vec3(0.0f, 325.0f, 100.0f), 300.f, mat_cube));
    // world.AddObject(std::make_shared<Sphere>(Vec3(x_start + 4 * spacing, y, z), radius, mat_metal_rough));

    world.UpdateTree();
    return world;
}

cg_raytracing::scene::World BuildScene3() {
    // Demo scene: one sphere per material, equally spaced in a row
    using World = cg_raytracing::scene::World;
    using StandardMaterial = cg_raytracing::geometry::StandardMaterial;
    using DielectricMaterial = cg_raytracing::geometry::DielectricMaterial;
    using EmissiveMaterial = cg_raytracing::geometry::EmissiveMaterial;
    using Sphere = cg_raytracing::geometry::Sphere;
    using Vec3 = cg_raytracing::math::Vec3;
    using Cube = cg_raytracing::geometry::Cube;

    auto world = World::CreateEmpty(2000.f);

    const float radius = 20.0f;
    const float spacing = 41.0f; // distance between sphere centers
    const float z = 100.0f;
    const float y = 0.0f;
    // Center the row: 5 spheres, total width = 4 * spacing
    const float x_start = -2.0f * spacing;

    // 1. Diffuse (Lambert) — blue
    auto mat_diffuse1 = std::make_shared<StandardMaterial>(
        StandardMaterial::Diffuse({ 0.2f, 0.3f, 0.9f }));

    // 2. Metal, low roughness (sharp reflection) — silver
    auto mat_metal = std::make_shared<StandardMaterial>(
        StandardMaterial::Metal({ 0.2f, 0.9f, 0.3f }, { 0.95f, 0.1f, 0.1f }, 1.0f));

    // 4. Dielectric (glass)
    auto mat_glass = std::make_shared<DielectricMaterial>(
        DielectricMaterial::Create(0.9f, { 0.9f, 0.9f, 0.9f }, 0.5f));

    auto mat_cube = std::make_shared<StandardMaterial>(
        StandardMaterial::Diffuse({ .3f, .3f, .9f }, { .5f, .5f, .5f }));

    world.AddObject(std::make_shared<Sphere>(Vec3(.0f, .0f, 100.0f), radius, mat_diffuse1));
    world.AddObject(std::make_shared<Sphere>(Vec3(40.5f, -10.0f, 120.5f), radius, mat_metal));
    world.AddObject(std::make_shared<Sphere>(Vec3(-7.5f, 0.f, 60.f), radius, mat_glass));

    world.AddObject(std::make_shared<Cube>(Vec3(0.0f, 320.5f, 100.0f), 300.f, mat_cube));

    world.UpdateTree();
    return world;
}

cg_raytracing::scene::World BuildScene4() {
    // Demo scene: one sphere per material, equally spaced in a row
    using World = cg_raytracing::scene::World;
    using StandardMaterial = cg_raytracing::geometry::StandardMaterial;
    using DielectricMaterial = cg_raytracing::geometry::DielectricMaterial;
    using EmissiveMaterial = cg_raytracing::geometry::EmissiveMaterial;
    using Sphere = cg_raytracing::geometry::Sphere;
    using Vec3 = cg_raytracing::math::Vec3;
    using Cube = cg_raytracing::geometry::Cube;
    using Mesh = cg_raytracing::geometry::Mesh;

    auto world = World::CreateEmpty(5000.f);

    auto ground =
        std::make_shared<Mesh>(Vec3(0.0f, 1000.0f, 0.0f));
    auto loader_status = ground->LoadFromObj("./assets/meshes/ChessBoard.obj", 1000.0);
    world.AddObject(ground);

    // auto ground_material = std::make_shared<StandardMaterial>(StandardMaterial::Diffuse(Vec3(.5f, .5f, .5f)));
    // world.AddObject(std::make_shared<Cube>(Vec3(.0f, 1000.f, .0f), 1000.f, ground_material));
    // 
    auto material1 = std::make_shared<DielectricMaterial>(DielectricMaterial::Create(1.5f, Vec3(1.f, 1.f, 1.f)));
    auto material2 = std::make_shared<StandardMaterial>(StandardMaterial::Diffuse(Vec3(.8f, .2f, .1f), Vec3(.2f, .2f, .2f)));
    auto material3 = std::make_shared<StandardMaterial>(StandardMaterial::Metal(Vec3(.8f, .8f, .8f), Vec3(1.f, 1.f, 1.f), 1000.f));
    
    world.AddObject(std::make_shared<Sphere>(Vec3(.0f, -20.f, .0f), 20.f, material1));
    world.AddObject(std::make_shared<Sphere>(Vec3(-40.0f, -20.f, .0f), 20.f, material2));
    world.AddObject(std::make_shared<Sphere>(Vec3(40.0f, -20.f, .0f), 20.f, material3));
    
    {
        auto material4 = std::make_shared<StandardMaterial>(StandardMaterial::Diffuse(Vec3(.2f, .8f, .2f), Vec3(.4f, .4f, .4f)));
        world.AddObject(std::make_shared<Sphere>(Vec3(-10.f, -5.f, -30.f), 5.f, material4));
    
        auto material5 = std::make_shared<StandardMaterial>(StandardMaterial::Diffuse(Vec3(.2f, .2f, .8f), Vec3(.4f, .4f, .4f)));
        world.AddObject(std::make_shared<Sphere>(Vec3(10.f, -5.f, 30.f), 5.f, material5));
    
        auto material6 = std::make_shared<StandardMaterial>(StandardMaterial::Diffuse(Vec3(.8f, .8f, .3f), Vec3(.5f, .5f, .5f)));
        world.AddObject(std::make_shared<Sphere>(Vec3(0.f, -5.f, 60.f), 5.f, material6));
    
        auto material7 = std::make_shared<StandardMaterial>(StandardMaterial::Diffuse(Vec3(.8f, .2f, .8f), Vec3(.5f, .5f, .5f)));
        world.AddObject(std::make_shared<Sphere>(Vec3(20.f, -5.f, 60.f), 5.f, material7));
    
        auto material8 = std::make_shared<StandardMaterial>(StandardMaterial::Diffuse(Vec3(.8f, .2f, .8f), Vec3(.5f, .5f, .5f)));
        world.AddObject(std::make_shared<Sphere>(Vec3(-20.f, -5.f, 60.f), 5.f, material8));
    
        auto material9 = std::make_shared<StandardMaterial>(StandardMaterial::Diffuse(Vec3(.8f, .5f, .5f), Vec3(.5f, .5f, .5f)));
        world.AddObject(std::make_shared<Sphere>(Vec3(-10.f, -5.f, 90.f), 5.f, material9));
    
        auto material10 = std::make_shared<StandardMaterial>(StandardMaterial::Diffuse(Vec3(.5f, .9f, .5f), Vec3(.5f, .5f, .5f)));
        world.AddObject(std::make_shared<Sphere>(Vec3(5.f, -5.f, 120.f), 5.f, material10));
    
        auto material11 = std::make_shared<StandardMaterial>(StandardMaterial::Metal(Vec3(.8f, .8f, .8f), Vec3(1.f, 1.f, 1.f), 10.f));
        world.AddObject(std::make_shared<Sphere>(Vec3(-60.f, -10.f, -35.f), 10.f, material11));
    
        auto material12 = std::make_shared<DielectricMaterial>(DielectricMaterial::Create(1.02f, Vec3(1.f, 1.f, .3f)));
        world.AddObject(std::make_shared<Sphere>(Vec3(-30.f, -5.f, -65.f), 5.f, material12));
    
        auto material13 = std::make_shared<StandardMaterial>(StandardMaterial::Diffuse(Vec3(1.f, 1.f, .5f), Vec3(.5f, .5f, .5f)));
        world.AddObject(std::make_shared<Sphere>(Vec3(-40.f, -5.f, -50.f), 5.f, material13));
    
        auto material14 = std::make_shared<StandardMaterial>(StandardMaterial::Diffuse(Vec3(.0f, 1.f, 1.f), Vec3(.5f, .5f, .5f)));
        world.AddObject(std::make_shared<Sphere>(Vec3(40.f, -5.f, -50.f), 5.f, material14));
    
        auto material15 = std::make_shared<DielectricMaterial>(DielectricMaterial::Create(0.9f, Vec3(.9f, .9f, .9f)));
        world.AddObject(std::make_shared<Sphere>(Vec3(20.f, -7.5f, -45.f), 7.5f, material15));

        auto material16 = std::make_shared<StandardMaterial>(StandardMaterial::Diffuse(Vec3(1.f, 1.f, 0.f), Vec3(.1f, .1f, .1f)));
        world.AddObject(std::make_shared<Sphere>(Vec3(20.f, -5.f, -80.f), 5.f, material16));
    }
    
    auto light_mat = std::make_shared<EmissiveMaterial>(EmissiveMaterial::Create(Vec3(1.f, 1.f, .3f), 2000.f));
    world.AddObject(std::make_shared<Sphere>(Vec3(.0f, -75.f, .0f), 15.f, light_mat));

    world.UpdateTree();

    return world;
}

// function to move the camera around. Probably in the future is better to move
// this in another file and optimize it
void HandleKeyDown(
    SDL_Event &_ev, std::unique_ptr<cg_raytracing::scene::Camera> &_my_camera,
    cg_raytracing::Texture2D &_tex,
    std::vector<cg_raytracing::scene::World> &_scenes,
    int &_current_scene) {
    SDL_assert(_ev.type == SDL_EVENT_KEY_DOWN);
    auto must_update = false;
    switch (_ev.key.scancode) {
    case SDL_SCANCODE_W:
        _my_camera->Rotate(cg_raytracing::math::Vec3(0.02, 0.0, 0.0));
        must_update = true;
        break;
    case SDL_SCANCODE_A:
        _my_camera->Rotate(cg_raytracing::math::Vec3(0.0, -0.02, 0.0));
        must_update = true;
        break;
    case SDL_SCANCODE_S:
        _my_camera->Rotate(cg_raytracing::math::Vec3(-0.02, 0.0, 0.0));
        must_update = true;
        break;
    case SDL_SCANCODE_D:
        _my_camera->Rotate(cg_raytracing::math::Vec3(0.0, 0.02, 0.0));
        must_update = true;
        break;
    /*case SDL_SCANCODE_J:
        _light.m_position.z -= 50.0f;
        must_update = true;
        break;
    case SDL_SCANCODE_U:
        _light.m_position.z += 50.0f;
        must_update = true;
        break;
    case SDL_SCANCODE_K:
        _light.m_position.x += 50.0f;
        must_update = true;
        break;
    case SDL_SCANCODE_H:
        _light.m_position.x -= 50.0f;
        must_update = true;
        break;
    */
    case SDL_SCANCODE_C: {
        static int s_current_preset = 0;
        s_current_preset = (s_current_preset + 1) % 3;

        const auto &preset = Config::CAMERA_PRESETS[s_current_preset];

        using Camera = cg_raytracing::scene::Camera;

        _my_camera.reset(nullptr);
        _my_camera = std::make_unique<Camera>(
            Config::SENSOR_SIZE_WIDTH, Config::FOCAL_LENGTH,
            Config::IMAGE_WIDTH, Config::IMAGE_HEIGHT, preset[0], preset[1]);

        must_update = true;
        break;
    } // don't remove, may cause errors due to variable declaration in switch

    case SDL_SCANCODE_TAB: {
        // Switch between scenes (Tab key)
        _current_scene = (_current_scene + 1) % static_cast<int>(_scenes.size());
        must_update = true;
        break;
    }

    default:
        break;
    }

    if (must_update) {
        _my_camera->BurstRays(_scenes[_current_scene]);
        _tex.CopyFromBuffer(
            std::bit_cast<uint8_t const *>(_my_camera->m_img_buf.data()), 0, 0,
            0, _tex.GetWidth(), _tex.GetHeight(),
            cg_raytracing::PixelFormat::RGB,
            cg_raytracing::PixelDataType::UNSIGNED_BYTE);
    }
}

int main() {
    using Camera = cg_raytracing::scene::Camera;
    std::unique_ptr<Camera> my_camera = std::make_unique<Camera>();
    my_camera->Translate(cg_raytracing::math::Vec3(.0f, -30.f, -200.0f));

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        std::println(std::cout, "SDL_Init error: {}", SDL_GetError());
        std::exit(1);
    }

    // Set double-buffering on the swapchain (render to one buffer, while
    // another is presented by the window)
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    // Set OpenGL 4.6
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
    // Disable compatibility profile sheganigans
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,
                        SDL_GL_CONTEXT_PROFILE_CORE);

    // SDL_CreateWindow requires SDL_WINDOW_OPENGL to use OpenGL
    auto window = SDL_CreateWindow("Test", 1000, 720,
                                   SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL |
                                       SDL_WINDOW_HIGH_PIXEL_DENSITY);
    if (window == nullptr) {
        std::println(std::cout, "SDL_CreateWindow error: {}", SDL_GetError());
        std::exit(1);
    }

    auto context = SDL_GL_CreateContext(window);
    if (context == nullptr) {
        std::println(std::cout, "SDL_GL_CreateContext error: {}",
                     SDL_GetError());
        std::exit(1);
    }

    auto gl_ctx = cg_raytracing::GLContextWrapper::CreateWrapper(
        context, [](void *window, void *ctx) {
            // To be used, generally a context must be bound to a window
            // This lambda does exactly that
            return SDL_GL_MakeCurrent((SDL_Window *)window, (SDL_GLContext)ctx);
        });
    if (!gl_ctx.MakeCurrent((void *)window)) {
        std::println(std::cout, "SDL_GL_MakeCurrent() error: {}",
                     SDL_GetError());
        std::exit(1);
    }

    glewExperimental = true;
    auto error = glewInit();
    if (error != GLEW_OK) {
        std::println(std::cout, "glewInit error: {}",
                     (const char *)glewGetErrorString(error));
        std::exit(1);
    }

    auto version_string = glewGetString(GLEW_VERSION);
    std::println(std::cout, "GLEW  version {}", (const char *)version_string);

    int32_t major{}, minor{};
    glGetIntegerv(GL_MAJOR_VERSION, &major);
    glGetIntegerv(GL_MINOR_VERSION, &minor);
    std::println(std::cout, "OpenGL {}.{}", major, minor);

    auto work_dir = std::filesystem::current_path();

    std::println(std::cout, "Working directory: {}", work_dir.string());

    using ShaderStage = cg_raytracing::Shader::ShaderStage;

    auto result = cg_raytracing::Shader::CreateShaderFromFiles(
        {{"./assets/main.vert", ShaderStage::VERTEX},
         {"./assets/main.frag", ShaderStage::FRAGMENT}});

    if (!result) {
        // This will now print "Could not find file assets/main.vert"
        // instead of crashing with an IOT instruction
        std::cerr << "Shader Error: " << result.error() << std::endl;
        return EXIT_FAILURE;
    }

    cg_raytracing::Shader shader = std::move(result.value());

    // Disable alpha-blending
    gl_ctx.SetBlendEnable(false);

    // Disable scissor (e.g. clipping outside a specific rect)
    gl_ctx.SetScissorEnable(false);

    // https://wikis.khronos.org/opengl/Debug_Output
    // https://registry.khronos.org/OpenGL-Refpages/gl4/html/glDebugMessageCallback.xhtml
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(DebugCallback, nullptr);

    auto vert_buf = cg_raytracing::VertexBuffer::CreateVertexBuffer().value();
    vert_buf.AddBuffer<Vertex2D>(
        0, 4); // Add a single buffer, containing per-vertex data,
               // reserve space for 4 vertices

    // Index buffer with size for 6 indices
    auto index_buf = cg_raytracing::IndexBuffer::CreateIndexBuffer(6).value();

    // 4 Vertices:
    // X, Y and U, V
    // X, Y are in screen coordinates (in [-1.0, 1.0])
    // U, V are in texture coordinates (in [0.0, 1.0])
    constexpr Vertex2D VERTICES[4] = {{-1.0, 1.0, 0.0, 0.0},
                                      {1.0, 1.0, 1.0, 0.0},
                                      {-1.0, -1.0, 0.0, 1.0},
                                      {1.0, -1.0, 1.0, 1.0}};

    // Indices in the vertex buffer to draw a quad
    constexpr float INDICES[6] = {0, 1, 2, 2, 1, 3};

    for (size_t i = 0; i < 4; i++) {
        vert_buf.PushVertexDataTyped(0, VERTICES[i]);
    }

    for (size_t i = 0; i < 6; i++) {
        index_buf.PushIndex(INDICES[i]);
    }

    // Use the two buffers
    // and the shader
    vert_buf.Bind();
    index_buf.Bind();
    shader.Bind();

    srand(time(0));

    auto tex = cg_raytracing::Texture2D::CreateTexture(
                   1, Config::IMAGE_WIDTH, Config::IMAGE_HEIGHT,
                   cg_raytracing::TextureFormat::RGB8)
                   .value();
    tex.SetUpscaleFilter(cg_raytracing::SamplerFilter::LINEAR);
    tex.SetDownscaleFilter(cg_raytracing::SamplerFilter::LINEAR);

    using Hittable = cg_raytracing::geometry::Hittable;
    using World = cg_raytracing::scene::World;
    using StandardMaterial = cg_raytracing::geometry::StandardMaterial;
    using Sphere = cg_raytracing::geometry::Sphere;
    using Cube = cg_raytracing::geometry::Cube;
    using Mesh = cg_raytracing::geometry::Mesh;
    using Vec3 = cg_raytracing::math::Vec3;

    std::vector<std::shared_ptr<Hittable>> hittables;

    // Build all scenes and store them; Tab switches between them
    std::vector<World> scenes;
    scenes.push_back(BuildScene1());
    scenes.push_back(BuildScene2());
    scenes.push_back(BuildScene3());
    scenes.push_back(BuildScene4());
    int current_scene = 3;

    auto begin = std::chrono::system_clock::now();

    my_camera->BurstRays(scenes[current_scene]);

    auto end = std::chrono::system_clock::now();
    auto diff =
        std::chrono::duration_cast<std::chrono::microseconds>(end - begin)
            .count();
    std::println(std::cout, "Took {} ms", (double)diff / 1e3);

    tex.CopyFromBuffer(
        std::bit_cast<uint8_t const *>(my_camera->m_img_buf.data()), 0, 0, 0,
        tex.GetWidth(), tex.GetHeight(), cg_raytracing::PixelFormat::RGB,
        cg_raytracing::PixelDataType::UNSIGNED_BYTE);
    tex.BindTexture(GL_TEXTURE_2D);

    bool close = false;
    while (!close) {
        SDL_Event ev{};
        while (SDL_PollEvent(&ev)) {
            switch (ev.type) {
            case SDL_EVENT_KEY_DOWN:
                HandleKeyDown(ev, my_camera, tex, scenes, current_scene);
                break;
            case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
                close = true;
                break;
            case SDL_EVENT_WINDOW_RESIZED: {
                int32_t w{}, h{};
                SDL_GetWindowSize(window, &w, &h);
                gl_ctx.SetViewport(
                    {.bottom_left_x = 0, .bottom_left_y = 0, .w = w, .h = h});
            } break;
            default:
                break;
            }
        }

        // glBindFramebuffer(GL_FRAMEBUFFER, 0);
        // Clear the current framebuffer
        // Clearing can be divided in color buffer, depth buffer and stencil
        // buffer
        glClearColor(1.f, 1.f, 1.f,
                     1.f);  // Specify clear color for color buffer
        glClearDepth(1.0f); // Specify clear for depth buffer
        glClear(GL_COLOR_BUFFER_BIT |
                GL_DEPTH_BUFFER_BIT); // Effectively clear color and depth

        // Draw indexed vertices:
        // 6 triangles, drawn using 6 indices
        // from the bound index buffer, starting
        // from offset 0 in the index buffer

        // In short, instead of using directly the vertex buffer
        // for drawing, which would require replicating the vertex
        // data for adjacent vertices, we write that data once
        // in the vertex buffer, and then we write that vertex
        // index multiple times in the index buffer. This
        // can reduce the memory usage by a lot, especially
        // if each vertex contains a lot of data

        // Example: a vertex contains the following data
        // X, Y, Z coordinates -> 12 bytes
        // U, V -> 8 bytes
        // Vertex normal -> 12 bytes
        // (vertex might also contain base color and so on...)
        // For a total of 32 bytes, for a quad we have 6
        // vertices -> 192 bytes
        // But what if we use a index buffer ?
        // 4 unique vertices -> 128 bytes
        // 6 indices, let's say 4 bytes each -> 24 bytes
        // In total 152 bytes
        // 40 bytes less
        // And this is only a very simple example
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (const void *)0);

        SDL_GL_SwapWindow(window);
    }

    SDL_GL_DestroyContext(context);
    SDL_DestroyWindow(window);

    SDL_Quit();
    return 0;
}