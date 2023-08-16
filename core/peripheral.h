#pragma once

#include "defs.h"

#define FETCH_FN(type_name)\
    template <class device, class type, class true_device = std::remove_cv_t<device>>\
    std::enable_if_t<std::is_same_v<true_device, type_name> && std::is_integral_v<type>, type>

#define STORE_FN(type_name)\
    template <class device, class type, class true_device = std::remove_cv_t<device>>\
    std::enable_if_t<std::is_same_v<true_device, type_name> && std::is_integral_v<type>, void>

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
