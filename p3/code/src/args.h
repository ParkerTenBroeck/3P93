#ifndef ARGS_H
#define ARGS_H

#include <iostream>
#include <util/slice.h>

#include "mpi/codec.h"

struct Scenes {

    enum Kind : u32{
        Halo,
        Brick,
        Bricks,
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
            case Bricks:
                return "bricks";
            case Test:
                return "test";
        }
    }
};

struct Arguments {
#ifdef USE_OPEN_MPI
    friend mpi::codec<Arguments>;
#endif
private:
    Arguments() {}
public:

    usize width = 640, height = 480;
    Scenes scene = Scenes::Test;
    bool write_frames = true;
    u32 frames = 300;
    f32 framerate = 30.0;
    bool freecam = false;

    explicit Arguments(char** argv, int argc) : Arguments(slice<char*>::from_raw(++argv, argc-1)){}

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
            }else if (arg.rfind("--frames=")==0) {
                try {
                    frames = std::stoi(arg.substr(1+arg.find_first_of('=')));
                }catch (std::exception& e) {
                    std::cout << "Invalid frames argument expected positive integer: " << e.what() << std::endl;
                }
            }else if (arg.rfind("--freecam=")==0) {
                freecam = true;
            }else if (arg.rfind("--framerate=")==0) {
                try {
                    framerate = std::stof(arg.substr(1+arg.find_first_of('=')));
                }catch (std::exception& e) {
                    std::cout << "Invalid framerate argument expected float: " << e.what() << std::endl;
                }
            }else if (arg.rfind("--scene=")==0) {
                std::string name = arg.substr(1+arg.find_first_of('='));
                if (name == "halo") {
                    scene = Scenes::Halo;
                } else if (name == "brick") {
                    scene = Scenes::Brick;
                } else if (name == "bricks") {
                    scene = Scenes::Bricks;
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

#endif //ARGS_H