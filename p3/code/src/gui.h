//
// Created by may on 15/10/25.
//

#ifndef GUI_H
#define GUI_H

#include "game.h"
#include "args.h"
#include "glad/gl.h"
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <chrono>
#include <cctype>

struct Key{
    u8 down : 1;
    u8 released : 1;
    u8 pressed : 1;
};

struct InputState {
    double mouse_x = 0.0;
    double mouse_y = 0.0;
    double scroll_y = 0.0;
    double scroll_x = 0.0;
    Key keys[1024] = {false};
    std::string typped;
};

InputState input{};

// GLFW Callbacks
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    auto shift = GLFW_MOD_SHIFT&mods!=0;
    auto ctrl = GLFW_MOD_CONTROL&mods!=0;
    auto super = GLFW_MOD_SUPER&mods!=0;
    auto alt = GLFW_MOD_ALT&mods!=0;
    auto caps = GLFW_MOD_CAPS_LOCK&mods!=0;
    auto num = GLFW_MOD_NUM_LOCK&mods!=0;

    if (GLFW_KEY_SPACE <= key && key <= GLFW_KEY_GRAVE_ACCENT && !ctrl && !super && !alt) {
        if (GLFW_KEY_A <= key && key <= GLFW_KEY_Z) {
            input.typped += (char)key + (key != GLFW_KEY_SPACE && caps ^ shift?0:32);
        }else if (shift){
            input.typped += (char)std::toupper((char)key);
        }else {
            input.typped += (char)key;
        }
    }

    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    if (key >= 0 && key < 1024) {
        input.keys[key].pressed = !input.keys[key].down && action == GLFW_PRESS || action == GLFW_REPEAT != 0;
        input.keys[key].down = action == GLFW_PRESS != 0;
        input.keys[key].released = action == GLFW_RELEASE != 0;
    }
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    input.scroll_y += yoffset;
    input.scroll_x += xoffset;
}

void cursor_callback(GLFWwindow* window, double xpos, double ypos) {
    input.mouse_x = xpos;
    input.mouse_y = ypos;
}

// Error callback
void glfw_error_callback(int error, const char* desc) {
    std::cerr << "GLFW Error " << error << ": " << desc << std::endl;
}

void handle_game_input(Game* game, f32 delta) {
    delta *= 10;
    Vector3<f32> movement{0., 0., 0.};
    auto facing = game->scene.m_camera.position-game->scene.m_camera.target;
    if (input.keys[GLFW_KEY_W].down) {
        movement = movement - facing.normalize()*delta;
    }
    if (input.keys[GLFW_KEY_S].down) {
        movement = movement + facing.normalize()*delta;
    }
    if (input.keys[GLFW_KEY_A].down) {
        movement = movement - facing.cross(game->scene.m_camera.up).normalize()*delta;
    }
    if (input.keys[GLFW_KEY_D].down) {
        movement = movement + facing.cross(game->scene.m_camera.up).normalize()*delta;
    }
    movement.y() = 0.f;
    if (input.keys[GLFW_KEY_SPACE].down) {
        movement = movement + game->scene.m_camera.up * delta;
    }
    if (input.keys[GLFW_KEY_LEFT_SHIFT].down) {
        movement = movement - game->scene.m_camera.up * delta;
    }
    game->scene.m_camera.position = game->scene.m_camera.position+movement;
    game->scene.m_camera.target = game->scene.m_camera.target+movement;
}

