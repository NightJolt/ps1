#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <optional>
#include <unordered_map>
#include <functional>
#include <set>
// #include <cstdio>

#if defined(PS1_WINDOWS)
#include <glew/glew.h>
#include <glfw/glfw3.h>
#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>
#elif defined(PS1_LINUX)
#include <GL/glew.h>
#include <GL/gl.h>
#include <GLFW/glfw3.h>
#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>
#endif


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

#if defined(PS1_DEBUG)
    #define DEBUG_CODE(code)\
                code;
#else
    #define DEBUG_CODE(code) do { } while (false)
#endif


typedef std::string str_t;

template <class T>
using dyn_arr_t = std::vector <T>;

template <class T>
using optional_t = std::optional <T>;

template <class T1, class T2>
using pair_t = std::pair <T1, T2>;

template <class T1, class T2>
using umap_t = std::unordered_map <T1, T2>;

template <class T>
using func_t = std::function <T>;

template <class T>  
using set_t = std::set <T>;


namespace ps1 {
    /*
    * KUSEG : cached / queued write
    * KSEG0 : cached / queued write
    * KSEG1 : uncached / non queued write
    * KSEG2 : cached / non queued write
    */
    
    constexpr uint32_t KUSEG_MASK = 0xFFFFFFFF;
    constexpr uint32_t KSEG0_MASK = 0x7FFFFFFF;
    constexpr uint32_t KSEG1_MASK = 0x1FFFFFFF;
    constexpr uint32_t KSEG2_MASK = 0xFFFFFFFF;

    constexpr uint32_t RAM_ADDR = 0x00000000;
    constexpr uint32_t RAM_KUSEG = 0x00000000;
    constexpr uint32_t RAM_KSEG0 = 0x80000000;
    constexpr uint32_t RAM_KSEG1 = 0xA0000000;
    constexpr uint32_t RAM_SIZE = 2 * 1024 * 1024;

    constexpr uint32_t EXPANSION1_ADDR = 0x1F000000;
    constexpr uint32_t EXPANSION1_KUSEG = 0x1F000000;
    constexpr uint32_t EXPANSION1_SIZE = 8 * 1024 * 1024;
    
    // constexpr uint32_t EXPANSION2_ADDR = 0x1F802000;
    constexpr uint32_t EXPANSION2_KUSEG = 0x1F802000;

    constexpr uint32_t SCRATCHPAD_ADDR = 0x1F800000;
    constexpr uint32_t SCRATCHPAD_KUSEG = 0x1F800000;
    constexpr uint32_t SCRATCHPAD_KSEG0 = 0x9F800000;
    constexpr uint32_t SCRATCHPAD_KSEG1 = 0xBF800000;
    constexpr uint32_t SCRATCHPAD_SIZE = 1024;

    constexpr uint32_t HARDREG_ADDR = 0x1F801000;
    constexpr uint32_t HARDREG_KUSEG = 0x1F801000;
    constexpr uint32_t HARDREG_KSEG0 = 0x9F801000;
    constexpr uint32_t HARDREG_KSEG1 = 0xBF801000;
    constexpr uint32_t HARDREG_SIZE = 8 * 1024;

    constexpr uint32_t BIOS_ADDR = 0x1FC00000;
    constexpr uint32_t BIOS_KUSEG = 0x1FC00000;
    constexpr uint32_t BIOS_KSEG0 = 0x9FC00000;
    constexpr uint32_t BIOS_KSEG1 = 0xBFC00000;
    constexpr uint32_t BIOS_SIZE = 512 * 1024;
    constexpr uint32_t BIOS_ENTRY = 0xBFC00000;

    // constexpr uint32_t ICPUCR_ADDR = 0xFFFE0000;
    constexpr uint32_t ICPUCR_KSEG2 = 0xFFFE0000;
    constexpr uint32_t ICPUCR_SIZE = 512;
}


namespace ps1 {
    typedef uint32_t cpu_reg_t;
    typedef uint32_t mem_addr_t;
    typedef uint32_t mem_size_t;

    union cpu_instr_t;

    struct bus_t;
    struct cpu_t;
    struct bios_t;
    struct ram_t;
    struct dma_t;
    struct gpu_t;
    struct vram_t;

    struct ps1_t;
    struct emulation_settings_t;
}
