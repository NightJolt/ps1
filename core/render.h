#pragma once

#include "defs.h"

namespace ps1::render {
    GLFWwindow* init();
    void exit();

    void begin_frame();
    void end_frame();

    bool should_close();

    void make_shader(const char*, const char*, uint32_t);
    void use_shader(uint32_t);
}