#ifndef ARGS_H
#define ARGS_H

#include <iostream>

#include <game.h>

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
    usize width = 720, height = 480;
    Scenes scene = Scenes::Test;
    bool write_frames = false;

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

    Game* make_game() {
        switch (this->scene) {
            case Scenes::Halo:
                return new Game(FrameBuffer{this->width, this->height});
            case Scenes::Brick:
                return new Game(FrameBuffer{this->width, this->height});
            case Scenes::Test:
                return new Game(FrameBuffer{this->width, this->height});
            default:
                std::cout << "Invalid scene argument passed" << std::endl;
                exit(-1);
        }
    }

    static Game* from_args(char **argv, int argc) {
        Arguments args{slice<char*>::from_raw(++argv, argc-1)};
        args.print();
        return args.make_game();
    }
};

#endif //ARGS_H