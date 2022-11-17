#include "bios.h"
#include "file.h"

namespace ps1 {
    constexpr uint32_t bios_binary_size = 512 * 1024;
}

ps1::bios_t::bios_t(const str_t& path) {
    auto binary_data = read_binary(path);
    
    ASSERT(binary_data.has_value(), "Failed to load BIOS binary");
    ASSERT(binary_data.value().size() == bios_binary_size, "BIOS binary size is invalid");

    data = std::move(binary_data.value());
}

uint32_t ps1::bios_t::fetch32(mem_addr_t offset) const {
    ASSERT(offset < 0 || offset + sizeof cpu_instr_t <= data.size(), "BIOS offset is out of bounds");

    // uint32_t data32 = 0;

    // for (int i = 0; i < sizeof uint32_t; i++) {
    //     data32 |= data[offset + i] << (i << 3);
    // }

    // return data32;

    return *(cpu_instr_t*)(data.data() + offset); // ! probably faster but not portable
}