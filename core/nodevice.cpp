#include "nodevice.h"

uint32_t ps1::nodevice_fetch32(void* device, mem_addr_t offset) {
    DEBUG_CODE(logger::push("fetching from nodevice", logger::type_t::warning));

    return 0;
}

void ps1::nodevice_store32(void* device, mem_addr_t offset, uint32_t value) {
    DEBUG_CODE(logger::push("storing into nodevice", logger::type_t::warning));
}