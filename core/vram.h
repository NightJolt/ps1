#pragma once

#include "defs.h"

namespace ps1 {
    struct vram_t {
        uint32_t fbo; // * frame buffer object
        uint32_t tbo; // * texture buffer object. used for rendering final result
        uint32_t rbo; // * render buffer object
        uint32_t vbo; // * vertex buffer object. used for drawing triangles
    };    

    // struct pos_t {
    //     pos_t(uint32_t v) : x((int16_t)v), y((int16_t)(v >> 16)) {}

    //     int32_t x;
    //     int32_t y;
    // };

    // struct rgb_t {
    //     rgb_t(uint32_t v) :  r((int8_t)v), g((int8_t)(v >> 8)), b((int8_t)(v >> 16)) {}

    //     uint8_t r;
    //     uint8_t g;
    //     uint8_t b;
    // };

    struct pos_t {
        pos_t(uint32_t v) : x(float((int16_t)v) / 1024.f), y(float((int16_t)(v >> 16)) / 512.f) {}

        float x;
        float y;
    };

    struct rgb_t {
        rgb_t(uint32_t v) :  r(float((uint8_t)v) / 255.f), g(float((uint8_t)(v >> 8)) / 255.f), b(float((uint8_t)(v >> 16)) / 255.f) {}

        float r;
        float g;
        float b;
    };

    struct vertex_t {
        pos_t pos;
        rgb_t rgb;
    };

    struct triangle_t {
        vertex_t vertices[3];
    };

    void vram_init(vram_t*);
    void vram_exit(vram_t*);

    void vram_draw_triangle(vram_t*, triangle_t);
}