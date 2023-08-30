#pragma once

#include "defs.h"

namespace ps1::file {
    uint8_t* read_binary(const str_t&);
    std::string read_text(const str_t&);

    void open_writable(const str_t&);
    void write(uint8_t*, size_t);
    void write32(uint32_t);
    void close_writable();
    
    void open_readable(const str_t&);
    void read(uint8_t*, size_t);
    uint32_t read32();
    void close_readable();
}