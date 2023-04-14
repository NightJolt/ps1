#include "bios.h"
#include "file.h"

void ps1::bios_init(bios_t* bios, const str_t& path) {
    auto binary_data = read_binary(path);
    
    ASSERT(binary_data.has_value(), "Failed to load BIOS binary");
    ASSERT(binary_data.value().size() == BIOS_SIZE, "BIOS binary size is invalid");

    bios->data = std::move(binary_data.value());
}

uint32_t ps1::bios_fetch32(void* bios, mem_addr_t offset) {
    // uint32_t data32 = 0;

    // for (int i = 0; i < sizeof uint32_t; i++) {
    //     data32 |= bios->data[offset + i] << (i << 3);
    // }

    // return data32;

    return *(uint32_t*)(((bios_t*)bios)->data.data() + offset); // ! faster but not portable
}

void ps1::bios_store32(void* bios, mem_addr_t offset, uint32_t value) {
    ASSERT(false, "BIOS is read-only");
}