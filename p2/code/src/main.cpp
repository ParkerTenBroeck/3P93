
#include "tiny_obj_loader.h"
#include "stb_image_write.h"
#include "stb_image.h"

// #define PAR

#ifdef PAR
#include <omp.h>
#endif

#include "game.h"

#include <chrono>
#include <iomanip>

void write_image(ref<Game> game, std::string&& path) {
    auto channels = 4;
    auto data = new u8[game.frame_buffer.height()*game.frame_buffer.width()*channels];

    #ifdef PAR
    #pragma omp parallel for
    #endif
    for (usize i = 0; i < game.frame_buffer.height() * game.frame_buffer.width(); i++) {
        auto color = game.frame_buffer[i].diffuse;
        auto normal = game.frame_buffer[i].normal;
        // auto color = (game.frame_buffer[i].normal.normalize()*0.5).add_scalar(0.5f);
        data[i*channels] = static_cast<u8>(std::min(255.f, std::pow(color.x(), 1.f/2.2f) * 255));
        data[i*channels+1] = static_cast<u8>(std::min(255.f, std::pow(color.y(), 1.f/2.2f) * 255));
        data[i*channels+2] = static_cast<u8>(std::min(255.f, std::pow(color.z(), 1.f/2.2f) * 255));
        data[i*channels+3] = normal.magnitude_squared() != 0 ? 255 : 0;
    }

    auto width = game.frame_buffer.width();
    auto height = game.frame_buffer.height();

    stbi_write_png(path.c_str(), width, height, channels, data, width * channels);
    delete[] data;
}

std::string leading(int value, int total_length) {
    std::stringstream ss;
    ss << std::setw(total_length) << std::setfill('0') << value;
    return ss.str();
}

struct Scenes {

    enum Kind {
        Halo,
        Brick,
        Test,
    } kind;

    Scenes(Kind kind) : kind(kind) {} // NOLINT

    operator Kind() const { return kind; } // NOLINT

    std::string_view str() const { // NOLINT
        switch (kind) {
            case Halo:
                return "halo";
            case Brick:
                return "brick";
            case Test:
                return "test";
        }
    }
};

struct Arguments {
    usize width = 1920, height = 1080;
    Scenes scene = Scenes::Halo;
    bool write_frames = true;

    explicit Arguments(slice<char*> args) {
        for (char* carg : args.iter()) {
            std::string arg = carg;
            if (arg.rfind("--height=")==0) {
                try {
                    height = std::stoi(arg.substr(1+arg.find_first_of('=')));
                }catch (std::exception& e) {
                    std::cout << "Invalid height argument expected positive integer: " << e.what() << std::endl;
                }
            }else if (arg.rfind("--width=")==0) {
                try {
                    width = std::stoi(arg.substr(1+arg.find_first_of('=')));
                }catch (std::exception& e) {
                    std::cout << "Invalid width argument expected positive integer: " << e.what() << std::endl;
                }
            }else if (arg.rfind("--scene=")==0) {
                std::string name = arg.substr(1+arg.find_first_of('='));
                if (name == "halo") {
                    scene = Scenes::Halo;
                } else if (name == "brick") {
                    scene = Scenes::Brick;
                } else if (name == "test") {
                    scene = Scenes::Test;
                }else {
                    std::cout << "Invalid scene argument expected a string: " << name << std::endl;
                }
            }else if (arg.rfind("--write_frames=")==0) {
                std::string value = arg.substr(1+arg.find_first_of('='));
                std::cout << value;
                if (value == "true") {
                    write_frames = true;
                } else if (value == "false") {
                    write_frames = false;
                }else {
                    std::cout << "Invalid flag, can only be true|t|false|f got: " << value << std::endl;
                }
            }
        }
    }

    void print() const {
        std::cout <<
            "width: " << width <<
            " height: " << height <<
            " write_frames: " << (write_frames?"true":"false") <<
            " scene: " << scene.str() <<
            std::endl;
    }
};

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


    u64 total = 0;
    for (int i = 0; i < 300; i ++) {
        auto start = std::chrono::high_resolution_clock::now();
        game->update(1, i);
        game->render();

        auto end = std::chrono::high_resolution_clock::now();
        auto duration = end - start;
        long long milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
        total += milliseconds;
        std::cout << "Frame: " << (i+1) << " Render Time: " << milliseconds << " ms" << std::endl;

        if (args.write_frames)
            write_image(*game, "../animation/frame_" + leading(i, 3) + ".png");
    }
    std::cout << "average frame time: " << (total/300.0) << "ms" << std::endl;


}
