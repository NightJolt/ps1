#include "bios.h"
#include "file.h"
#include "logger.h"

void ps1::bios_init(bios_t* bios, const str_t& path) {
    bios->data = read_binary(path);
    
    ASSERT(bios->data, "Failed to load BIOS binary");

    DEBUG_CODE(logger::push("bios loaded", logger::type_t::info, "bios"));
}

void ps1::bios_del(bios_t* bios) {
    delete[] bios->data;
}

uint32_t ps1::bios_fetch32(void* bios, mem_addr_t offset) {
    // uint32_t data32 = 0;

    // for (int i = 0; i < sizeof uint32_t; i++) {
    //     data32 |= bios->data[offset + i] << (i << 3);
    // }

    // return data32;

    return *(uint32_t*)(((bios_t*)bios)->data + offset); // ! faster but not portable
}

void ps1::bios_store32(void* bios, mem_addr_t offset, uint32_t value) {
    ASSERT(false, "BIOS is read-only");
}