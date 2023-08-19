#pragma once

#include "defs.h"
#include "peripheral.h"

namespace ps1 {
    struct bios_t {
        uint8_t* data = nullptr;
    };

    void bios_init(bios_t*, const str_t&);
    void bios_exit(bios_t*);

    FETCH_FN(bios_t) fetch(void* bios, mem_addr_t offset) {
        return *(type_t*)(((device_t*)bios)->data + offset);
    }
}