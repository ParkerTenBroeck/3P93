
#include "tiny_obj_loader.h"
#include "stb_image_write.h"
#include "stb_image.h"

#include "game.h"

#include <chrono>

int main(){
    Game game;
    game.update(0, 0);



    for (int i = 0; i < 1; i ++) {
        auto start = std::chrono::high_resolution_clock::now();

        game.render();

        auto end = std::chrono::high_resolution_clock::now();
        auto duration = end - start;
        long long milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
        std::cout << "Render Time: " << milliseconds << " ms" << std::endl;
    }


    auto data = new u8[game.frame_buffer.height()*game.frame_buffer.width()*3];
    for (usize i = 0; i < game.frame_buffer.height() * game.frame_buffer.width(); i++) {
        auto color = game.frame_buffer[i].diffuse;
        // auto color = (game.frame_buffer[i].normal.normalize()*0.5).add_scalar(0.5f);
        data[i*3] = static_cast<u8>(std::min(255.f, std::powf(color.x(), 1/2.2) * 255));
        data[i*3+1] = static_cast<u8>(std::min(255.f, std::powf(color.y(), 1/2.2) * 255));
        data[i*3+2] = static_cast<u8>(std::min(255.f, std::powf(color.z(), 1/2.2) * 255));
    }
    stbi_write_png("../output.png", game.frame_buffer.width(), game.frame_buffer.height(), 3, data, game.frame_buffer.width() * 3);
    delete[] data;
}
