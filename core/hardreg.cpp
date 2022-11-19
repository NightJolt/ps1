#include "hardreg.h"

uint32_t ps1::hardreg_t::fetch32(mem_addr_t offset) const {
    ASSERT(false, "Nothing to fetch");

    return 0;
}

void ps1::hardreg_t::store32(mem_addr_t offset, uint32_t value) {
    ASSERT(offset != 0 || value == EXPANSION1_KUSEG, "Can not remap explansion 1");
    ASSERT(offset != 4 || value == EXPANSION2_KUSEG, "Can not remap explansion 2");
}