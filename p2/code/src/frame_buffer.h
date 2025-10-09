//
// Created by may on 04/10/25.
//

#ifndef FRAME_BUFFER_H
#define FRAME_BUFFER_H

#include <memory>
#include <optional>

#include "vec_math.h"
#include "slice.h"
#include "texture.h"

struct Pixel {
    Vector3<f32> ambient;
    Vector3<f32> diffuse;
    Vector3<f32> specular;

    TextureId ambient_map;
    TextureId diffuse_map;
    TextureId specular_map;
    TextureId normal_map;

    i32 shininess{};

    Vector2<f32> uv;

    Vector3<f32> normal;
    Vector3<f32> tangent;
    Vector3<f32> bitangent;

    Vector3<f32> position;

    u32 depth{0xFFFFFFFE};

    INLINE void set_smaller_depth(Pixel pixel) {
        if (pixel.depth < this->depth) {
            *this = pixel;
        }
    }
};

struct FrameBuffer {
private:
    usize m_width;
    usize m_height;
    ptr_mut<Pixel> m_pixels;
public:
    FrameBuffer(usize const width, usize const height) : m_width(width), m_height(height) {
        this->m_pixels = new Pixel[width * height];
    }

    FrameBuffer(FrameBuffer&& other) noexcept : m_width(other.m_width), m_height(other.m_height), m_pixels(other.m_pixels) {
        other.m_pixels = nullptr;
    }

    [[nodiscard]]
    ref<Pixel> operator[] (Vector2<usize> xy) const {
        return this->m_pixels[xy[0] + xy[1]*this->m_width];
    }

    [[nodiscard]]
    ref_mut<Pixel> operator[] (Vector2<usize> xy) {
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

    [[nodiscard]]
    slice<const Pixel> pixels() const {
        return slice<const Pixel>::from_raw(this->m_pixels, this->m_width*this->m_height);
    }

    [[nodiscard]]
    slice<Pixel> pixels() {
        return slice<Pixel>::from_raw(this->m_pixels, this->m_width*this->m_height);
    }

    ~FrameBuffer() {
        delete[] this->m_pixels;
    }
};

#endif //FRAME_BUFFER_H
