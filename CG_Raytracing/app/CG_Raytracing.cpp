// CG_Raytracing.cpp : Defines the entry point for the application.
//

#include "CG_Raytracing.h"

#include <vec3.hpp>
#include <kd_tree.hpp>
#include <hittable.hpp>
#include <triangle.hpp>
#include <sphere.hpp>
#include <cube.hpp>
#include <mesh.hpp>
#include <camera.hpp>
#include <config.hpp>
#include <GLContext.hpp>
#include <IndexBuffer.hpp>
//#include <point_light.hpp>
#include <Shader.hpp>
#include <Texture2D.hpp>
#include <VertexBuffer.hpp>
#include <world.hpp>
#include <material.hpp>

#include <GL/glew.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_assert.h>
#include <bit>
#include <ctime>
#include <filesystem>
#include <format>
#include <iostream>
#include <memory>
#include <print>
#include <chrono>

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

// function to move the camera around. Probably in the future is better to move
// this in another file and optimize it
void HandleKeyDown(SDL_Event &_ev,
                   std::unique_ptr<cg_raytracing::scene::Camera> &_my_camera,
                   cg_raytracing::Texture2D &_tex,
                   cg_raytracing::scene::World const& _world,
                   std::vector<std::shared_ptr<cg_raytracing::geometry::Hittable>> const& _hittables) {
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

        const auto& preset = Config::CAMERA_PRESETS[s_current_preset];

        using Camera = cg_raytracing::scene::Camera;

        _my_camera.reset(nullptr);
        _my_camera = std::make_unique<Camera>(
            Config::SENSOR_SIZE_WIDTH, Config::FOCAL_LENGTH,
            Config::IMAGE_WIDTH, Config::IMAGE_HEIGHT,
            preset[0], preset[1]
        );
        
        must_update = true;
        break;
    } // don't remove, may cause errors due to variable declaration in switch
    
    default:
        break;
    }

    if (must_update) {
        _my_camera->BurstRays(_world);
        _tex.CopyFromBuffer(std::bit_cast<uint8_t const*>(_my_camera->m_img_buf.data()), 0, 0, 0,
            _tex.GetWidth(), _tex.GetHeight(),
            cg_raytracing::PixelFormat::RGB,
            cg_raytracing::PixelDataType::UNSIGNED_BYTE);
    }
}

