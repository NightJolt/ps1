#include "vram.h"
#include "render.h"
#include "logger.h"

namespace {
    constexpr uint32_t vram_width = 1024;
    constexpr uint32_t vram_height = 512;
}

namespace ps1 {
    void tsb_init(texture_stream_buffer_t* tsb) {
        tsb->buffer = new float[vram_width * vram_height * 3];
    }

    void tsb_exit(texture_stream_buffer_t* tsb) {
        delete[] tsb->buffer;
    }
}

void ps1::vram_init(vram_t* vram) {
    {
        glGenFramebuffers(1, &vram->fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, vram->fbo);

        glGenTextures(1, &vram->tbo);
        glBindTexture(GL_TEXTURE_2D, vram->tbo);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, vram_width, vram_height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
        
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, vram->tbo, 0);

        glGenRenderbuffers(1, &vram->rbo);
        glBindRenderbuffer(GL_RENDERBUFFER, vram->rbo);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, vram_width, vram_height);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, vram->rbo);
        
        glClearColor(.0f, .0f, .0f, 1.f);
        glClear(GL_COLOR_BUFFER_BIT);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    {
        glGenBuffers(1, &vram->vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vram->vbo);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(vertex_t), 0);

        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_t), (const void*)sizeof(pos_t));
    }

    tsb_init(&vram->texture_stream_buffer);
}

void ps1::vram_exit(vram_t* vram) {
    glDeleteFramebuffers(1, &vram->fbo);
    glDeleteTextures(1, &vram->tbo);
    glDeleteRenderbuffers(1, &vram->rbo);
    glDeleteBuffers(1, &vram->vbo);

    tsb_exit(&vram->texture_stream_buffer);
}

void ps1::vram_draw_triangle(vram_t* vram, triangle_t triangle) {
    glBindFramebuffer(GL_FRAMEBUFFER, vram->fbo);
    glViewport(0, 0, vram_width, vram_height);
    
    glBindBuffer(GL_ARRAY_BUFFER, vram->vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(triangle_t), &triangle, GL_STATIC_DRAW);
    glDrawArrays(GL_TRIANGLES, 0, 3);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void ps1::vram_draw_quad(vram_t* vram, quad_t quad) {
    triangle_t triangles[2] = {
        { quad.vertices[0], quad.vertices[1], quad.vertices[2] },
        { quad.vertices[1], quad.vertices[2], quad.vertices[3] }
    };

    glBindFramebuffer(GL_FRAMEBUFFER, vram->fbo);
    glViewport(0, 0, vram_width, vram_height);

    glBindBuffer(GL_ARRAY_BUFFER, vram->vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(triangle_t) * 2, triangles, GL_STATIC_DRAW);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void ps1::vram_set_texture_stream_specs(vram_t* vram, uint32_t xpos, uint32_t ypos, uint32_t width, uint32_t height) {
    vram->texture_stream_buffer.xpos = xpos;
    vram->texture_stream_buffer.ypos = ypos;
    vram->texture_stream_buffer.width = width;
    vram->texture_stream_buffer.height = height;
    vram->texture_stream_buffer.texels_left = width * height;
    vram->texture_stream_buffer.index = 0;
}

void ps1::vram_send_texture_stream_data(vram_t* vram, uint32_t data) {
    struct stream_rgb_t {
        uint16_t r : 5;
        uint16_t g : 5;
        uint16_t b : 5;
        uint16_t mask : 1;
    };

    stream_rgb_t* rgb_0 = (stream_rgb_t*)(((uint16_t*)&data) + 1);
    stream_rgb_t* rgb_1 = (stream_rgb_t*)((uint16_t*)&data);

    vram->texture_stream_buffer.buffer[vram->texture_stream_buffer.index++] = float(rgb_1->r) / 31.f;
    vram->texture_stream_buffer.buffer[vram->texture_stream_buffer.index++] = float(rgb_1->g) / 31.f;
    vram->texture_stream_buffer.buffer[vram->texture_stream_buffer.index++] = float(rgb_1->b) / 31.f;

    vram->texture_stream_buffer.buffer[vram->texture_stream_buffer.index++] = float(rgb_0->r) / 31.f;
    vram->texture_stream_buffer.buffer[vram->texture_stream_buffer.index++] = float(rgb_0->g) / 31.f;
    vram->texture_stream_buffer.buffer[vram->texture_stream_buffer.index++] = float(rgb_0->b) / 31.f;

    vram->texture_stream_buffer.texels_left -= 2;

    if (vram->texture_stream_buffer.texels_left == 0) {
        glBindTexture(GL_TEXTURE_2D, vram->tbo);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
		glTexSubImage2D(GL_TEXTURE_2D, 0, vram->texture_stream_buffer.xpos, vram->texture_stream_buffer.ypos, vram->texture_stream_buffer.width, vram->texture_stream_buffer.height, GL_RGB, GL_FLOAT, vram->texture_stream_buffer.buffer);

        logger::push("rendered texture stream", logger::type_t::message, "vram");

        vram->texture_stream_buffer.index = 0;
    }
}
