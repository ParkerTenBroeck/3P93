//
// Created by may on 05/10/25.
//

#ifndef SCENE_H
#define SCENE_H

#include "obj.h"
#include "vec_math.h"

class Camera {
public:
    Vector3<f32> up{0, 1, 0};
    Vector3<f32> target{0, 0, 0};
    Vector3<f32> position{0, 0, 0};
    f32 fov{M_PI/3};
    f32 zoom{1.f};

    [[nodiscard]]
    Matrix4<f32> view() const {
        return Matrix4<f32>::look_at(this->position, this->target, this->up);
    }
};

class ObjectId {
    friend class Scene;
    usize idx;
    explicit ObjectId(const usize idx) : idx(idx) {}
public:
    explicit ObjectId() : idx(0) {}
};

class Light {
public:
    Vector3<f32> position_or_direction{0,0,0};
    Vector3<f32> color{1, 1, 1};
    f32 intensity{1.f};
    bool global{false};
    f32 radius{0.2f};
};

/**
 * All the objects and lights in the scene as well as the camera
 */
class Scene {
    std::vector<Object> m_objects{};
public:
    std::vector<Light> m_lights{};
    Camera m_camera{};

    ObjectId add_object(Object&& obj) {
        m_objects.push_back(obj);
        return ObjectId(this->m_objects.size()-1);
    }

    [[nodiscard]]
    ref<std::vector<Object>> objects() const {
        return this->m_objects;
    }

    [[nodiscard]]
    ref<Object> operator[](const ObjectId id) const {
        return this->m_objects[id.idx];
    }

    [[nodiscard]]
    ref_mut<Object> operator[](const ObjectId id) {
        return this->m_objects[id.idx];
    }

    [[nodiscard]]
    Matrix4<f32> proj_view(Vector2<f32> view_port) const {
        auto aspect = view_port.x()/view_port.y();
        return Matrix4<f32>::projection(0.1, 500, aspect, m_camera.fov)*m_camera.view();
    };
};

#endif //SCENE_H
