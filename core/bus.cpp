#include "bus.h"

void ps1::bus_t::add_device(peripheral_i* device, mem_range_t range) {
    devices.emplace_back(range, device);
}

uint32_t ps1::bus_t::fetch32(mem_addr_t mem_addr) const {
    for (const auto& [range, device] : devices) {
        if (range.contains(mem_addr)) {
            return device->fetch32(range.offset(mem_addr));
        }
    }

    return 0;
}