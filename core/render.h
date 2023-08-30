#pragma once

#include "defs.h"

namespace ps1::render {
    GLFWwindow* init();
    void exit();

    void begin_frame();
    void end_frame();

    bool should_close();

    void load_vertex_shader(const char*);
    void load_fragment_shader(const char*);

    uint32_t get_vertex_shader();
    uint32_t get_fragment_shader();
}