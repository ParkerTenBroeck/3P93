//
// Created by may on 05/10/25.
//

#ifndef SCENE_H
#define SCENE_H

#include "frame_buffer.h"
#include "obj.h"
#include "vec_math.h"

class Camera {
public:
    Vector3<f32> target{0, 0, 1};
    Vector3<f32> position{0, 0, 0};
    f32 fov{M_PI/3};
    f32 zoom{1};

    [[nodiscard]]
    Matrix4<f32> view() const {
        return Matrix4<f32>::look_at(this->position, this->position+this->target, {0, 1, 0});
    }
};

class ObjectId {
    friend class Scene;
    usize idx;
    explicit ObjectId(const usize idx) : idx(idx) {}
};

class Scene {
    std::vector<Object> m_objects{};
public:
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
    Matrix4<f32> proj_view(ref<FrameBuffer> frame) const {
        auto aspect = static_cast<f32>(frame.width())/static_cast<f32>(frame.height());
        return Matrix4<f32>::perspective(0.1, 500, aspect, m_camera.fov)*m_camera.view();
    };
};

#endif //SCENE_H
