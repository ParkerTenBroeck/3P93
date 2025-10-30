#ifndef GUI_H
#define GUI_H

#include "../util/types.h"
#include <string>
#include "glad/gl.h"
#include <GLFW/glfw3.h>

struct Key{
    u8 down : 1 = 0;
    u8 released : 1 = 0;
    u8 pressed : 1 = 0;
    u8 caps : 1 = 0;
    u8 super : 1 = 0;
    u8 shift : 1 = 0;
    u8 ctrl : 1 = 0;
    u8 alt : 1 = 0;
};

struct MouseButton{
    u8 down : 1 = 0;
    u8 released : 1 = 0;
    u8 pressed : 1 = 0;
    u8 caps : 1 = 0;
    u8 super : 1 = 0;
    u8 shift : 1 = 0;
    u8 ctrl : 1 = 0;
    u8 alt : 1 = 0;
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
};

#endif
