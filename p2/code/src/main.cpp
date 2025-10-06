#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#undef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#undef STB_IMAGE_WRITE_IMPLEMENTATION
#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"
#undef TINYOBJLOADER_IMPLEMENTATION

#include "game.h"


int main(){
    Game game;
    game.update(0, 0);
    game.render();

    auto data = new u8[game.frame_buffer.height()*game.frame_buffer.width()*3];
    for (usize i = 0; i < game.frame_buffer.height() * game.frame_buffer.width(); i++) {
        auto n = game.frame_buffer[i].normal.normalize();
        data[i*3] = static_cast<u8>((n.x() + 1) / 2 * 255);
        data[i*3+1] = static_cast<u8>((n.y() + 1) / 2 * 255);
        data[i*3+2] = static_cast<u8>((n.z() + 1) / 2 * 255);
    }
    stbi_write_png("../output.png", game.frame_buffer.width(), game.frame_buffer.height(), 3, data, game.frame_buffer.width() * 3);
    delete[] data;
}
