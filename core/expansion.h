#pragma once

#include "defs.h"
#include "peripheral.h"

namespace ps1 {
    struct expansion_t {};

    uint8_t expansion_fetch8(void*, mem_addr_t);
}