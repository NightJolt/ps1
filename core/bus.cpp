#include "bus.h"

void ps1::bus_init(bus_t* bus) {}

void ps1::bus_exit(bus_t* bus) {
    bus->devices.clear();
}

void ps1::bus_connect(bus_t* bus, device_info_t device_info) {
    bus->devices.emplace_back(device_info);
}

uint32_t ps1::bus_fetch32(bus_t* bus, mem_addr_t mem_addr) {
    ASSERT(mem_addr % 4 == 0, "Unaligned memory access");

    mem_addr = mask_addr(mem_addr);

    for (auto& device_info : bus->devices) {
        if (device_info.mem_range.contains(mem_addr)) {
            ASSERT(device_info.fetch32, "32 bit mode fetch is not implemented on this device");

            return device_info.fetch32(device_info.device, device_info.mem_range.offset(mem_addr));
        }
    }

    ASSERT(false, "Unmapped memory address");

    return 0;
}

uint16_t ps1::bus_fetch16(bus_t* bus, mem_addr_t mem_addr) {
    ASSERT(mem_addr % 2 == 0, "Unaligned memory access");

    mem_addr = mask_addr(mem_addr);

    for (auto& device_info : bus->devices) {
        if (device_info.mem_range.contains(mem_addr)) {
            ASSERT(device_info.fetch16, "16 bit mode fetch is not implemented on this device");

            return device_info.fetch16(device_info.device, device_info.mem_range.offset(mem_addr));
        }
    }

    ASSERT(false, "Unmapped memory address");

    return 0;
}

uint8_t ps1::bus_fetch8(bus_t* bus, mem_addr_t mem_addr) {
    mem_addr = mask_addr(mem_addr);

    for (auto& device_info : bus->devices) {
        if (device_info.mem_range.contains(mem_addr)) {
            ASSERT(device_info.fetch8, "8 bit mode fetch is not implemented on this device");

            return device_info.fetch8(device_info.device, device_info.mem_range.offset(mem_addr));
        }
    }

    ASSERT(false, "Unmapped memory address");

    return 0;
}

void ps1::bus_store32(bus_t* bus, mem_addr_t mem_addr, uint32_t data) {
    ASSERT(mem_addr % 4 == 0, "Unaligned memory access");

    mem_addr = mask_addr(mem_addr);
    
    for (auto& device_info : bus->devices) {
        if (device_info.mem_range.contains(mem_addr)) {
            ASSERT(device_info.store32, "32 bit mode store is not implemented on this device");

            device_info.store32(device_info.device, device_info.mem_range.offset(mem_addr), data);

            return;
        }
    }

    ASSERT(false, "Unmapped memory address");
}

void ps1::bus_store16(bus_t* bus, mem_addr_t mem_addr, uint16_t data) {
    ASSERT(mem_addr % 2 == 0, "Unaligned memory access");

    mem_addr = mask_addr(mem_addr);

    for (auto& device_info : bus->devices) {
        if (device_info.mem_range.contains(mem_addr)) {
            ASSERT(device_info.store16, "16 bit mode store is not implemented on this device");

            device_info.store16(device_info.device, device_info.mem_range.offset(mem_addr), data);

            return;
        }
    }

    ASSERT(false, "Unmapped memory address");
}

void ps1::bus_store8(bus_t* bus, mem_addr_t mem_addr, uint8_t data) {
    mem_addr = mask_addr(mem_addr);

    for (auto& device_info : bus->devices) {
        if (device_info.mem_range.contains(mem_addr)) {
            ASSERT(device_info.store8, "8 bit mode store is not implemented on this device");

            device_info.store8(device_info.device, device_info.mem_range.offset(mem_addr), data);

            return;
        }
    }

    ASSERT(false, "Unmapped memory address");
}