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
    FrameBuffer frame_buffer{720, 480};

    // FrameBuffer frame_buffer{1920,1080};
    // FrameBuffer frame_buffer{4096, 2160};
    Game() {
        auto brick = scene.add_object(Object::load("../assets/brick/brick.obj", resource_store));
        // scene[brick].m_position.z() += 5;

        scene.m_lights.emplace_back(Light{});
        scene.m_lights.emplace_back(Light{});
        scene.m_lights.emplace_back(Light{});

        scene.m_camera.target = {0, 0, 0};
        // scene[brick].m_rotation.y() = M_PI;

        // auto mario = scene.add_object(Object::load("../assets/head/mariohead.obj", resource_store));
        // scene[mario].m_position.z() += 50;
        // scene[mario].m_rotation.y() = M_PI;

        auto castle = scene.add_object(Object::load("../assets/castle/peach_castle.obj", resource_store));
        scene[castle].m_position.z() += 100;
        scene[castle].m_position.y() -= 20;
        scene[castle].m_rotation.y() = M_PI;

    }

    void update(f32 delta, f64 time) {
        auto max_time = 300;
        scene.m_camera.target = {0, 0, 0};
        scene.m_camera.position.y() = 2;
        scene.m_camera.position.z() = -7-(std::sin(time / max_time * M_PI * 2)*5+5);

        scene.m_lights[0].color = {1, 0.1, 0.1};
        scene.m_lights[1].color = {0.1, 1, 0.1};
        scene.m_lights[2].color = {0.1, 0.1, 1};

        scene.m_lights[0].intensity = 40;
        scene.m_lights[1].intensity = 40;
        scene.m_lights[2].intensity = 40;

        scene.m_lights[0].position.x() = static_cast<f32>(std::sin(time / max_time * M_PI * 2))*3;
        scene.m_lights[0].position.z() = static_cast<f32>(std::cos(time / max_time * M_PI * 2))*-3;

        scene.m_lights[1].position.y() = static_cast<f32>(std::sin(time*2 / max_time * M_PI * 2))*3;
        scene.m_lights[1].position.z() = static_cast<f32>(std::cos(time*2 / max_time * M_PI * 2))*-3;

        scene.m_lights[2].position.y() = static_cast<f32>(std::sin(time*3 / max_time * M_PI * 2))*3;
        scene.m_lights[2].position.x() = static_cast<f32>(std::cos(time*3 / max_time * M_PI * 2))*-3;

    }

    void render() {
        Renderer::render(this->frame_buffer, this->scene, this->resource_store);
    }
};

#endif //GAME_H
