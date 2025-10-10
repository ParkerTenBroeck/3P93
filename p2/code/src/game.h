//
// Created by may on 05/10/25.
//

#ifndef GAME_H
#define GAME_H
#include "renderer.h"
#include "resource_store.h"
#include "scene.h"


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

        scene.m_lights[0].color = {1, 0.0, 0.0};
        scene.m_lights[1].color = {0.0, 1, 0.0};
        scene.m_lights[2].color = {0.0, 0.0, 1};

        scene.m_lights[0].intensity = 20;
        scene.m_lights[1].intensity = 20;
        scene.m_lights[2].intensity = 20;

        scene.m_lights[3].position_or_direction = {1,1,1};
        scene.m_lights[3].color = {1,1,1};
        scene.m_lights[3].global = true;
        scene.m_lights[3].intensity = 0.5;
    }

    void update(f32 delta, f64 time) override {

        auto max_time = 300.f;
        auto percent = static_cast<f32>(time) / max_time;

        scene.m_camera.target = {0, static_cast<float>(-1.5 - std::sin(percent * M_PIf) * 0.5), 0};
        scene.m_camera.position.y() = 2-std::sin(percent * M_PIf)*3;
        scene.m_camera.position.z() = std::cos(percent * M_PIf * 2)*7/(1.f+std::sin(percent * M_PIf)*0.3f);
        scene.m_camera.position.x() = std::sin(percent * M_PIf * 2)*7/(1.f+std::sin(percent * M_PIf)*0.3f);

        const auto scale = 6.f;
        scene.m_lights[0].position_or_direction.x() = std::sin(percent * M_PIf * 2)*scale;
        scene.m_lights[0].position_or_direction.z() = std::cos(percent * M_PIf * 2)*-scale;

        scene.m_lights[1].position_or_direction.y() = std::sin(percent*2 * M_PIf * 2)*scale;
        scene.m_lights[1].position_or_direction.z() = std::cos(percent*2 * M_PIf * 2)*-scale;

        scene.m_lights[2].position_or_direction.y() = std::sin(percent*3 * M_PIf * 2)*scale;
        scene.m_lights[2].position_or_direction.x() = std::cos(percent*3 * M_PIf * 2)*-scale;
    }
};


class BrickGame final : public Game {
public:
    explicit BrickGame(FrameBuffer&& frame_buffer) : Game(std::move(frame_buffer)) {
        const auto brick = scene.add_object(Object::load("../assets/brick/brick.obj", resource_store));
        scene[brick].m_position.z() = 0;

        const auto castle = scene.add_object(Object::load("../assets/castle/peach_castle.obj", resource_store));
        scene[castle].m_position.z() += 100;
        scene[castle].m_position.y() -= 20;
        scene[castle].m_rotation.y() = M_PI;

        scene.m_lights.emplace_back(Light{});
        scene.m_lights.emplace_back(Light{});
        scene.m_lights.emplace_back(Light{});
        scene.m_lights.emplace_back(Light{});

        scene.m_lights[0].color = {1, 0.0, 0.0};
        scene.m_lights[1].color = {0.0, 1, 0.0};
        scene.m_lights[2].color = {0.0, 0.0, 1};

        scene.m_lights[0].intensity = 20;
        scene.m_lights[1].intensity = 20;
        scene.m_lights[2].intensity = 20;

        scene.m_lights[3].position_or_direction = {-1,1,-1};
        scene.m_lights[3].color = {1,1,1};
        scene.m_lights[3].global = true;
        scene.m_lights[3].intensity = 0.1;

        scene.m_camera.target = {0, 0, 0};
    }

    void update(f32 delta, f64 time) override {
        const auto max_time = 300.f;
        const auto percent = static_cast<f32>(time) / max_time;

        scene.m_camera.target = {0, 0, 0};
        scene.m_camera.position.y() = 2;
        scene.m_camera.position.z() = -7-(std::sin(percent * M_PIf * 2)*5+5);

        const auto scale = 3.f;
        scene.m_lights[0].position_or_direction.x() = std::sin(percent * M_PIf * 2)*scale;
        scene.m_lights[0].position_or_direction.z() = std::cos(percent * M_PIf * 2)*-scale;

        scene.m_lights[1].position_or_direction.y() = std::sin(percent*2 * M_PIf * 2)*scale;
        scene.m_lights[1].position_or_direction.z() = std::cos(percent*2 * M_PIf * 2)*-scale;

        scene.m_lights[2].position_or_direction.y() = std::sin(percent*3 * M_PIf * 2)*scale;
        scene.m_lights[2].position_or_direction.x() = std::cos(percent*3 * M_PIf * 2)*-scale;
    }
};

#endif //GAME_H
