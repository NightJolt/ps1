#pragma once

#include "defs.h"
#include "peripheral.h"
#include "logger.h"

namespace ps1 {
    struct gpu_t {
        uint8_t* data;
    };

    FETCH_FN(gpu_t) fetch(void* gpu, mem_addr_t offset) {
        DEBUG_CODE(logger::push("fetching", logger::type_t::warning, "gpu"));

        return offset == 4 ? 0x10000000 : 0;
    }

    STORE_FN(gpu_t) store(void* gpu, mem_addr_t offset, type_t value) {
        DEBUG_CODE(logger::push("storing", logger::type_t::warning, "gpu"));
    }
}