#pragma once

#include "defs.h"
#include "peripheral.h"
#include "logger.h"

namespace ps1 {
    struct nodevice_t {};

    uint32_t nodevice_fetch32(void*, mem_addr_t);
    void nodevice_store32(void*, mem_addr_t, uint32_t);
    void nodevice_store16(void*, mem_addr_t, uint16_t);
    void nodevice_store8(void*, mem_addr_t, uint8_t);
}