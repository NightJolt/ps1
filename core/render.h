#pragma once

#include "defs.h"

namespace ps1::render {
    GLFWwindow* init();
    void exit();

    void begin_frame();
    void end_frame();

    bool should_close();

    uint32_t make_shader(const char*, const char*);
}