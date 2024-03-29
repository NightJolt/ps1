#pragma once

#include "defs.h"
#include "peripheral.h"
#include "logger.h"

namespace ps1 {
    struct nodevice_t {};

    FETCH_FN(nodevice_t) fetch(void* nodevice, mem_addr_t offset) {
        static str_t len = std::to_string(sizeof(type_t) * 8);
        static str_t msg = len + " bit fetching from nodevice";

        DEBUG_CODE(logger::push(msg, logger::type_t::error, "nodevice"));

        return 0;
    }

    STORE_FN(nodevice_t) store(void* nodevice, mem_addr_t offset, type_t value) {
        static str_t len = std::to_string(sizeof(type_t) * 8);
        static str_t msg = len + " bit storing into nodevice";

        DEBUG_CODE(logger::push(msg, logger::type_t::warning, "nodevice"));
    }
}