//
// Created by may on 05/10/25.
//

#ifndef TEXTURE_H
#define TEXTURE_H

#include "types.h"
#include "vec_math.h"
#include "stb_image.h"

template<typename T>
INLINE inline T euclidean_remainder(T a, T b) {
    int r = a % b;
    return r >= 0 ? r : r + std::abs(b);
}

/**
 * A unique ID that references already loaded textures
 */
class TextureId {
    friend class ResourceStore;
    u32 id;
    explicit TextureId(u32 id) : id(id) {}
public:
    TextureId() : TextureId(0) {}

    [[nodiscard]]
    bool exists() const{
        return id != 0;
    }
};

/**
 * A texture with 4 channels each in the range [0.0f, 1.0f]
 */
class Texture {
    usize m_width;
    usize m_height;
    bool m_transparent;
    ptr_mut<Vector4<f32>> m_pixels;
    TextureId m_id;

    friend class ResourceStore;

    explicit Texture(usize width, usize height, bool transparent, ptr_mut<Vector4<f32>> pixels) : m_width(width), m_height(height), m_transparent(transparent), m_pixels(pixels), m_id() {}
public:
    Texture(Texture&& texture) noexcept {
        m_id = texture.m_id;
        m_width = texture.m_width;
        m_height = texture.m_height;
        m_transparent = texture.m_transparent;
        m_pixels = texture.m_pixels;
        texture.m_pixels = nullptr;
    }

    ~Texture() {
        delete[] this->m_pixels;
    }

    [[nodiscard]]
    usize width() const {
        return this->m_width;
    }

    [[nodiscard]]
    usize height() const {
        return this->m_height;
    }

    [[nodiscard]]
    bool transparent() const {
        return this->m_transparent;
    }

    [[nodiscard]]
    TextureId get_id() const {
        return this->m_id;
    }

    [[nodiscard]]
    ref<Vector4<f32>> operator[](const usize i) const {
        return this->m_pixels[i];
    }

    [[nodiscard]]
    ref_mut<Vector4<f32>> operator[](const usize i) {
        return this->m_pixels[i];
    }

    [[nodiscard]]
    ref<Vector4<f32>> operator[](ref<Vector2<usize>> coord) const {
        return this->m_pixels[coord.x() + coord.y()*this->width()];
    }

    [[nodiscard]]
    ref_mut<Vector4<f32>> operator[](ref<Vector2<usize>> coord) {
        return this->m_pixels[coord.x() + coord.y()*this->width()];
    }

    [[nodiscard]]
    INLINE ref<Vector4<f32>> resolve_uv_wrapping(ref<Vector2<f32>> uv) const {
        const auto x = euclidean_remainder(
            static_cast<isize>(uv.x() * static_cast<f32>(this->width())),
            static_cast<isize>(this->width())
            );
        const auto y = euclidean_remainder(
            static_cast<isize>(this->height())-static_cast<isize>(uv.y() * static_cast<f32>(this->height())),
            static_cast<isize>(this->height())
            );
        return this->m_pixels[x+y*this->width()];
    }

};

#endif //TEXTURE_H
