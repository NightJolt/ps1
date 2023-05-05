#pragma once

#include "defs.h"
#include "peripheral.h"

namespace ps1 {
    typedef uint32_t(*fetch32_func)(void*, mem_addr_t);
    typedef uint16_t(*fetch16_func)(void*, mem_addr_t);
    typedef uint8_t(*fetch8_func)(void*, mem_addr_t);

    typedef void(*store32_func)(void*, mem_addr_t, uint32_t);
    typedef void(*store16_func)(void*, mem_addr_t, uint16_t);
    typedef void(*store8_func)(void*, mem_addr_t, uint8_t);

    struct device_info_t {
        void* device;
        mem_range_t mem_range;

        fetch32_func fetch32 = nullptr;
        fetch16_func fetch16 = nullptr;
        fetch8_func fetch8 = nullptr;

        store32_func store32 = nullptr;
        store16_func store16 = nullptr;
        store8_func store8 = nullptr;
    };

    struct bus_t {
        dyn_arr_t <device_info_t> devices;
    };

    void bus_connect(bus_t*, device_info_t);

    uint32_t bus_fetch32(bus_t*, mem_addr_t);
    uint16_t bus_fetch16(bus_t*, mem_addr_t);
    uint8_t bus_fetch8(bus_t*, mem_addr_t);

    void bus_store32(bus_t*, mem_addr_t, uint32_t);
    void bus_store16(bus_t*, mem_addr_t, uint16_t);
    void bus_store8(bus_t*, mem_addr_t, uint8_t);
}