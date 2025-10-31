
#ifdef GUI

#include <iostream>
#include <vector>
#include <chrono>
#include <cctype>

#include <game.h>
#include <args.h>
#include <ui/gui.h>

InputState input{};
VisualKind visual = VisualKind::Color;

void key_callback(GLFWwindow *window, int key, int /*scancode*/, int action, int mods) {
    const auto shift = (GLFW_MOD_SHIFT&mods)!=0;
    const auto ctrl = (GLFW_MOD_CONTROL&mods)!=0;
    const auto super = (GLFW_MOD_SUPER&mods)!=0;
    const auto alt = (GLFW_MOD_ALT&mods)!=0;
    const auto caps = (GLFW_MOD_CAPS_LOCK&mods)!=0;

    if (key < 0 || key > GLFW_KEY_LAST) return;

    if (GLFW_KEY_SPACE <= key && key <= GLFW_KEY_GRAVE_ACCENT && !ctrl && !super && !alt) {
        if (GLFW_KEY_A <= key && key <= GLFW_KEY_Z) {
            input.typped += key + (key != GLFW_KEY_SPACE && caps ^ shift?0:32);
        }else if (shift){
            input.typped += key;
        }else {
            input.typped += key;
        }
    }

    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    input.keys[key].shift = shift;
    input.keys[key].ctrl = ctrl;
    input.keys[key].super = super;
    input.keys[key].alt = alt;
    input.keys[key].caps = caps;

    input.keys[key].pressed = (!input.keys[key].down && action == GLFW_PRESS) || action == GLFW_REPEAT;
    if (action != GLFW_REPEAT) {
        input.keys[key].down = action == GLFW_PRESS;
        input.keys[key].released = action == GLFW_RELEASE;
    }
}

void scroll_callback(GLFWwindow */*window*/, double xoffset, double yoffset) {
    input.scroll_y += yoffset;
    input.scroll_x += xoffset;
}

void cursor_callback(GLFWwindow */*window*/, double xpos, double ypos) {
    input.mouse_delta_x += xpos - input.mouse_x;
    input.mouse_delta_y += ypos - input.mouse_y;
    input.mouse_x = xpos;
    input.mouse_y = ypos;
}

void mouse_button_callback(GLFWwindow */*window*/, int button, int action, int mods) {
    if (button < 0 || button > GLFW_MOUSE_BUTTON_LAST) return;

    input.mouse_buttons[button].down = action == GLFW_PRESS;
    input.mouse_buttons[button].pressed = action == GLFW_PRESS;
    input.mouse_buttons[button].released = action == GLFW_RELEASE;

    input.mouse_buttons[button].shift = (GLFW_MOD_SHIFT&mods)!=0;
    input.mouse_buttons[button].ctrl = (GLFW_MOD_CONTROL&mods)!=0;
    input.mouse_buttons[button].super = (GLFW_MOD_SUPER&mods)!=0;
    input.mouse_buttons[button].alt = (GLFW_MOD_ALT&mods)!=0;
    input.mouse_buttons[button].caps = (GLFW_MOD_CAPS_LOCK&mods)!=0;
}

void glfw_error_callback(int error, const char *desc) {
    std::cerr << "GLFW Error " << error << ": " << desc << std::endl;
}

