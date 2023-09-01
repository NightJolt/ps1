#include "vram.h"
#include "render.h"

namespace {
    constexpr uint32_t vram_width = 1024;
    constexpr uint32_t vram_height = 512;
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
}

void ps1::vram_exit(vram_t* vram) {
    glDeleteFramebuffers(1, &vram->fbo);
    glDeleteTextures(1, &vram->tbo);
    glDeleteRenderbuffers(1, &vram->rbo);
}

void ps1::vram_draw_triangle(vram_t* vram, triangle_t triangle) {
    glBindBuffer(GL_ARRAY_BUFFER, vram->vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(triangle), &triangle, GL_STATIC_DRAW);

    glBindFramebuffer(GL_FRAMEBUFFER, vram->fbo);
    glViewport(0, 0, vram_width, vram_height);
    
    glDrawArrays(GL_TRIANGLES, 0, 3);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
