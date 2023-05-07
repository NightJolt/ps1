#pragma once

#include "defs.h"
#include "peripheral.h"

namespace ps1 {
    struct ram_t {
        uint8_t* data;
    };

    void ram_init(ram_t*);
    void ram_del(ram_t*);

    template <class device, class type, class true_device = std::remove_cv_t<device>>
    std::enable_if_t<std::is_same_v<true_device, ram_t> && std::is_integral_v<type>, type>
    fetch(void* ram, mem_addr_t offset) {
        return *(type*)(((device*)ram)->data + offset);
    }

    template <class device, class type, class true_device = std::remove_cv_t<device>>
    std::enable_if_t<std::is_same_v<true_device, ram_t> && std::is_integral_v<type>, void>
    store(void* ram, mem_addr_t offset, type value) {
        *(type*)(((device*)ram)->data + offset) = value;
    }
}