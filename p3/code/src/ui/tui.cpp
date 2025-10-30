#ifndef GUI

#include "../game.h"
#include "../args.h"

#include <chrono>
#include <iomanip>

#include "stb_image_write.h"

inline void write_image(ref<Game> game, std::string&& path) {
    auto channels = 4;
    auto data = new u8[game.frame_buffer.height()*game.frame_buffer.width()*channels];

    #ifdef PAR
    #pragma omp parallel for
    #endif
    for (usize i = 0; i < game.frame_buffer.height() * game.frame_buffer.width(); i++) {
        auto color = game.frame_buffer[i].diffuse;
        auto normal = game.frame_buffer[i].normal;
        auto normal_color = (game.frame_buffer[i].tangent*0.5).add_scalar(0.5f);

        // u8 r = game.frame_buffer[i].normal.dot(game.frame_buffer[i].tangent) * 255;
        // u8 g = game.frame_buffer[i].normal.dot(game.frame_buffer[i].bitangent)* 255;
        // u8 b = game.frame_buffer[i].bitangent.dot(game.frame_buffer[i].tangent)* 255;
        // data[i*channels] = r;
        // data[i*channels+1] = g;
        // data[i*channels+2] = b;

        data[i*channels] = static_cast<u8>(std::min(255.f, std::pow(color.x(), 1.f/2.2f) * 255));
        data[i*channels+1] = static_cast<u8>(std::min(255.f, std::pow(color.y(), 1.f/2.2f) * 255));
        data[i*channels+2] = static_cast<u8>(std::min(255.f, std::pow(color.z(), 1.f/2.2f) * 255));



        // data[i*channels] = static_cast<u8>(std::min(255.f, normal_color.x() * 255));
        // data[i*channels+1] = static_cast<u8>(std::min(255.f, normal_color.y() * 255));
        // data[i*channels+2] = static_cast<u8>(std::min(255.f, normal_color.z() * 255));
        data[i*channels+3] = normal.magnitude_squared() != 0 ? 255 : 0;
    }

    auto width = game.frame_buffer.width();
    auto height = game.frame_buffer.height();

    stbi_write_png(path.c_str(), width, height, channels, data, width * channels);
    delete[] data;
}

inline std::string leading(int value, int total_length) {
    std::stringstream ss;
    ss << std::setw(total_length) << std::setfill('0') << value;
    return ss.str();
}

int main(int argc, char** argv){
    Arguments args{slice<char*>::from_raw(++argv, argc-1)};
    args.print();

    Game* game;
    switch (args.scene) {
        case Scenes::Halo:
            game = new HaloGame(FrameBuffer{args.width, args.height});
            break;
        case Scenes::Brick:
            game = new BrickGame(FrameBuffer{args.width, args.height});
            break;
        case Scenes::Test:
            game = new TestGame(FrameBuffer{args.width, args.height});
            break;
        default:
            std::cout << "Invalid scene argument passed" << std::endl;
            exit(-1);
    }

    f64 duration = 3.;
    u64 frames = 300;
    u64 total_ms = 0;
    for (int i = 0; i < frames; i ++) {
        auto start = std::chrono::high_resolution_clock::now();
        game->update(1.f/duration, i*duration/frames);
        game->render();

        auto end = std::chrono::high_resolution_clock::now();
        auto duration = end - start;
        long long milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
        total_ms += milliseconds;
        std::cout << "Frame: " << (i+1) << " Render Time: " << milliseconds << " ms" << std::endl;

        if (args.write_frames)
            write_image(*game, "../animation/frame_" + leading(i, 3) + ".png");
    }
    std::cout << "average frame time: " << (total_ms/300.0) << "ms" << std::endl;
}

#endif