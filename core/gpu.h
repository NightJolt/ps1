#pragma once

#include "defs.h"
#include "peripheral.h"
#include "logger.h"

namespace ps1 {
    union gpu_stat_t {
        enum struct texture_depth_t : uint32_t {
            bit_4 = 1,
            bit_8 = 2,
            bit_15 = 3,
        };

        enum struct dma_dir_t : uint32_t {
            off = 0,
            fifo = 1,
            cpu_to_gp0 = 2,
            vram_to_cpu = 3,
        };

        struct {
            uint32_t texture_page_x_base : 4;
            uint32_t texture_page_y_base : 1;
            uint32_t semi_transparency : 2;
            texture_depth_t texture_depth : 2;
            uint32_t dither : 1;
            uint32_t draw_to_display : 1;
            uint32_t set_mask_bit_on_draw : 1;
            uint32_t preserve_masked_pixels : 1;
            uint32_t interlance_field : 1;
            uint32_t reverse_flag : 1;
            uint32_t texture_page_y_base_2 : 1;
            uint32_t horizontal_resolution_2 : 1;
            uint32_t horizontal_resolution_1 : 2;
            uint32_t vertical_resolution : 1;
            uint32_t video_mode : 1;
            uint32_t display_area_color_depth : 1;
            uint32_t vertical_interlace : 1;
            uint32_t display_disable : 1;
            uint32_t interrupt_request : 1;
            uint32_t dma_data_request : 1;
            uint32_t ready_to_receive_cmd : 1;
            uint32_t ready_to_send_vram_to_cpu : 1;
            uint32_t ready_to_recieve_dma_block : 1;
            dma_dir_t dma_dir : 2;
            uint32_t draw_mode : 1;
        };

        uint32_t raw;

        uint32_t get() {
            dma_data_request =
                dma_dir == dma_dir_t::off ? 0 :
                dma_dir == dma_dir_t::fifo ? 1 :
                dma_dir == dma_dir_t::cpu_to_gp0 ? ready_to_recieve_dma_block :
                dma_dir == dma_dir_t::vram_to_cpu ? ready_to_send_vram_to_cpu : 0;

            // ! temp hack to avoid loophole
            vertical_resolution = 0;

            // ! temp override. needs proper timing to be emulated
            ready_to_receive_cmd = 1;
            ready_to_send_vram_to_cpu = 1;
            ready_to_recieve_dma_block = 1;

            return raw;
        }
    };

    struct gpu_cmd_buffer_t {
        void push(uint32_t value) {
            buffer[size++] = value;
        }

        uint32_t buffer[16];
        uint32_t size;
    };

    typedef void (*gp0_fn_t)(gpu_t*);
    
    struct gp0_fn_info_t {
        gp0_fn_t fn;
        uint32_t args_left;
    };

    enum struct gp0_data_mode_t : uint32_t {
        command,
        texture,
    };

    struct gpu_t {
        gpu_stat_t stat;

        bool rect_texture_x_flip;
        bool rect_texture_y_flip;
        uint32_t texture_window_x_mask;
        uint32_t texture_window_y_mask;
        uint32_t texture_window_x_offset;
        uint32_t texture_window_y_offset;
        uint32_t drawing_area_left;
        uint32_t drawing_area_top;
        uint32_t drawing_area_right;
        uint32_t drawing_area_bottom;
        int16_t drawing_offset_x;
        int16_t drawing_offset_y;
        uint32_t display_vram_x_start;
        uint32_t display_vram_y_start;
        uint32_t display_horiz_start;
        uint32_t display_horiz_end;
        uint32_t display_line_start;
        uint32_t display_line_end;

        gpu_cmd_buffer_t gp0_cmd_buffer;
        gp0_fn_info_t gp0_fn_info;
        uint32_t gp0_cmd_opcode; // * used for state recovery
        gp0_data_mode_t gp0_data_mode;
    };

    void gpu_init(gpu_t*);
    void gpu_exit(gpu_t*);
    
    void gpu_save_state(gpu_t*);
    void gpu_load_state(gpu_t*);

    void gp0(gpu_t*, uint32_t);
    void gp1(gpu_t*, uint32_t);

    FETCH_FN(gpu_t) fetch(void* device, mem_addr_t offset) {
        gpu_t* gpu = (gpu_t*)device;

        if (offset == 0) {
            return 0;
        } else if (offset == 4) {
            return gpu->stat.get();
        }
        
        ASSERT(false, "unhandled gpu fetch");

        return 0;
    }

    STORE_FN(gpu_t) store(void* device, mem_addr_t offset, type_t value) {
        gpu_t* gpu = (gpu_t*)device;

        if (offset == 0) {
            gp0(gpu, value);
        } else if (offset == 4) {
            gp1(gpu, value);
        } else {
            ASSERT(false, "unhandled gpu store");
        }
    }
}