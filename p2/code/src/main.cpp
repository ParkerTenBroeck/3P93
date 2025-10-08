
#include "tiny_obj_loader.h"
#include "stb_image_write.h"
#include "stb_image.h"

#include "game.h"

#include <chrono>


void write_image(ref<Game> game, std::string&& path) {
    auto data = new u8[game.frame_buffer.height()*game.frame_buffer.width()*3];
    for (usize i = 0; i < game.frame_buffer.height() * game.frame_buffer.width(); i++) {
        auto color = game.frame_buffer[i].diffuse;
        // auto color = (game.frame_buffer[i].normal.normalize()*0.5).add_scalar(0.5f);
        data[i*3] = static_cast<u8>(std::min(255.f, std::powf(color.x(), 1/2.2) * 255));
        data[i*3+1] = static_cast<u8>(std::min(255.f, std::powf(color.y(), 1/2.2) * 255));
        data[i*3+2] = static_cast<u8>(std::min(255.f, std::powf(color.z(), 1/2.2) * 255));
    }
    stbi_write_png(path.c_str(), game.frame_buffer.width(), game.frame_buffer.height(), 3, data, game.frame_buffer.width() * 3);
    delete[] data;
}

std::string leading(int value, int total_length) {
    std::stringstream ss;
    ss << std::setw(total_length) << std::setfill('0') << value;
    return ss.str();
}

int main(){
    Game game;



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
