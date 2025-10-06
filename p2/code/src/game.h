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

    Game() {
        auto id = scene.add_object(Object::load("../assets/head/mariohead.obj", resource_store));
        scene[id].m_position[2] += 5;
    }

    void update(f32 delta, f64 time) {

    }

    void render() {
        Renderer::render(this->frame_buffer, this->scene);
    }
};

#endif //GAME_H
