//
// Created by may on 05/10/25.
//

#ifndef GAME_H
#define GAME_H
#include "renderer.h"
#include "resource_store.h"
#include "scene.h"

#ifndef M_PIf
#define M_PIf static_cast<f32>(M_PI)
#endif

class Game {
public:
    ResourceStore resource_store{};
    Scene scene{};
    FrameBuffer frame_buffer;


    explicit Game(FrameBuffer&& frame_buffer) : frame_buffer(std::move(frame_buffer)) {}

    virtual void update(f32 delta, f64 time){}

    void render() {
        Renderer::render(this->frame_buffer, this->scene, this->resource_store);
    }

    virtual ~Game() = default;
};

class HaloGame final : public Game {
public:
    explicit HaloGame(FrameBuffer&& frame_buffer) : Game(std::move(frame_buffer)) {
        auto halo = scene.add_object(Object::load("../assets/halo/spartan_armour_mkv_-_halo_reach.obj", resource_store));
        scene[halo].m_position.z() = 0;

        scene.m_lights.emplace_back(Light{});
        scene.m_lights.emplace_back(Light{});
        scene.m_lights.emplace_back(Light{});
        scene.m_lights.emplace_back(Light{});

        scene.m_lights[0].color = {1, 0.1, 0.1};
        scene.m_lights[1].color = {0.1, 1, 0.1};
        scene.m_lights[2].color = {0.1, 0.1, 1};

        scene.m_lights[0].intensity = 20;
        scene.m_lights[1].intensity = 20;
        scene.m_lights[2].intensity = 20;

        scene.m_lights[3].position_or_direction = {1,1,1};
        scene.m_lights[3].color = {1,1,1};
        scene.m_lights[3].global = true;
        scene.m_lights[3].intensity = 0.5;
    }

    void update(f32 delta, f64 time) override {

        auto meow_time = (f32)time / 10.f;

        scene.m_camera.target = Vector3<f32>{0, static_cast<float>(-1.5 - std::sin(meow_time * M_PIf) * 0.5), 0};
        // scene.m_camera.position.y() = 2-std::sin(percent * M_PIf)*3;
        scene.m_camera.position.z() = std::cos(meow_time * M_PIf * 2)*7;///(1.f+std::sin(percent * M_PIf)*0.3f);
        scene.m_camera.position.x() = std::sin(meow_time * M_PIf * 2)*7;///(1.f+std::sin(percent * M_PIf)*0.3f);

        const auto scale = 4.f;
        scene.m_lights[0].position_or_direction.x() = std::sin(meow_time * M_PIf * 2)*scale;
        scene.m_lights[0].position_or_direction.z() = std::cos(meow_time * M_PIf * 2)*-scale;

        scene.m_lights[1].position_or_direction.y() = std::sin(meow_time*2 * M_PIf * 2)*scale;
        scene.m_lights[1].position_or_direction.z() = std::cos(meow_time*2 * M_PIf * 2)*-scale;

        scene.m_lights[2].position_or_direction.y() = std::sin(meow_time*3 * M_PIf * 2)*scale;
        scene.m_lights[2].position_or_direction.x() = std::cos(meow_time*3 * M_PIf * 2)*-scale;
    }
};

class BrickGame final : public Game {
public:
    explicit BrickGame(FrameBuffer&& frame_buffer) : Game(std::move(frame_buffer)) {
        const auto brick = scene.add_object(Object::load("../assets/bricks/Mauerrest_C.obj", resource_store));
        scene[brick].m_position.y() -= 10;

        const auto city = scene.add_object(Object::load("../assets/city/full_gameready_city_buildings.obj", resource_store));
        scene[city].m_position.y() -= 10;
        // scene[brick].m_scale.x() = 0.2;
        // scene[brick].m_scale.y() = 0.2;
        // scene[brick].m_scale.z() = 0.2;

        scene.m_lights.emplace_back(Light{});
        scene.m_lights.emplace_back(Light{});
        scene.m_lights.emplace_back(Light{});
        scene.m_lights.emplace_back(Light{});

        scene.m_lights[0].color = {1, 0.03, 0.03};
        scene.m_lights[1].color = {0.03, 1, 0.03};
        scene.m_lights[2].color = {0.03, 0.03, 1};

        scene.m_lights[0].intensity = 5000;
        scene.m_lights[1].intensity = 5000;
        scene.m_lights[2].intensity = 5000;

        scene.m_lights[3].position_or_direction = {-1,1,-1};
        scene.m_lights[3].color = {1,1,1};
        scene.m_lights[3].global = true;
        scene.m_lights[3].intensity = 0.1;

        scene.m_camera.target = {0, 0, 0};
    }

