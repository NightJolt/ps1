#pragma once

#include "defs.h"

namespace ps1 {
    struct vram_t {
        uint32_t fbo; // * frame buffer object
        uint32_t tbo; // * texture buffer object. used for rendering final result
        uint32_t rbo; // * render buffer object
        uint32_t vbo; // * vertex buffer object. used for drawing triangles
    };

    struct pos_t {
        pos_t(uint32_t v) : x(float((int16_t)v) / 1024.f), y(float((int16_t)(v >> 16)) / 512.f) {}

        float x;
        float y;
    };

    struct rgb_t {
        rgb_t(uint32_t v) : r(float((uint8_t)v) / 255.f), g(float((uint8_t)(v >> 8)) / 255.f), b(float((uint8_t)(v >> 16)) / 255.f) {}
        rgb_t(float r, float g, float b) : r(r), g(g), b(b) {}

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

    struct quad_t {
        vertex_t vertices[4];
    };

    void vram_init(vram_t*);
    void vram_exit(vram_t*);

    void vram_draw_triangle(vram_t*, triangle_t);
    void vram_draw_quad(vram_t*, quad_t);
}