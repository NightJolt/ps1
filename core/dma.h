#pragma once

#include "defs.h"
#include "peripheral.h"
#include "logger.h"

namespace ps1 {
    struct dma_t {
        uint32_t control; // * +0x70
    };

    void dma_init(dma_t*);
    void dma_exit(dma_t*);
    
    // void dma_save_state(dma_t*);
    // void dma_load_state(dma_t*);

    FETCH_FN(dma_t) fetch(void* dma, mem_addr_t offset) {
        DEBUG_CODE(logger::push("fetching", logger::type_t::warning, "dma"));

        return offset == 0x70 ? ((device_t*)dma)->control : 0;
    }

    STORE_FN(dma_t) store(void* dma, mem_addr_t offset, type_t value) {
        DEBUG_CODE(logger::push("storing", logger::type_t::warning, "dma"));
    }
}