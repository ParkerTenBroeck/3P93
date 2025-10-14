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

public:
    explicit Texture(ref<std::string> path, TextureId id) : m_width(0), m_height(0), m_transparent(false), m_id(id) {


        i32 width, height, channels;
        auto result = stbi_loadf(path.c_str(), &width, &height, &channels, 4);


        if (!result)
            std::cout << "Failed to load texture: " << path << std::endl;
        if (width == 0)
            std::cout << "Texture width cannot be zero: " << path << std::endl;
        if (height == 0)
            std::cout << "Texture height cannot be zero: " << path << std::endl;

        if (!result || width == 0 || height == 0) {
            this->m_width = 2;
            this->m_height = 2;
            this->m_pixels = new Vector4<f32>[this->m_width * this->m_height];
            this->m_pixels[0] = {0, 0, 0, 1};
            this->m_pixels[1] = {.5, 0, .5, 1};
            this->m_pixels[2] = {0, 0, 0, 1};
            this->m_pixels[3] = {.5, 0, .5, 1};
        }else{
            this->m_width = static_cast<usize>(width);
            this->m_height = static_cast<usize>(height);
            this->m_pixels = new Vector4<f32>[this->m_width * this->m_height];

            for (usize i = 0; i < this->m_width * this->m_height; i++) {
                this->m_pixels[i] = {result[i*4], result[i*4+1], result[i*4+2], result[i*4+3]};
                this->m_transparent |= result[i*4+3] != 1.;
            }
        }

        stbi_image_free(result);

        std::cout << "Loaded texture: " << path << std::endl;
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
            static_cast<isize>(uv.y() * static_cast<f32>(this->height())),
            static_cast<isize>(this->height())
            );
        return this->m_pixels[x+y*this->width()];
    }

};

#endif //TEXTURE_H
