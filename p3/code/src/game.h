#ifndef GAME_H
#define GAME_H

#include "renderer/renderer.h"
#include "resources/resource_store.h"
#include "renderer/scene.h"
#include <args.h>

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


    static std::unique_ptr<Game> make_game(const Arguments& args) {
        auto game = std::make_unique<Game>(FrameBuffer{args.width, args.height});;
        switch (args.scene) {
            case Scenes::Halo:
                if (!args.freecam) game->rotating_camera();
                game->add_rotating_lights(4.f);
                game->add_halo();
                break;
            case Scenes::Brick:
                game->add_cube();
                game->add_rotating_lights(4.f);
                break;
            case Scenes::Bricks:
                if (!args.freecam) game->panning_camera();
                game->add_bricks();
                game->add_rotating_lights(2.f, {0, -2, 0});
                break;
            case Scenes::Test:
                game->add_rotating_lights(4.f);
                game->add_cube();
                game->fun_mesh();
                game->scene.m_camera.position = {0, 10, -5};
                game->scene.m_camera.target = {0, 0, 0};
                break;
            default:
                std::cout << "Invalid scene argument passed" << std::endl;
                exit(-1);
        }

        return game;
    }


    explicit Game(FrameBuffer&& frame_buffer) : frame_buffer(std::move(frame_buffer)) {}

    void panning_camera() {
        systems.push_back(new Lambda([](Game* game, auto, auto time) {
            game->scene.m_camera.position.z() = std::sin(std::sin((f32)time/10.f*M_PIf*2) * M_PIf/4+M_PIf)*40;
            game->scene.m_camera.position.x() = std::cos(std::sin((f32)time/10.f*M_PIf*2) * M_PIf/4+M_PIf)*40;
        }));
    }

    void rotating_camera() {
        systems.push_back(new Lambda([](Game* game, auto, auto time) {
            game->scene.m_camera.fov = M_PI/10;
            game->scene.m_camera.target = Vector3<f32>{0, -1.0f, 0};

            game->scene.m_camera.position.z() = std::cos((f32)time /10.f * M_PIf * 2)*7;
            game->scene.m_camera.position.x() = std::sin((f32)time /10.f * M_PIf * 2)*7;
        }));
    }

    void fun_mesh() {
        auto obj = Object{Mesh{}};
        obj.m_scale.x() = 10.f;
        obj.m_scale.y() = 0.2f;
        obj.m_scale.z() = 10.f;

        obj.m_position.x() = -5.f;
        obj.m_position.y() = -1.f;
        obj.m_position.z() = -5.f;
        auto& mesh = std::get<Mesh>(obj.m_kind);
        mesh.m_material.diffuse = {1.f, 1.f, 1.f};
        mesh.m_material.ambient = {0.02f, 0.f, 0.f};
        mesh.m_material.diffuse_map = resource_store.rgba_gamma_corrected("../assets/brick/wood.png");
        mesh.m_material.normal_map = resource_store.normal_map("../assets/brick/normal_test.png");
        mesh.m_material.shininess = 256;
        mesh.m_material.backface_cull = false;
        float width = 60.;
        float height = 60.;

        for (float x = 0; x < width; x++) {
            for (float y = 0; y < height; y++) {
                mesh.m_faces.emplace_back(Face{
                    {Vector3<f32>{x/width, 0.f, y/height}, {x/width, 0.f, (y+1)/height}, {(x+1)/width, 0.f, y/height}},
                    {Vector3<f32>{0.f, 1.f, 0.f}, {0.f, 1.f, 0.f}, {0.f, 1.f, 0.f}},
                    {Vector2<f32>{1.f-x/width, y/height}, {1.f-x/width, (y+1)/height}, {1.f-(x+1)/width, y/height}}
                });
                mesh.m_faces.emplace_back(Face{
                    {Vector3<f32>{(x+1)/width, 0.f, (y+1)/height}, {(x+1)/width, 0.f, y/height}, {x/width, 0.f, (y+1)/height}},
                    {Vector3<f32>{0.f, 1.f, 0.f}, {0.f, 1.f, 0.f}, {0.f, 1.f, 0.f}},
                    {Vector2<f32>{1.f-(x+1)/width, (y+1)/height}, {1.f-(x+1)/width, y/height}, {1.f-x/width, (y+1)/height}}
                });
            }
        }

        auto obj_id = scene.add_object(std::move(obj));

        systems.push_back(new Lambda([obj_id](auto game, auto, auto time) {
            Mesh& mesh = std::get<Mesh>(game->scene[obj_id].m_kind);
            #ifdef USE_OPEN_MP
            #pragma omp parallel for schedule(static)
            #endif
            for (usize f = 0; f < mesh.m_faces.size(); f ++) {
                auto& face = mesh.m_faces[f];
                for (int i = 0; i < 3; i++) {
                    auto inner = face.points[i].x()*15 + face.points[i].z()*20 + (f32)time;
                    face.points[i].y() = std::sin(inner);
                    face.normals[i] = Vector3<f32>{
                        -15.f*std::cos(inner), 1.f, -20.f*std::cos(inner)
                    }.normalize();
                }
            }
        }));
    }

    void airport() {
        // auto airport =
            scene.add_object(Object::load("../assets/airport/Sunshine Airport.obj", resource_store));
        // scene[airport].m_scale.x() = 0.001f;
        // scene[airport].m_scale.y() = 0.001f;
        // scene[airport].m_scale.z() = 0.001f;
    }

    void add_light_following_player() {
        const auto l1_id = scene.m_lights.size();
        scene.m_lights.emplace_back(Light{});
        systems.push_back(new Lambda([l1_id](Game* game, auto, f64) {
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

    void add_rotating_lights(f32 scale, Vector3<f32> offset = {0,0,0}) {
        const auto l1_id = scene.m_lights.size();
        scene.m_lights.emplace_back(Light{});
        systems.push_back(new Lambda([l1_id, scale, offset](auto game, auto, auto time) {
            game->scene.m_lights[l1_id].color = {1, 0.03, 0.03};
            game->scene.m_lights[l1_id].intensity = 20;
            game->scene.m_lights[l1_id].position_or_direction =
                Vector3<f32>{std::sin((f32)time/10.f * M_PIf * 2), 0.0f, std::cos((f32)time/10.f * M_PIf * 2)}*scale+offset;
        }));

        const auto l2_id = scene.m_lights.size();
        scene.m_lights.emplace_back(Light{});
        systems.push_back(new Lambda([l2_id, scale, offset](auto game, auto, auto time) {
            game->scene.m_lights[l2_id].color = {0.03, 1, 0.03};
            game->scene.m_lights[l2_id].intensity = 20;
            game->scene.m_lights[l2_id].position_or_direction =
                Vector3<f32>{0.0f, std::sin((f32)time/10.f * M_PIf * 2), std::cos((f32)time/10.f * M_PIf * 2)}*scale+offset;
        }));

        const auto l3_id = scene.m_lights.size();
        scene.m_lights.emplace_back(Light{});
        systems.push_back(new Lambda([l3_id, scale, offset](auto game, auto, auto time) {
            game->scene.m_lights[l3_id].color = {0.03, 0.03, 1};
            game->scene.m_lights[l3_id].intensity = 20;
            game->scene.m_lights[l3_id].position_or_direction =
                Vector3<f32>{std::cos((f32)time/10.f * M_PIf * 2), std::sin((f32)time/10.f * M_PIf * 2), 0.0f}*scale+offset;
        }));
    }

    void add_halo() {
        auto halo = scene.add_object(Object::load("../assets/halo/spartan_armour_mkv_-_halo_reach.obj", resource_store));
        scene[halo].m_position.y() = -4;
    }

    void add_bricks() {
        const auto brick = scene.add_object(Object::load("../assets/bricks/Mauerrest_C.obj", resource_store));
        scene[brick].m_position.y() -= 10;
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