void handle_game_input(Game *game, f32 delta) {
    Vector3<f32> movement{0., 0., 0.};
    // project facing vector onto the plane defined by the up normal
    auto facing = (game->scene.m_camera.target-game->scene.m_camera.position);
    facing = facing-(facing.dot(game->scene.m_camera.up)*game->scene.m_camera.up);
    facing = facing.normalize();

    const auto speed = input.keys[GLFW_KEY_LEFT_CONTROL].down?1.0f:0.3f;

    if (input.keys[GLFW_KEY_W].down) {
        movement = movement + speed*facing*delta;
    }
    if (input.keys[GLFW_KEY_S].down) {
        movement = movement - speed*facing*delta;
    }
    if (input.keys[GLFW_KEY_A].down) {
        movement = movement - speed*facing.cross(game->scene.m_camera.up).normalize()*delta;
    }
    if (input.keys[GLFW_KEY_D].down) {
        movement = movement + speed*facing.cross(game->scene.m_camera.up).normalize()*delta;
    }

    if (input.keys[GLFW_KEY_SPACE].down) {
        movement = movement + speed*game->scene.m_camera.up * delta;
    }
    if (input.keys[GLFW_KEY_LEFT_SHIFT].down) {
        movement = movement - speed*game->scene.m_camera.up * delta;
    }

    game->scene.m_camera.fov -= input.scroll_y/10.f;
    game->scene.m_camera.fov = std::clamp(game->scene.m_camera.fov, 0.00001f, M_PIf);

    game->scene.m_camera.position = game->scene.m_camera.position+movement*7;

    static f32 yaw = 0.f;
    static f32 pitch = 0.f;

    if (input.mouse_buttons[GLFW_MOUSE_BUTTON_LEFT].down) {
        yaw -= (f32)input.mouse_delta_x*0.005f;
        pitch -= (f32)input.mouse_delta_y*0.005f;
        pitch = std::clamp(pitch, -M_PIf/2 + 0.001f, M_PIf/2 - 0.001f);
    }
    facing.z() = std::cos(yaw) * std::cos(pitch);
    facing.y() = std::sin(pitch);
    facing.x() = std::sin(yaw) * std::cos(pitch);
    game->scene.m_camera.target = game->scene.m_camera.position+facing.normalize();

    game->scene.m_camera.target = facing+game->scene.m_camera.position;

    if (input.keys[GLFW_KEY_C].pressed) {
        visual = VisualKind::Color;
    }else if (input.keys[GLFW_KEY_F].pressed) {
        visual = VisualKind::Depth;
    }else if (input.keys[GLFW_KEY_N].pressed) {
        visual = VisualKind::Normal;
    }else if (input.keys[GLFW_KEY_B].pressed) {
        visual = VisualKind::Bitangent;
    }else if (input.keys[GLFW_KEY_T].pressed) {
        visual = VisualKind::Tangent;
    }else if (input.keys[GLFW_KEY_P].pressed) {
        visual = VisualKind::Position;
    }
}

void fill_buffer(const VisualKind visual, Game *game, std::vector<f32> &pixels) {
    switch (visual) {
        case VisualKind::Color: {
            #ifdef USE_OPEN_MP
            #pragma omp parallel for
            #endif
            for (usize i = 0; i < game->frame_buffer.width() * game->frame_buffer.height(); i++) {
                pixels[i*4+0] = game->frame_buffer[i].diffuse.x();
                pixels[i*4+1] = game->frame_buffer[i].diffuse.y();
                pixels[i*4+2] = game->frame_buffer[i].diffuse.z();
                pixels[i*4+3] = 1.f;
            }
        }break;
        case VisualKind::Depth: {
            #ifdef USE_OPEN_MP
            #pragma omp parallel for
            #endif
            for (usize i = 0; i < game->frame_buffer.width() * game->frame_buffer.height(); i++) {
                pixels[i*4+0] = game->frame_buffer[i].depth;
                pixels[i*4+3] = 1.f;
            }
        }break;
        case VisualKind::Normal: {

            #ifdef USE_OPEN_MP
            #pragma omp parallel for
            #endif
            for (usize i = 0; i < game->frame_buffer.width() * game->frame_buffer.height(); i++) {
                pixels[i*4+0] = game->frame_buffer[i].normal.x();
                pixels[i*4+1] = game->frame_buffer[i].normal.y();
                pixels[i*4+2] = game->frame_buffer[i].normal.z();
                pixels[i*4+3] = 1.f;
            }
        }break;
        case VisualKind::Bitangent: {

            #ifdef USE_OPEN_MP
            #pragma omp parallel for
            #endif
            for (usize i = 0; i < game->frame_buffer.width() * game->frame_buffer.height(); i++) {
                pixels[i*4+0] = game->frame_buffer[i].bitangent.x();
                pixels[i*4+1] = game->frame_buffer[i].bitangent.y();
                pixels[i*4+2] = game->frame_buffer[i].bitangent.z();
                pixels[i*4+3] = 1.f;
            }
        }break;
        case VisualKind::Tangent: {

            #ifdef USE_OPEN_MP
            #pragma omp parallel for
            #endif
            for (usize i = 0; i < game->frame_buffer.width() * game->frame_buffer.height(); i++) {
                pixels[i*4+0] = game->frame_buffer[i].tangent.x();
                pixels[i*4+1] = game->frame_buffer[i].tangent.y();
                pixels[i*4+2] = game->frame_buffer[i].tangent.z();
                pixels[i*4+3] = 1.f;
            }
        }break;
        case VisualKind::Position: {

            #ifdef USE_OPEN_MP
            #pragma omp parallel for
            #endif
            for (usize i = 0; i < game->frame_buffer.width() * game->frame_buffer.height(); i++) {
                pixels[i*4+0] = game->frame_buffer[i].position.x();
                pixels[i*4+1] = game->frame_buffer[i].position.y();
                pixels[i*4+2] = game->frame_buffer[i].position.z();
                pixels[i*4+3] = 1.f;
            }
        }break;
    }
}



