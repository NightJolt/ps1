#include "nodevice.h"

uint32_t ps1::nodevice_fetch32(void* device, mem_addr_t offset) {
    DEBUG_CODE(logger::push("32 bit fetching from nodevice", logger::type_t::warning));

    return 0;
}

void ps1::nodevice_store32(void* device, mem_addr_t offset, uint32_t value) {
    DEBUG_CODE(logger::push("32 bit storing into nodevice", logger::type_t::warning));
}

void ps1::nodevice_store16(void* device, mem_addr_t offset, uint16_t value) {
    DEBUG_CODE(logger::push("16 bit storing into nodevice", logger::type_t::warning));
}

void ps1::nodevice_store8(void* device, mem_addr_t offset, uint8_t value) {
    DEBUG_CODE(logger::push("8 bit storing into nodevice", logger::type_t::warning));
}