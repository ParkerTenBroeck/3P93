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
    // FrameBuffer frame_buffer{720, 480};

    FrameBuffer frame_buffer{4096, 2160};

    Game() {
        auto brick = scene.add_object(Object::load("../assets/brick/brick.obj", resource_store));
        scene[brick].m_position.z() += 5;
        // scene[brick].m_rotation.y() = M_PI;

        // auto mario = scene.add_object(Object::load("../assets/head/mariohead.obj", resource_store));
        // scene[mario].m_position.z() += 50;
        // scene[mario].m_rotation.y() = M_PI;

        // auto castle = scene.add_object(Object::load("../assets/castle/peach_castle.obj", resource_store));
        // scene[castle].m_position.z() += 100;
        // scene[castle].m_position.y() -= 20;
        // scene[castle].m_rotation.y() = M_PI;
    }

    void update(f32 delta, f64 time) {

    }

    void render() {
        Renderer::render(this->frame_buffer, this->scene, this->resource_store);
    }
};

#endif //GAME_H
