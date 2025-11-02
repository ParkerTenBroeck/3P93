#ifndef GAME_H
#define GAME_H
#include "renderer/renderer.h"
#include "resources/resource_store.h"
#include "renderer/scene.h"


class Game;
class System {
    public:
    virtual ~System() = default;

    virtual void init(Game* /*game*/) {}
    virtual void update(Game* /*game*/, f64 /*delta*/, f64 /*time*/) {}
};

template<typename U>
class Lambda : public System{
    U m_update;
public:
    explicit Lambda(U&& update) : m_update(update) {}
    void update(Game* game, f64 delta, f64 time) override { return m_update(game, delta, time); }
};

class Game {
public:
    ResourceStore resource_store{};
    Scene scene{};
    FrameBuffer frame_buffer;
    std::vector<System*> systems;


    explicit Game(FrameBuffer&& frame_buffer) : frame_buffer(std::move(frame_buffer)) {
        add_rotating_lights(4.f);
        // add_bricks();
        // add_halo();
        // add_cube();
        add_global_light();
        airport();
        // add_minecraft_world();
        // add_light_following_player();
    }

    void airport() {
        auto airport = scene.add_object(Object::load("../assets/airport/Sunshine Airport.obj", resource_store));
        // scene[airport].m_scale.x() = 0.001f;
        // scene[airport].m_scale.y() = 0.001f;
        // scene[airport].m_scale.z() = 0.001f;
    }

    void add_light_following_player() {
        const auto l1_id = scene.m_lights.size();
        scene.m_lights.emplace_back(Light{});
        systems.push_back(new Lambda([l1_id](Game* game, auto, f64 time) {
            game->scene.m_lights[l1_id].color = {0.5, 0.5, 0.4};
            game->scene.m_lights[l1_id].intensity = 1;
            game->scene.m_lights[l1_id].radius = 0;
            game->scene.m_lights[l1_id].position_or_direction = game->scene.m_camera.position;
        }));
    }

    void add_minecraft_world() {

        auto halo = scene.add_object(Object::load("../assets/city/Untitled.obj", resource_store));
        scene[halo].m_scale.x() = 1.f;
        scene[halo].m_scale.y() = 1.f;
        scene[halo].m_scale.z() = 1.f;

    }

    void add_global_light() {
        const auto global_id = scene.m_lights.size();
        scene.m_lights.emplace_back(Light{});
        scene.m_lights[global_id].position_or_direction = {0,1,1};
        scene.m_lights[global_id].color = {1,1,1};
        scene.m_lights[global_id].global = true;
        scene.m_lights[global_id].intensity = 0.2;
    }

    void add_rotating_lights(f32 scale) {
        const auto l1_id = scene.m_lights.size();
        scene.m_lights.emplace_back(Light{});
        systems.push_back(new Lambda([l1_id, scale](auto game, auto, auto time) {
            game->scene.m_lights[l1_id].color = {1, 0.03, 0.03};
            game->scene.m_lights[l1_id].intensity = 20;
            game->scene.m_lights[l1_id].position_or_direction.x() = std::sin((f32)time/10.f * M_PIf * 2)*scale;
            game->scene.m_lights[l1_id].position_or_direction.z() = std::cos((f32)time/10.f * M_PIf * 2)*-scale;
        }));

        const auto l2_id = scene.m_lights.size();
        scene.m_lights.emplace_back(Light{});
        systems.push_back(new Lambda([l2_id, scale](auto game, auto, auto time) {
            game->scene.m_lights[l2_id].color = {0.03, 1, 0.03};
            game->scene.m_lights[l2_id].intensity = 20;
            game->scene.m_lights[l2_id].position_or_direction.y() = std::sin((f32)time/10.f * M_PIf * 2)*scale;
            game->scene.m_lights[l2_id].position_or_direction.z() = std::cos((f32)time/10.f * M_PIf * 2)*-scale;
        }));

        const auto l3_id = scene.m_lights.size();
        scene.m_lights.emplace_back(Light{});
        systems.push_back(new Lambda([l3_id, scale](auto game, auto, auto time) {
            game->scene.m_lights[l3_id].color = {0.03, 0.03, 1};
            game->scene.m_lights[l3_id].intensity = 20;
            game->scene.m_lights[l3_id].position_or_direction.y() = std::sin((f32)time/10.f * M_PIf * 2)*scale;
            game->scene.m_lights[l3_id].position_or_direction.x() = std::cos((f32)time/10.f * M_PIf * 2)*-scale;
        }));
    }

    void add_halo() {
        auto halo = scene.add_object(Object::load("../assets/halo/spartan_armour_mkv_-_halo_reach.obj", resource_store));
        scene[halo].m_position.z() = 0;
    }

    void add_bricks() {
        const auto brick = scene.add_object(Object::load("../assets/bricks/Mauerrest_C.obj", resource_store));
        scene[brick].m_position.y() -= 10;

        const auto city = scene.add_object(Object::load("../assets/city/full_gameready_city_buildings.obj", resource_store));
        scene[city].m_position.y() -= 10;
    }

    void add_cube() {
        auto cube = scene.add_object(Object::load("../assets/brick/brick.obj", resource_store));
        systems.push_back(new Lambda([cube](auto game, auto, auto time) {
            game->scene[cube].m_rotation.y() = (f32)time/10.f * M_PIf*2;
        }));
    }

    void update(f32 delta, f64 time) {
        for (auto& system : systems) {
            system->update(this, delta, time);
        }
    }

    void render() {
        Renderer::render(this->frame_buffer, this->scene, this->resource_store);
    }
};

#endif //GAME_H