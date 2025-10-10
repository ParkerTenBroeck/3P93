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

    bool halo;
    FrameBuffer frame_buffer;

    explicit Game(FrameBuffer&& frame_buffer, bool halo) : frame_buffer(std::move(frame_buffer)), halo(halo) {
        if (halo) {
            auto halo = scene.add_object(Object::load("../assets/halo/spartan_armour_mkv_-_halo_reach.obj", resource_store));
            scene[halo].m_position.z() = 0;

            scene.m_lights.emplace_back(Light{});
            scene.m_lights.emplace_back(Light{});
            scene.m_lights.emplace_back(Light{});
            scene.m_lights.emplace_back(Light{});
            scene.m_lights[3].position_or_direction = {1,1,1};
            scene.m_lights[3].color = {1,1,1};
            scene.m_lights[3].global = true;
            scene.m_lights[3].intensity = 0.5;
        }else {
            auto brick = scene.add_object(Object::load("../assets/brick/brick.obj", resource_store));
            scene[brick].m_position.z() = 0;

            scene.m_lights.emplace_back(Light{});
            scene.m_lights.emplace_back(Light{});
            scene.m_lights.emplace_back(Light{});
            scene.m_lights.emplace_back(Light{});
            scene.m_lights[3].position_or_direction = {-1,1,-1};
            scene.m_lights[3].color = {1,1,1};
            scene.m_lights[3].global = true;
            scene.m_lights[3].intensity = 0.1;

            scene.m_camera.target = {0, 0, 0};

            auto castle = scene.add_object(Object::load("../assets/castle/peach_castle.obj", resource_store));
            scene[castle].m_position.z() += 100;
            scene[castle].m_position.y() -= 20;
            scene[castle].m_rotation.y() = M_PI;
        }
    }

    void update(f32 delta, f64 time) {

        auto max_time = 300.f;
        auto percent = static_cast<f32>(time) / max_time;
        if (halo) {
            scene.m_camera.target = {0, (float)(-1.5-std::sin(percent * M_PI)*0.5), 0};
            scene.m_camera.position.y() = 2-std::sin(percent * M_PI)*3;
            scene.m_camera.position.z() = std::cos(percent * M_PI * 2)*7/(1.+std::sin(percent * M_PI)*0.3);
            scene.m_camera.position.x() = std::sin(percent * M_PI * 2)*7/(1.+std::sin(percent * M_PI)*0.3);
        }else {
            scene.m_camera.target = {0, 0, 0};
            scene.m_camera.position.y() = 2;
            scene.m_camera.position.z() = -7-(std::sin(time / max_time * M_PI * 2)*5+5);
        }
        scene.m_lights[0].color = {1, 0.0, 0.0};
        scene.m_lights[1].color = {0.0, 1, 0.0};
        scene.m_lights[2].color = {0.0, 0.0, 1};

        scene.m_lights[0].intensity = 20;
        scene.m_lights[1].intensity = 20;
        scene.m_lights[2].intensity = 20;

        const auto scale = halo?6.f:3.f;
        scene.m_lights[0].position_or_direction.x() = static_cast<f32>(std::sin(percent * M_PI * 2))*scale;
        scene.m_lights[0].position_or_direction.z() = static_cast<f32>(std::cos(percent * M_PI * 2))*-scale;

        scene.m_lights[1].position_or_direction.y() = static_cast<f32>(std::sin(percent*2 * M_PI * 2))*scale;
        scene.m_lights[1].position_or_direction.z() = static_cast<f32>(std::cos(percent*2 * M_PI * 2))*-scale;

        scene.m_lights[2].position_or_direction.y() = static_cast<f32>(std::sin(percent*3 * M_PI * 2))*scale;
        scene.m_lights[2].position_or_direction.x() = static_cast<f32>(std::cos(percent*3 * M_PI * 2))*-scale;

    }

    void render() {
        Renderer::render(this->frame_buffer, this->scene, this->resource_store);
    }
};

#endif //GAME_H
