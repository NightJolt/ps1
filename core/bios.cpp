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