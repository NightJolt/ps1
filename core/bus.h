#pragma once

#include "defs.h"
#include "peripheral.h"

namespace ps1 {
    typedef uint32_t(*fetch32_func)(void*, mem_addr_t);
    typedef void(*store32_func)(void*, mem_addr_t, uint32_t);

    struct device_info_t {
        void* device;
        mem_range_t mem_range;
        fetch32_func fetch32;
        store32_func store32;
    };

    struct bus_t {
        dyn_arr_t <device_info_t> devices;
    };

    void bus_connect(bus_t*, device_info_t);

    uint32_t bus_fetch32(bus_t*, mem_addr_t);
    void bus_store32(bus_t*, mem_addr_t, uint32_t);
}