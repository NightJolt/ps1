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

                return raw;
            }
        };

    struct gpu_t {
        gpu_stat_t stat;

        bool rect_texture_x_flip;
        bool rect_texture_y_flip;

        uint8_t texture_window_x_mask;
        uint8_t texture_window_y_mask;
        uint8_t texture_window_x_offset;
        uint8_t texture_window_y_offset;
        uint16_t drawing_area_left;
        uint16_t drawing_area_top;
        uint16_t drawing_area_right;
        uint16_t drawing_area_bottom;
        int16_t drawing_offset_x;
        int16_t drawing_offset_y;
        uint16_t display_vram_x_start;
        uint16_t display_vram_y_start;
        uint16_t display_horiz_start;
        uint16_t display_horiz_end;
        uint16_t display_line_start;
        uint16_t display_line_end;
    };

    void gpu_init(gpu_t*);
    void gpu_exit(gpu_t*);

    inline void gp0_set_draw_mode(gpu_t* gpu, uint32_t value) {
        gpu_stat_t stat;
        stat.raw = value;

        gpu->stat.texture_page_x_base = stat.texture_page_x_base;
        gpu->stat.texture_page_y_base = stat.texture_page_y_base;
        gpu->stat.semi_transparency = stat.semi_transparency;
        gpu->stat.texture_depth = stat.texture_depth;
        gpu->stat.dither = stat.dither;
        gpu->stat.draw_to_display = stat.draw_to_display;
        gpu->stat.texture_page_y_base_2 = stat.texture_page_y_base_2; // ? disable texture

        gpu->rect_texture_x_flip = (value >> 12) & 0x1;
        gpu->rect_texture_y_flip = (value >> 13) & 0x1;
    }

    inline void gp1_reset(gpu_t* gpu) {
        gpu->stat.raw = 0x14802000;

        gpu->rect_texture_x_flip = false;
        gpu->rect_texture_y_flip = false;

        gpu->texture_window_x_mask = 0;
        gpu->texture_window_y_mask = 0;
        gpu->texture_window_x_offset = 0;
        gpu->texture_window_y_offset = 0;
        gpu->drawing_area_left = 0;
        gpu->drawing_area_top = 0;
        gpu->drawing_area_right = 0;
        gpu->drawing_area_bottom = 0;
        gpu->drawing_offset_x = 0;
        gpu->drawing_offset_y = 0;
        gpu->display_vram_x_start = 0;
        gpu->display_vram_y_start = 0;
        gpu->display_horiz_start = 0x200;
        gpu->display_horiz_end = 0xc00;
        gpu->display_line_start = 0x10;
        gpu->display_line_end = 0x100;

        // todo: clear fifo and invalidate cache
    }

    inline void gp0(gpu_t* gpu, uint32_t value) {
        uint32_t opcode = value >> 24;

        switch(opcode) {
            case 0xE1: {
                gp0_set_draw_mode(gpu, value);

                break;
            }

            case 0x00: {
                // * NOP. does not take up space in fifo

                break;
            }

            default: {
                ASSERT(false, "ILLEGAL GP0 OPCODE");

                break;
            }
        }
    }

    inline void gp1(gpu_t* gpu, uint32_t value) {
        uint32_t opcode = value >> 24;

        switch(opcode) {
            case 0x00: {
                gp1_reset(gpu);

                break;
            }

            default: {
                ASSERT(false, "ILLEGAL GP1 OPCODE");

                break;
            }
        }
    }

    FETCH_FN(gpu_t) fetch(void* device, mem_addr_t offset) {
        DEBUG_CODE(logger::push("fetching", logger::type_t::warning, "gpu"));

        gpu_t* gpu = (gpu_t*)device;

        return offset == 4 ? gpu->stat.get() : 0;
    }

    STORE_FN(gpu_t) store(void* device, mem_addr_t offset, type_t value) {
        DEBUG_CODE(logger::push("storing", logger::type_t::warning, "gpu"));

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