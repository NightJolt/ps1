#include "bios.h"
#include "file.h"

ps1::bios_t::bios_t(const str_t& path) {
    auto binary_data = read_binary(path);
    
    ASSERT(binary_data.has_value(), "Failed to load BIOS binary");
    ASSERT(binary_data.value().size() == BIOS_SIZE, "BIOS binary size is invalid");

    data = std::move(binary_data.value());
}

uint32_t ps1::bios_t::fetch32(mem_addr_t offset) const {
    // uint32_t data32 = 0;

    // for (int i = 0; i < sizeof uint32_t; i++) {
    //     data32 |= data[offset + i] << (i << 3);
    // }

    // return data32;

    return *(uint32_t*)(data.data() + offset); // ! probably faster but not portable
}

void ps1::bios_t::store32(mem_addr_t offset, uint32_t value) {
    ASSERT(false, "BIOS is read-only");
}