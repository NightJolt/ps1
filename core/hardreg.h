#pragma once

#include "defs.h"
#include "peripheral.h"

namespace ps1 {
    struct hardreg_t {};

    uint32_t hardreg_fetch32(void*, mem_addr_t);
    void hardreg_store32(void*, mem_addr_t, uint32_t);
}