#pragma once

#include "defs.h"
#include "peripheral.h"
#include "logger.h"

namespace ps1 {
    struct nodevice_t {};

    template <class device, class type, class true_device = std::remove_cv_t<device>>
    std::enable_if_t<std::is_same_v<true_device, nodevice_t> && std::is_integral_v<type>, type>
    fetch(void* nodevice, mem_addr_t offset) {
        static str_t len = std::to_string(sizeof(type) * 8);
        static str_t msg = len + " bit fetching from nodevice";

        DEBUG_CODE(logger::push(msg, logger::type_t::error, "nodevice"));

        return 0;
    }

    template <class device, class type, class true_device = std::remove_cv_t<device>>
    std::enable_if_t<std::is_same_v<true_device, nodevice_t> && std::is_integral_v<type>, void>
    store(void* nodevice, mem_addr_t offset, type value) {
        static str_t len = std::to_string(sizeof(type) * 8);
        static str_t msg = len + " bit storing into nodevice";

        DEBUG_CODE(logger::push(msg, logger::type_t::warning, "nodevice"));
    }
}