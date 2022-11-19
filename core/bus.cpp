#include "bus.h"

void ps1::bus_t::add_device(peripheral_i* device, mem_range_t range) {
    devices.emplace_back(range, device);
}

uint32_t ps1::bus_t::fetch32(mem_addr_t mem_addr) const {
    ASSERT(mem_addr % 4 == 0, "Unaligned memory access");

    for (const auto& [range, device] : devices) {
        if (range.contains(mem_addr)) {
            return device->fetch32(range.offset(mem_addr));
        }
    }

    ASSERT(false, "Unmapped memory address");

    return 0;
}

void ps1::bus_t::store32(mem_addr_t mem_addr, uint32_t data) {
    ASSERT(mem_addr % 4 == 0, "Unaligned memory access");
    
    for (const auto& [range, device] : devices) {
        if (range.contains(mem_addr)) {
            device->store32(range.offset(mem_addr), data);
            return;
        }
    }

    ASSERT(false, "Unmapped memory address");
}