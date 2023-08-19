#pragma once

#include "defs.h"

#define FETCH_FN(type_name)\
    template <class device_t, class type_t, class true_device_t = std::remove_cv_t<device_t>>\
    std::enable_if_t<std::is_same_v<true_device_t, type_name> && std::is_integral_v<type_t>, type_t>

#define STORE_FN(type_name)\
    template <class device_t, class type_t, class true_device_t = std::remove_cv_t<device_t>>\
    std::enable_if_t<std::is_same_v<true_device_t, type_name> && std::is_integral_v<type_t>, void>

namespace ps1 {
    struct mem_range_t {
        mem_range_t() = default;
        mem_range_t(mem_addr_t, mem_size_t);

        mem_addr_t offset(mem_addr_t) const;
        bool contains(mem_addr_t) const;

        mem_addr_t start;
        mem_size_t size;
    };

    mem_addr_t mask_addr(mem_addr_t);
}
