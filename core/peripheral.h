#pragma once

#include "defs.h"

namespace ps1 {
    struct mem_range_t {
        mem_range_t() = default;
        mem_range_t(mem_addr_t, mem_addr_t);

        mem_addr_t offset(mem_addr_t) const;
        bool contains(mem_addr_t) const;

        ps1::mem_addr_t start;
        ps1::mem_addr_t size;
    };

    struct peripheral_i {
        virtual ~peripheral_i() = 0;

        virtual uint32_t fetch32(mem_addr_t) const = 0;
        virtual void store32(mem_addr_t, uint32_t) = 0;
    };
}
