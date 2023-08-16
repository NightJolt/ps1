#include "peripheral.h"

ps1::mem_range_t::mem_range_t(mem_addr_t start, mem_size_t size) : start(start), size(size) {}

ps1::mem_addr_t ps1::mem_range_t::offset(mem_addr_t mem_addr) const {
    return mem_addr - start;
}

bool ps1::mem_range_t::contains(mem_addr_t mem_addr) const {
    auto o = offset(mem_addr);

    return o < size && o >= 0;
}

namespace ps1 {
    static constexpr mem_addr_t mem_masking_table[] = {
        KUSEG_MASK, KUSEG_MASK, KUSEG_MASK, KUSEG_MASK,
        KSEG0_MASK,
        KSEG1_MASK,
        KSEG2_MASK, KSEG2_MASK
    };
}

ps1::mem_addr_t ps1::mask_addr(mem_addr_t addr) {
    return addr & mem_masking_table[addr >> 29];
}