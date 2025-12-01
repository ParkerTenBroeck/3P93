#ifndef TUI_H
#define TUI_H

#include <args.h>
#include <iomanip>

#include <stb_image_write.h>
inline void write_image(ref<Game> game, std::string&& path) {
    auto channels = 4;
    auto data = new u8[game.frame_buffer.height()*game.frame_buffer.width()*channels];

#ifdef USE_OPEN_MP
#pragma omp parallel for
#endif
    for (usize i = 0; i < game.frame_buffer.height() * game.frame_buffer.width(); i++) {
        auto color = game.frame_buffer[i].diffuse;
        auto normal = game.frame_buffer[i].normal;

        if (normal.magnitude_squared() != 0) {
            data[i*channels] = static_cast<u8>(std::min(255.f, std::pow(color.x(), 1.f/2.2f) * 255));
            data[i*channels+1] = static_cast<u8>(std::min(255.f, std::pow(color.y(), 1.f/2.2f) * 255));
            data[i*channels+2] = static_cast<u8>(std::min(255.f, std::pow(color.z(), 1.f/2.2f) * 255));
            data[i*channels+3] = 255;
        }else {
            data[i*channels+3] = 0;
        }
    }

    auto width = game.frame_buffer.width();
    auto height = game.frame_buffer.height();

    stbi_write_png(path.c_str(), width, height, channels, data, width * channels);
    delete[] data;
}

template<typename T>
inline std::string leading(T value, int total_length) {
    std::stringstream ss;
    ss << std::setw(total_length) << std::setfill('0') << value;
    return ss.str();
}

void run_tui(const Arguments& args);

#endif //TUI_H