    void update(f32 delta, f64 time) override {
        auto meow_time = (f32)time / 10.f;

        // scene.m_camera.target = {0, 0, 0};
        // scene.m_camera.position.z() = std::sin(std::sin(percent*M_PIf*2) * M_PIf/4+M_PIf)*40;
        // scene.m_camera.position.x() = std::cos(std::sin(percent*M_PIf*2) * M_PIf/4+M_PIf)*40;

        const auto scale = 3.f;
        scene.m_lights[0].position_or_direction.y() = -2;
        scene.m_lights[0].position_or_direction.x() = std::sin(meow_time * M_PIf * 2)*scale/1.7f;
        scene.m_lights[0].position_or_direction.z() = std::cos(meow_time * M_PIf * 2)*-scale/1.7f;

        scene.m_lights[1].position_or_direction.y() = -2+std::sin(meow_time*2 * M_PIf * 2)*scale/1.7f;
        scene.m_lights[1].position_or_direction.z() = std::cos(meow_time*2 * M_PIf * 2)*-scale/1.7f;

        scene.m_lights[2].position_or_direction.y() = -2+std::sin(meow_time*3 * M_PIf * 2)*scale;
        scene.m_lights[2].position_or_direction.x() = std::cos(meow_time*3 * M_PIf * 2)*-scale;
    }
};

class TestGame final : public Game {
    ObjectId brick{};
public:
    explicit TestGame(FrameBuffer&& frame_buffer) : Game(std::move(frame_buffer)) {
        this->brick = scene.add_object(Object::load("../assets/brick/brick.obj", resource_store));
        // scene[brick].m_position.x() = 0.5;
        // scene[brick].m_position.y() = 0.5;
        // scene[brick].m_position.z() = 0.5;
        // scene[brick].m_scale.x() = 0.5;
        // scene[brick].m_scale.y() = 0.5;
        // scene[brick].m_scale.z() = 0.5;

        scene.m_lights.emplace_back(Light{});
        scene.m_lights.emplace_back(Light{});
        scene.m_lights.emplace_back(Light{});
        scene.m_lights.emplace_back(Light{});

        scene.m_lights[0].color = {1, 0.03, 0.03};
        scene.m_lights[1].color = {0.03, 1, 0.03};
        scene.m_lights[2].color = {0.03, 0.03, 1};

        scene.m_lights[0].intensity = 10;
        scene.m_lights[1].intensity = 10;
        scene.m_lights[2].intensity = 10;

        scene.m_lights[3].position_or_direction = {-1,1,-1};
        scene.m_lights[3].color = {1,1,1};
        scene.m_lights[3].global = true;
        scene.m_lights[3].intensity = 0.1;

        scene.m_camera.position = {0, 0, -5};
        scene.m_camera.target = {0, 0, -4};
    }

    void update(f32 delta, f64 time) override {

        auto meow_time = (f32)time / 10.f;
        //std::sin(percent * M_PIf*2)*5;
        // scene.m_camera.position.x() = std::cos(percent * M_PIf*2)*5;

        this->scene[this->brick].m_rotation.y() = meow_time * M_PIf*2;

        const auto scale = 3.f;
        scene.m_lights[0].position_or_direction.x() = std::sin(meow_time * M_PIf * 2)*scale;
        scene.m_lights[0].position_or_direction.z() = std::cos(meow_time * M_PIf * 2)*-scale;

        scene.m_lights[1].position_or_direction.y() = std::sin(meow_time*2 * M_PIf * 2)*scale;
        scene.m_lights[1].position_or_direction.z() = std::cos(meow_time*2 * M_PIf * 2)*-scale;

        scene.m_lights[2].position_or_direction.y() = std::sin(meow_time*3 * M_PIf * 2)*scale;
        scene.m_lights[2].position_or_direction.x() = std::cos(meow_time*3 * M_PIf * 2)*-scale;
    }
};

#endif //GAME_H
