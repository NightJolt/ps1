#pragma once

#include "defs.h"
#include "peripheral.h"

namespace ps1 {
    struct ram_t {
        uint8_t* data = nullptr;
    };

    void ram_init(ram_t*);
    void ram_exit(ram_t*);
    
    void ram_save_state(ram_t*);
    void ram_load_state(ram_t*);

    FETCH_FN(ram_t) fetch(void* ram, mem_addr_t offset) {
        return *(type*)(((device*)ram)->data + offset);
    }

    STORE_FN(ram_t) store(void* ram, mem_addr_t offset, type value) {
        *(type*)(((device*)ram)->data + offset) = value;
    }
}