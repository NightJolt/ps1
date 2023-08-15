#pragma once

#include "defs.h"

namespace ps1::logger {
    enum struct type_t {
        message,
        info,
        warning,
        error
    };

    void push(const str_t&, type_t type, const str_t& = "all");
    void spam(const str_t&);
    
    void display();
}