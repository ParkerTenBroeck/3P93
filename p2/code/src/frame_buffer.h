//
// Created by may on 04/10/25.
//

#ifndef FRAME_BUFFER_H
#define FRAME_BUFFER_H

#include "vec_math.h"

struct Pixel {
    Vector3<f32> ambient;
    Vector3<f32> diffuse;
    Vector3<f32> specular;

    f32 shininess{};

    Vector2<f32> uv;

    Vector3<f32> normal;
    Vector3<f32> tangent;
    Vector3<f32> bitangent;

    Vector3<f32> position;

    u32 depth{0xFFFFFFFEull};
};

struct FrameBuffer {
private:
    usize m_width;
    usize m_height;
    Pixel* m_pixels;
public:
    FrameBuffer(usize const width, usize const height) : m_width(width), m_height(height) {
        this->m_pixels = new Pixel[width * height];
    }

    [[nodiscard]]
    ref<Pixel> operator[] (std::array<usize, 2> xy) const {
        return this->m_pixels[xy[0] + xy[1]*this->m_width];
    }

    [[nodiscard]]
    ref_mut<Pixel> operator[] (std::array<usize, 2> xy) {
        return this->m_pixels[xy[0] + xy[1]*this->m_width];
    }

    [[nodiscard]]
    ref<Pixel> operator[] (usize index) const {
        return this->m_pixels[index];
    }

    [[nodiscard]]
    ref_mut<Pixel> operator[] (usize index) {
        return this->m_pixels[index];
    }

    [[nodiscard]]
    usize height() const {
        return this->m_height;
    }

    [[nodiscard]]
    usize width() const {
        return this->m_width;
    }

    ~FrameBuffer() {
        delete[] this->m_pixels;
    }
};

#endif //FRAME_BUFFER_H
