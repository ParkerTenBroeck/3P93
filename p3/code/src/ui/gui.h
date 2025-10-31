#ifndef GUI_H
#define GUI_H

#include <string>

#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include <util/types.h>
#include <args.h>

struct Key{
    u8 down : 1;
    u8 released : 1;
    u8 pressed : 1;
    u8 caps : 1;
    u8 super : 1;
    u8 shift : 1;
    u8 ctrl : 1;
    u8 alt : 1;
};

struct MouseButton{
    u8 down : 1;
    u8 released : 1;
    u8 pressed : 1;
    u8 caps : 1;
    u8 super : 1;
    u8 shift : 1;
    u8 ctrl : 1;
    u8 alt : 1;
};

struct InputState {
    f64 mouse_x = 0.0;
    f64 mouse_y = 0.0;
    f64 mouse_delta_x = 0.0;
    f64 mouse_delta_y = 0.0;
    f64 scroll_y = 0.0;
    f64 scroll_x = 0.0;
    Key keys[GLFW_KEY_LAST+1] = {};
    MouseButton mouse_buttons[GLFW_MOUSE_BUTTON_LAST+1] = {};
    std::string typped;
};

enum class VisualKind : int {
    Color = 'c',
    Depth = 'd',
    Normal = 'n',
    Bitangent = 'b',
    Tangent = 't',
    Position = 'p',
    Roughness = 'r',
    Metalic = 'm',
    X = 'x',
};

void run_gui(Arguments& args);

#endif
