#include "vram.h"

void ps1::vram_init(vram_t* vram) {
    glGenBuffers(1, &vram->buffer_id);
    glBindBuffer(GL_ARRAY_BUFFER, vram->buffer_id);

    uint32_t mem_size = 1024 * 512 * 4;

    glBufferStorage(GL_ARRAY_BUFFER, mem_size, nullptr, GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT);
    vram->buffer = glMapBufferRange(GL_ARRAY_BUFFER, 0, mem_size, GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT);
}

void ps1::vram_exit(vram_t* vram) {
    glBindBuffer(GL_ARRAY_BUFFER, vram->buffer_id);
    glUnmapBuffer(GL_ARRAY_BUFFER);
    glDeleteBuffers(1, &vram->buffer_id);
}
