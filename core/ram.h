#pragma once

#include "defs.h"
#include "peripheral.h"

namespace ps1 {
    struct ram_t {
        uint8_t* data;
    };

    void ram_init(ram_t*);
    void ram_del(ram_t*);

    uint32_t ram_fetch32(void*, mem_addr_t);
    void ram_store32(void*, mem_addr_t, uint32_t);
}