void run_gui(Arguments& args) {

    auto game = args.make_game();

    f32 fps = 0;

    glfwSetErrorCallback(glfw_error_callback);

    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW\n";
        return;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(game->frame_buffer.width(), game->frame_buffer.height(), "CPU Rasterizer", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create window\n";
        glfwTerminate();
        return;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    if (!gladLoaderLoadGL()) {
        std::cerr << "Failed to initialize GLAD\n";
        return;
    }

    glfwSetKeyCallback(window, key_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetCursorPosCallback(window, cursor_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);

    // Create texture
    GLuint tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

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
        uniform int kind;
        void main() {
            FragColor = texture(tex, texCoord);

            if (kind == 0x63){ // c
                FragColor.x = pow(FragColor.x, 1./2.2);
                FragColor.y = pow(FragColor.y, 1./2.2);
                FragColor.z = pow(FragColor.z, 1./2.2);
            }else if (kind == 0x64){ // d
                FragColor.x = pow(1.0 - FragColor.x / 256.0 / 256.0 / 256.0 / 256.0, 1./2.2);
                FragColor.y = FragColor.x;
                FragColor.z = FragColor.x;
            }else if (kind == 0x62 || kind == 0x6e || kind == 0x74){  // b n t
                FragColor = FragColor/2+0.5;
            }

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
        i32 width, height;
        glfwGetWindowSize(window, &width, &height);
        glViewport(0, 0, width, height);
        glfwPollEvents();

        auto now = std::chrono::high_resolution_clock::now();

        f64 time = std::chrono::duration<f64>(now-start).count();
        f32 delta = std::chrono::duration<f32>(now-frame_start).count();
        frame_start = now;


        handle_game_input(game, delta);

        game->update(delta, time);
        game->render();

        fill_buffer(visual, game, pixels);

        auto render_time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now()-frame_start).count();
        // fps *= 99.f/100.f;
        // fps += (1.f/render_time*1000.f) / 100.f;
        frame_count += 1;
        fps = fps + ((1.f/static_cast<f32>(render_time)) * 1000 - fps) / static_cast<f32>(frame_count);
        std::cout << "Frame: " << frame_count << " Render Time: " <<  render_time << " ms, FPS: " << fps << std::endl;


        // Upload pixels to GPU
        int vertexColorLocation = glGetUniformLocation(prog, "kind");
        glUniform1i(vertexColorLocation, static_cast<GLint>(visual));
        glBindTexture(GL_TEXTURE_2D, tex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, game->frame_buffer.width(), game->frame_buffer.height(), 0,
                     GL_RGBA, GL_FLOAT, pixels.data());

        glClear(GL_COLOR_BUFFER_BIT);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        glfwSwapBuffers(window);

        input.scroll_x = 0.f;
        input.scroll_y = 0.f;
        input.mouse_delta_x = 0.f;
        input.mouse_delta_y = 0.f;
        input.typped.clear();
        for (auto & key : input.keys) {
            key.pressed = false;
            key.released = false;
        }
    }

    glDeleteTextures(1, &tex);
    glfwTerminate();
}


#endif