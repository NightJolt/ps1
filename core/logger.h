#pragma once

#include "defs.h"

namespace ps1::logger {
    void push(const str_t&, const str_t& = "all");
    
    void display();
}