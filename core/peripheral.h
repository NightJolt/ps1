#pragma once

#include "defs.h"

namespace ps1 {
    struct mem_range_t {
        mem_range_t() = default;
        mem_range_t(mem_addr_t, mem_size_t);

        mem_addr_t offset(mem_addr_t) const;
        bool contains(mem_addr_t) const;

        mem_addr_t start;
        mem_size_t size;
    };
}
