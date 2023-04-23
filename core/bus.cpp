#include "bus.h"

void ps1::bus_connect(bus_t* bus, device_info_t device_info) {
    bus->devices.emplace_back(device_info);
}

uint32_t ps1::bus_fetch32(bus_t* bus, mem_addr_t mem_addr) {
    ASSERT(mem_addr % 4 == 0, "Unaligned memory access");

    for (auto& device_info : bus->devices) {
        if (device_info.mem_range.contains(mem_addr)) {
            return device_info.fetch32(device_info.device, device_info.mem_range.offset(mem_addr));
        }
    }

    ASSERT(false, "Unmapped memory address");

    return 0;
}

void ps1::bus_store32(bus_t* bus, mem_addr_t mem_addr, uint32_t data) {
    ASSERT(mem_addr % 4 == 0, "Unaligned memory access");
    
    for (auto& device_info : bus->devices) {
        if (device_info.mem_range.contains(mem_addr)) {
            device_info.store32(device_info.device, device_info.mem_range.offset(mem_addr), data);

            return;
        }
    }

    ASSERT(false, "Unmapped memory address");
}

void ps1::bus_store16(bus_t* bus, mem_addr_t mem_addr, uint16_t data) {
    ASSERT(mem_addr % 2 == 0, "Unaligned memory access");
    
    for (auto& device_info : bus->devices) {
        if (device_info.mem_range.contains(mem_addr)) {
            ASSERT(device_info.store16, "16 bit mode store is not implemented on device");

            device_info.store16(device_info.device, device_info.mem_range.offset(mem_addr), data);

            return;
        }
    }

    ASSERT(false, "Unmapped memory address");
}