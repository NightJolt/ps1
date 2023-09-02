#pragma once

#include "defs.h"

namespace ps1 {
    struct texture_stream_buffer_t {
        float* buffer;
        uint32_t index;
        uint32_t texels_left;
        uint32_t xpos;
        uint32_t ypos;
        uint32_t width;
        uint32_t height;
    };

    struct vram_t {
        uint32_t fbo; // * frame buffer object
        uint32_t tbo; // * texture buffer object. used for rendering final result
        uint32_t rbo; // * render buffer object
        uint32_t vbo; // * vertex buffer object. used for drawing triangles

        texture_stream_buffer_t texture_stream_buffer; // * used for streaming texture data from cpu to gpu
    };

    struct pos_t {
        pos_t(uint32_t v) : x(float((int16_t)v) / 512.f - 1.f), y(float((int16_t)(v >> 16)) / 256.f - 1.f) {}
        pos_t(int16_t x, int16_t y) : x(float(x) / 1024.f), y(float(y) / 512.f) {}
        pos_t(float x, float y) : x(x), y(y) {}

        pos_t operator+(const pos_t& other) const {
            return pos_t(x + other.x, y + other.y);
        }

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

    void vram_set_texture_stream_specs(vram_t*, uint32_t, uint32_t, uint32_t, uint32_t);
    void vram_send_texture_stream_data(vram_t*, uint32_t);
}