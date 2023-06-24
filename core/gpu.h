#pragma once

#include "defs.h"
#include "peripheral.h"

namespace ps1 {
    struct gpu_t {
        uint8_t* data;
    };

    void gpu_init(gpu_t*);
    void gpu_del(gpu_t*);

    FETCH_FN(gpu_t) fetch(void* gpu, mem_addr_t offset) {
        return 0;
    }

    STORE_FN(gpu_t) store(void* gpu, mem_addr_t offset, type value) {
    }
}