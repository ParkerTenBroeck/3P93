
#include "tiny_obj_loader.h"
#include "stb_image_write.h"
#include "stb_image.h"

#include "game.h"

#include <chrono>
#include <iomanip>

void write_image(ref<Game> game, std::string&& path) {
    auto channels = 4;
    auto data = new u8[game.frame_buffer.height()*game.frame_buffer.width()*channels];
    for (usize i = 0; i < game.frame_buffer.height() * game.frame_buffer.width(); i++) {
        auto color = game.frame_buffer[i].diffuse;
        auto normal = game.frame_buffer[i].normal;
        // auto color = (game.frame_buffer[i].normal.normalize()*0.5).add_scalar(0.5f);
        data[i*channels] = static_cast<u8>(std::min(255.f, std::pow(color.x(), 1.f/2.2f) * 255));
        data[i*channels+1] = static_cast<u8>(std::min(255.f, std::pow(color.y(), 1.f/2.2f) * 255));
        data[i*channels+2] = static_cast<u8>(std::min(255.f, std::pow(color.z(), 1.f/2.2f) * 255));
        data[i*channels+3] = normal.magnitude_squared() != 0 ? 255 : 0;
    }
    stbi_write_png(path.c_str(), game.frame_buffer.width(), game.frame_buffer.height(), channels, data, game.frame_buffer.width() * channels);
    delete[] data;
}

std::string leading(int value, int total_length) {
    std::stringstream ss;
    ss << std::setw(total_length) << std::setfill('0') << value;
    return ss.str();
}

int main(int argc, char** argv){
    usize width = 720, height = 480;
    bool halo = false;
    // const auto width = 1920, height = 1080;
    // const auto width = 4096, height = 2160;

    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg.rfind("--height=")==0) {
            height = std::stoi(arg.substr(1+arg.find_first_of('=')));

        }else if (arg.rfind("--width=")==0) {
            width = std::stoi(arg.substr(1+arg.find_first_of('=')));
        }else if (arg.rfind("--scene=")==0) {
            std::string scene = arg.substr(1+arg.find_first_of('='));
            if (scene == "halo") {
                halo = true;
            } else if (scene == "brick") {
                halo = false;
            }
        }
    }
    std::cout << "width: " << width << " height: " << height << " scene: " << std::endl;

    if (argc == 3) {
        width = std::stol(argv[1]);
        height = std::stol(argv[2]);
    }
    Game game{FrameBuffer{width, height}, halo};



    for (int i = 0; i < 300; i ++) {
        auto start = std::chrono::high_resolution_clock::now();
        game.update(1, i);
        game.render();

        auto end = std::chrono::high_resolution_clock::now();
        auto duration = end - start;
        long long milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
        std::cout << "Frame: " << (i+1) << " Render Time: " << milliseconds << " ms" << std::endl;

        write_image(game, "../animation/frame_" + leading(i, 3) + ".png");
    }



}
