
#include "tiny_obj_loader.h"
#include "stb_image_write.h"
#include "stb_image.h"

#include "game.h"


int main(){
    // Matrix<f32, 2, 3> m1{
    //     1, 2, 3,
    //     4, 5, 6
    // };
    // Matrix<f32, 3, 2> m2{
    //     10, 11,
    //     20, 21,
    //     30, 31,
    // };
    // m1.print();
    // m2.print();
    //
    // (m1*m2).print();
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