int main() {
    using Camera = cg_raytracing::scene::Camera;
    std::unique_ptr<Camera> my_camera = std::make_unique<Camera>();

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
    tex.SetUpscaleFilter(cg_raytracing::SamplerFilter::NEAREST);
    tex.SetDownscaleFilter(cg_raytracing::SamplerFilter::LINEAR);
    // std::vector<uint8_t> temp_buf{};
    // temp_buf.resize(tex.GetSizeBytes());
    // for (auto &val : temp_buf) {
    //     val = (uint8_t)(rand() % 256);
    // }
    // tex.CopyFromBuffer(temp_buf.data(), 0, 0, 0, tex.GetWidth(),
    //                    tex.GetHeight(), cg_raytracing::PixelFormat::RGB,
    //                    cg_raytracing::PixelDataType::UNSIGNED_BYTE);

    /*
    cg_raytracing::scene::PointLight light(
        cg_raytracing::math::Vec3(0.0f, 0.0f, 150.0f),
        cg_raytracing::math::Vec3(1.0f, 1.0f, 1.0f),
        1.5f
    );*/

    using Hittable = cg_raytracing::geometry::Hittable;
    using World = cg_raytracing::scene::World;
    using StandardMaterial = cg_raytracing::geometry::StandardMaterial;
    using Sphere = cg_raytracing::geometry::Sphere;
    using Cube = cg_raytracing::geometry::Cube;
    using Mesh = cg_raytracing::geometry::Mesh;
    using Vec3 = cg_raytracing::math::Vec3;

    std::vector<std::shared_ptr<Hittable>> hittables;
    // Filippo qua va inserito il loader:
    // Uhmm, questo richide che il loader ritorni
    // una classe Mesh derivata da Hittable
    // che implementi i metodi richiesti
    // hittables = ObjLoader::Load("model.obj", &mat);

    auto mat_sphere = std::make_shared<StandardMaterial>(
    StandardMaterial::Diffuse({.4f, .4f, .8f}, {.5f, .5f, .5f})
    );
    auto mat_cube = std::make_shared<StandardMaterial>(
        StandardMaterial::Diffuse({.4f, .4f, .8f}, {.5f, .5f, .5f })
    );
    auto mat_train = std::make_shared<StandardMaterial>(
        StandardMaterial::Diffuse({0.7f, 0.2f, 0.2f})
    );

    auto world = World::CreateEmpty(1000.f);

    using EmissiveMaterial = cg_raytracing::geometry::EmissiveMaterial;

    // Emissive sphere acting as a light source in the scene
    auto mat_light1 = std::make_shared<EmissiveMaterial>(
        EmissiveMaterial::Create({1.0f, 1.0f, 0.0f}, 5.0f)
    );
    auto mat_light2 = std::make_shared<EmissiveMaterial>(
        EmissiveMaterial::Create({ 0.0f, 1.0f, 0.0f }, 10.0f)
    );

    // Transparent sphere acting as a glass ball in the scene

    using DielectricMaterial = cg_raytracing::geometry::DielectricMaterial;

    auto mat_glass = std::make_shared<DielectricMaterial>(
        DielectricMaterial::Create(1.2f) // glass
    );

    world.AddObject(std::make_shared<Sphere>(Vec3(50.0f, 0.0f, 180.0f), 25.f, mat_glass));

    // Red sphere in the middle of the scene
    world.AddObject(std::make_shared<Sphere>(
        Vec3(0.0f, 0.0f, 220.0f), 20.f, mat_sphere
    ));



    world.AddObject(std::make_shared<Sphere>(cg_raytracing::math::Vec3(0.0f, -45.0f, 180.0f), 15.f, mat_light1));
    world.AddObject(std::make_shared<Sphere>(cg_raytracing::math::Vec3(-40.f, 10.0f, 170.0f), 15.f, mat_light2));


    std::shared_ptr<Mesh> train = std::make_shared<Mesh>(Vec3(0.0f, 0.0f, 200.0f));
    // TODO: handle exception
    auto loader_status = train->LoadFromObj("./assets/meshes/Lampada.obj", 10.0);
    train->Rotate(Vec3(0.5f, 0.0f, 0.0f));

    world.AddObject(std::make_shared<Sphere>(Vec3(0.0f, 0.0f, 200.0f), 30.f, mat_sphere));
    world.AddObject(std::make_shared<Cube>(Vec3(0.0f, 85.0f, 200.0f), 50.f, mat_cube));
    // world.AddObject(std::make_shared<Sphere>(Vec3(0.0f, 0.0f, 250.0f), 20.f, mat_cube));
    // world.AddObject(train);

    world.UpdateTree();

    auto begin = std::chrono::system_clock::now();

    my_camera->BurstRays(world);

    auto end = std::chrono::system_clock::now();
    auto diff = std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count();
    std::println(std::cout, "Took {} ms", (double)diff / 1e3);


    tex.CopyFromBuffer(std::bit_cast<uint8_t const*>(my_camera->m_img_buf.data()), 0, 0, 0, tex.GetWidth(),
                       tex.GetHeight(), cg_raytracing::PixelFormat::RGB,
                       cg_raytracing::PixelDataType::UNSIGNED_BYTE);
    tex.BindTexture(GL_TEXTURE_2D);

    bool close = false;
    while (!close) {
        SDL_Event ev{};
        while (SDL_PollEvent(&ev)) {
            switch (ev.type) {
            case SDL_EVENT_KEY_DOWN:
                HandleKeyDown(ev, my_camera, tex, world, hittables);
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