int main(int argc, char** argv) {
    auto game = Arguments::from_args(argv, argc);

    glfwSetErrorCallback(glfw_error_callback);

    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW\n";
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(game->frame_buffer.width(), game->frame_buffer.height(), "CPU Image to OpenGL", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create window\n";
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    if (!gladLoaderLoadGL()) {
        std::cerr << "Failed to initialize GLAD\n";
        return -1;
    }

    glfwSetKeyCallback(window, key_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetCursorPosCallback(window, cursor_callback);

    // Create texture
    GLuint tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    std::vector<f32> pixels(game->frame_buffer.width() * game->frame_buffer.height() * 4);

    // Fullscreen quad setup
    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    // Simple texture display shader
    const char* vs = R"(
        #version 330 core
        out vec2 texCoord;
        const vec2 verts[4] = vec2[](
            vec2(-1.0, -1.0),
            vec2( 1.0, -1.0),
            vec2(-1.0,  1.0),
            vec2( 1.0,  1.0)
        );
        const vec2 uvs[4] = vec2[](
            vec2(0.0, 1.0),
            vec2(1.0, 1.0),
            vec2(0.0, 0.0),
            vec2(1.0, 0.0)
        );
        void main() {
            texCoord = uvs[gl_VertexID];
            gl_Position = vec4(verts[gl_VertexID], 0.0, 1.0);
        }
    )";

    const char* fs = R"(
        #version 330 core
        in vec2 texCoord;
        out vec4 FragColor;
        uniform sampler2D tex;
        void main() {
            FragColor = texture(tex, texCoord);
            FragColor.x = pow(FragColor.x, 1./2.2);
            FragColor.y = pow(FragColor.y, 1./2.2);
            FragColor.z = pow(FragColor.z, 1./2.2);
        }
    )";

    auto compileShader = [](GLenum type, const char* src) -> GLuint {
        GLuint s = glCreateShader(type);
        glShaderSource(s, 1, &src, nullptr);
        glCompileShader(s);
        int success;
        glGetShaderiv(s, GL_COMPILE_STATUS, &success);
        if (!success) {
            char log[512];
            glGetShaderInfoLog(s, 512, nullptr, log);
            std::cerr << "Shader compile error:\n" << log << std::endl;
        }
        return s;
    };

    GLuint vsID = compileShader(GL_VERTEX_SHADER, vs);
    GLuint fsID = compileShader(GL_FRAGMENT_SHADER, fs);
    GLuint prog = glCreateProgram();
    glAttachShader(prog, vsID);
    glAttachShader(prog, fsID);
    glLinkProgram(prog);
    glUseProgram(prog);
    glDeleteShader(vsID);
    glDeleteShader(fsID);

    glUniform1i(glGetUniformLocation(prog, "tex"), 0);

    usize frame_count = 0;
    const auto start = std::chrono::high_resolution_clock::now();
    auto frame_start = std::chrono::high_resolution_clock::now();

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        auto now = std::chrono::high_resolution_clock::now();

        f64 time = std::chrono::duration<f64>(now-start).count();
        f32 delta = std::chrono::duration<f32>(now-frame_start).count();
        frame_start = now;


        handle_game_input(game, delta);
        game->update(delta, time);
        game->render();
        for (usize i = 0; i < game->frame_buffer.width() * game->frame_buffer.height(); i++) {
            pixels[i*4+0] = game->frame_buffer[i].diffuse.x();
            pixels[i*4+1] = game->frame_buffer[i].diffuse.y();
            pixels[i*4+2] = game->frame_buffer[i].diffuse.z();
            pixels[i*4+3] = 1.f;
        }
        // std::cout << "Frame: " << frame_count << " Render Time: " <<  std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now()-frame_start).count() << " ms" << std::endl;
        std::cout << "Typped: " << (int)input.keys[GLFW_KEY_SPACE].down << std::endl;
        frame_count += 1;


        // Upload pixels to GPU
        glBindTexture(GL_TEXTURE_2D, tex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, game->frame_buffer.width(), game->frame_buffer.height(), 0,
                     GL_RGBA, GL_FLOAT, pixels.data());

        glClear(GL_COLOR_BUFFER_BIT);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        glfwSwapBuffers(window);

        input.scroll_x = 0.f;
        input.scroll_y = 0.f;
        input.typped.clear();
        for (auto & key : input.keys) {
            key.pressed = false;
            key.released = false;
        }
    }

    glDeleteTextures(1, &tex);
    glfwTerminate();
    return 0;
}

#endif
