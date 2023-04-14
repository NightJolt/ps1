#include "peripheral.h"

ps1::mem_range_t::mem_range_t(mem_addr_t start, mem_size_t size) : start(start), size(size) {}

ps1::mem_addr_t ps1::mem_range_t::offset(mem_addr_t mem_addr) const {
    return mem_addr - start;
}

bool ps1::mem_range_t::contains(mem_addr_t mem_addr) const {
    auto o = offset(mem_addr);

    return o < size && o >= 0;
}
