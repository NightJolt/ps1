#pragma once

#include "defs.h"
#include "peripheral.h"

namespace ps1 {
    struct bios_t {
        uint8_t* data;
    };

    void bios_init(bios_t*, const str_t&);
    void bios_del(bios_t*);

    template <class device, class type, class true_device = std::remove_cv_t<device>>
    std::enable_if_t<std::is_same_v<true_device, bios_t> && std::is_integral_v<type>, type>
    fetch(void* bios, mem_addr_t offset) {
        return *(type*)(((device*)bios)->data + offset);
    }
}