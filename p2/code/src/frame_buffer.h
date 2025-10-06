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

struct ColorOrTexture {
private:
    Vector3<f32> data{0, 0, 0};

public:
    ColorOrTexture() = default;
    explicit ColorOrTexture(const Vector3<f32> color) : data(color) {}
    explicit ColorOrTexture(TextureId texture) : data({NAN, *reinterpret_cast<f32*>(&texture), 0}) {
        u32 value = 0xFFFFFFFF;
        data.x() = *reinterpret_cast<f32*>(&value);
    }

    static ColorOrTexture from_or_default(ref<std::optional<std::shared_ptr<const Texture>>> texture, const Vector3<f32> color) {

        if (texture.has_value()) {
            return ColorOrTexture(texture->get()->get_id());
        }
        return ColorOrTexture(color);
    }

    [[nodiscard]]
    bool is_texture() const {
        return !is_color();
    }

    [[nodiscard]]
    bool is_color() const {
        return *reinterpret_cast<const u32*>(&data.x()) != 0xFFFFFFFF;
    }

    ref<Vector3<f32>> color() const {
        return data;
    }

    ref_mut<Vector3<f32>> color() {
        return data;
    }

    ref<TextureId> texture_id() const {
        return *reinterpret_cast<const TextureId *>(&data.y());
    }

    ref_mut<TextureId> texture_id() {
        return *reinterpret_cast<TextureId *>(&data.y());
    }
};

struct Pixel {
    ColorOrTexture ambient;
    ColorOrTexture diffuse;
    ColorOrTexture specular;

    f32 shininess{};

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
