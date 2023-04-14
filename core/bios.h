#pragma once

#include "defs.h"
#include "peripheral.h"

namespace ps1 {
    struct bios_t {
        dyn_arr_t <uint8_t> data;
    };

    void bios_init(bios_t*, const str_t&);

    uint32_t bios_fetch32(void*, mem_addr_t);
    void bios_store32(void*, mem_addr_t, uint32_t);
}