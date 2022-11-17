#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <optional>

#if defined(PS1_DEBUG)
    #include <iostream>
#endif

#if defined(PS1_DEBUG)
    #define ASSERT(condition, message)\
                do {\
                    if (!(condition)) {\
                        std::cerr << "Assertion failed: \033[1;31m" << message << "\033[0m (" << __FILE__ << "(" << __LINE__ << "))" << std::endl;\
                        std::terminate();\
                    }\
                } while (false)
#else
    #define ASSERT(condition, message) do { } while (false)
#endif


typedef std::string str_t;

template <class T>
using dyn_arr_t = std::vector <T>;

template <class T>
using optional_t = std::optional <T>;

template <class T1, class T2>
using pair_t = std::pair <T1, T2>;


namespace ps1 {
    typedef uint32_t cpu_reg_t;
    typedef uint32_t cpu_instr_t;
    typedef uint32_t mem_addr_t;

    struct bus_t;
    struct cpu_t;
    struct bios_t;
}