#pragma once

#include "defs.h"

namespace ps1 {
    struct vram_t {
        uint32_t buffer_id;
        void* buffer;

        uint32_t vertex_shader;
        uint32_t fragment_shader;
    };

    void vram_init(vram_t* vram);
    void vram_exit(vram_t* vram);
}