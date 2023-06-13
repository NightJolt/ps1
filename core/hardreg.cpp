#include "hardreg.h"
#include "logger.h"

uint32_t ps1::hardreg_fetch32(void* device, mem_addr_t offset) {
    ASSERT(false, "Nothing to fetch");

    return 0;
}

void ps1::hardreg_store32(void* device, mem_addr_t offset, uint32_t value) {
    ASSERT(offset != 0 || value == EXPANSION1_KUSEG, "Can not remap explansion 1");
    ASSERT(offset != 4 || value == EXPANSION2_KUSEG, "Can not remap explansion 2");

    logger::push("storing into hardreg", logger::type_t::warning, "hardreg");
}