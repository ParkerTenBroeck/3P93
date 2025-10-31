#ifndef FRAME_BUFFER_H
#define FRAME_BUFFER_H

#include <util/vec_math.h>
#include <util/slice.h>
#include <renderer/pixel.h>

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
    usize size() const {
        return m_width*m_height;
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