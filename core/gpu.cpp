#include "gpu.h"
#include "file.h"

void ps1::gpu_init(gpu_t* gpu) {
    gpu->stat.raw = 0;
    gpu->stat.display_disable = 1;

    gpu->gp0_fn_info.args_left = 0;
    gpu->gp0_cmd_buffer.size = 0;
    gpu->gp0_data_mode = gp0_data_mode_t::command;
}

void ps1::gpu_exit(gpu_t* gpu) {}

namespace {
    uint32_t sign_extend_11(uint32_t value) {
        return ((int16_t)(value << 5)) >> 5;
    }

    struct pos_t {
        pos_t(uint32_t v) : x((int16_t)v), y((int16_t)(v >> 16)) {}

        int32_t x;
        int32_t y;
    };

    struct rgb_t {
        rgb_t(uint32_t v) :  r((int8_t)v), g((int8_t)(v >> 8)), b((int8_t)(v >> 16)) {}

        int8_t r;
        int8_t g;
        int8_t b;
    };
}

namespace ps1 {
    void gp0_nop(gpu_t* gpu) {}

    void gp0_clear_cache(gpu_t* gpu) {
        logger::push("GP0: clearing cache", logger::type_t::message, "gpu");
    }

    void gp0_quad_mono_opaque(gpu_t* gpu) {
        logger::push("GP0: drawing quad", logger::type_t::message, "gpu");
    }

    void gp0_quad_blend_opaque_textured(gpu_t* gpu) {
        logger::push("GP0: drawing textured quad", logger::type_t::message, "gpu");
    }

    void gp0_triangle_shaded_opaque(gpu_t* gpu) {
        logger::push("GP0: drawing shaded triangle", logger::type_t::message, "gpu");

        pos_t vertice_pos[] = {
            gpu->gp0_cmd_buffer.buffer[1],
            gpu->gp0_cmd_buffer.buffer[3],
            gpu->gp0_cmd_buffer.buffer[5],
        };

        rgb_t vertice_rgb[] = {
            gpu->gp0_cmd_buffer.buffer[0],
            gpu->gp0_cmd_buffer.buffer[2],
            gpu->gp0_cmd_buffer.buffer[4],
        };
    }

    void gp0_quad_shaded_opaque(gpu_t* gpu) {
        logger::push("GP0: drawing shaded quad", logger::type_t::message, "gpu");
    }

    void gp0_load_texture(gpu_t* gpu) {
        uint32_t resolution = gpu->gp0_cmd_buffer.buffer[2];
        uint32_t width = resolution & 0xffff;
        uint32_t height = resolution >> 16;
        uint32_t texture_size = width * height;

        texture_size += texture_size & 0x1; // * round up to be even
        
        gpu->gp0_fn_info.args_left = texture_size >> 1;
        gpu->gp0_data_mode = gp0_data_mode_t::texture;
    }

    void gp0_store_texture(gpu_t* gpu) {
        uint32_t resolution = gpu->gp0_cmd_buffer.buffer[2];
        uint32_t width = resolution & 0xffff;
        uint32_t height = resolution >> 16;
        
        logger::push("GP0: storing image into ram", logger::type_t::message, "gpu");
    }

    void gp0_set_draw_mode(gpu_t* gpu) {
        uint32_t value = gpu->gp0_cmd_buffer.buffer[0];

        gpu->stat.texture_page_x_base = value & 0xf;
        gpu->stat.texture_page_y_base = (value >> 4) & 0x1;
        gpu->stat.semi_transparency = (value >> 5) & 0x3;
        gpu->stat.texture_depth = (gpu_stat_t::texture_depth_t)((value >> 7) & 0x3);
        gpu->stat.dither = (value >> 9) & 0x1;
        gpu->stat.draw_to_display = (value >> 10) & 0x1;
        gpu->stat.texture_page_y_base_2 = (value >> 11) & 0x1; // ? disable texture
        gpu->rect_texture_x_flip = (value >> 12) & 0x1;
        gpu->rect_texture_y_flip = (value >> 13) & 0x1;
    }

    void gp0_config_texture_window(gpu_t* gpu) {
        uint32_t value = gpu->gp0_cmd_buffer.buffer[0];

        gpu->texture_window_x_mask = value & 0x1f;
        gpu->texture_window_y_mask = (value >> 5) & 0x1f;
        gpu->texture_window_x_offset = (value >> 10) & 0x1f;
        gpu->texture_window_y_offset = (value >> 15) & 0x1f;
    }
    
    void gp0_set_draw_area_top_left(gpu_t* gpu) {
        uint32_t value = gpu->gp0_cmd_buffer.buffer[0];

        gpu->drawing_area_left = value & 0x3ff;
        gpu->drawing_area_top = (value >> 10) & 0x3ff;
    }
    
    void gp0_set_draw_area_bottom_right(gpu_t* gpu) {
        uint32_t value = gpu->gp0_cmd_buffer.buffer[0];

        gpu->drawing_area_right = value & 0x3ff;
        gpu->drawing_area_top = (value >> 10) & 0x3ff;
    }
    
    void gp0_set_drawing_offset(gpu_t* gpu) {
        uint32_t value = gpu->gp0_cmd_buffer.buffer[0];

        gpu->drawing_offset_x = sign_extend_11(value & 0x7ff);
        gpu->drawing_offset_y = sign_extend_11((value >> 11) & 0x7ff);
    }

    void gp0_set_mask_bits(gpu_t* gpu) {
        uint32_t value = gpu->gp0_cmd_buffer.buffer[0];

        gpu->stat.set_mask_bit_on_draw = (value >> 0) & 0x1;
        gpu->stat.preserve_masked_pixels = (value >> 1) & 0x1;
    }
}

namespace ps1 {
    umap_t<uint32_t, gp0_fn_info_t> gp0_fns = {
        { 0x00, { gp0_nop, 1 } },
        { 0x01, { gp0_clear_cache, 1 } },
        { 0x28, { gp0_quad_mono_opaque, 5 } },
        { 0x2C, { gp0_quad_blend_opaque_textured, 9 } },
        { 0x30, { gp0_triangle_shaded_opaque, 6 } },
        { 0x38, { gp0_quad_shaded_opaque, 8 } },
        { 0xA0, { gp0_load_texture, 3 } },
        { 0xC0, { gp0_store_texture, 3 } },
        { 0xE1, { gp0_set_draw_mode, 1 } },
        { 0xE2, { gp0_config_texture_window, 1 } },
        { 0xE3, { gp0_set_draw_area_top_left, 1 } },
        { 0xE4, { gp0_set_draw_area_bottom_right, 1 } },
        { 0xE5, { gp0_set_drawing_offset, 1 } },
        { 0xE6, { gp0_set_mask_bits, 1 } },
    };

    gp0_fn_info_t get_gp0_fn_info(uint32_t opcode) {
        auto it = gp0_fns.find(opcode);

        if (it == gp0_fns.end()) {
            return { nullptr, 0 };
        }

        return it->second;
    }
}

void ps1::gp0(gpu_t* gpu, uint32_t value) {
    if (gpu->gp0_fn_info.args_left == 0) {
        gpu->gp0_cmd_opcode = value >> 24;

        gp0_fn_info_t info = get_gp0_fn_info(gpu->gp0_cmd_opcode);

        ASSERT(info.fn, "ILLEGAL GP0 OPCODE");

        gpu->gp0_fn_info = info;
        gpu->gp0_cmd_buffer.size = 0;
    }

    gpu->gp0_fn_info.args_left--;

    if (gpu->gp0_data_mode == gp0_data_mode_t::command) {
        gpu->gp0_cmd_buffer.push(value);

        if (gpu->gp0_fn_info.args_left == 0) {
            gpu->gp0_fn_info.fn(gpu);
        }
    } else if (gpu->gp0_data_mode == gp0_data_mode_t::texture) {
        // ! copy data to vram

        if (gpu->gp0_fn_info.args_left == 0) {
            gpu->gp0_data_mode = gp0_data_mode_t::command;
        }
    } else {
        ASSERT(false, "ILLEGAL GP0 DATA MODE");
    }
}

namespace ps1 {
    void gp1_clear_fifo(gpu_t* gpu) {
        gpu->gp0_cmd_buffer.size = 0;
        gpu->gp0_fn_info.args_left = 0;
        gpu->gp0_data_mode = gp0_data_mode_t::command;
    }

    void gp1_reset(gpu_t* gpu) {
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
        gp1_clear_fifo(gpu);
    }

    void gp1_acknowledge_irq(gpu_t* gpu) {
        gpu->stat.interrupt_request = 0;
    }

    void gp1_set_display_disabled(gpu_t* gpu, uint32_t value) {
        gpu->stat.display_disable = value & 0x1;
    }

    void gp1_set_display_mode(gpu_t* gpu, uint32_t value) {
        gpu->stat.horizontal_resolution_1 = (value >> 0) & 0x3;
        gpu->stat.vertical_resolution = (value >> 2) & 0x1;
        gpu->stat.video_mode = (value >> 3) & 0x1;
        gpu->stat.display_area_color_depth = (value >> 4) & 0x1;
        gpu->stat.vertical_interlace = (value >> 5) & 0x1;
        gpu->stat.horizontal_resolution_2 = (value >> 6) & 0x1;
        gpu->stat.reverse_flag = (value >> 7) & 0x1; // !
    }

    void gp1_set_display_addr_in_vram(gpu_t* gpu, uint32_t value) {
        gpu->display_vram_x_start = value & 0x3fe; // * 2 LSB ignored to align with pixels
        gpu->display_vram_y_start = (value >> 10) & 0x1ff;
    }

    void gp1_set_display_horizontal_range(gpu_t* gpu, uint32_t value) {
        gpu->display_horiz_start = value & 0xfff;
        gpu->display_horiz_end = (value >> 12) & 0xfff;
    }

    void gp1_set_display_vertical_range(gpu_t* gpu, uint32_t value) {
        gpu->display_line_start = value & 0x3ff;
        gpu->display_line_end = (value >> 10) & 0x3ff;
    }

    void gp1_set_dma_direction(gpu_t* gpu, uint32_t value) {
        gpu->stat.dma_dir = gpu_stat_t::dma_dir_t { value & 0x3 };
    }
}

void ps1::gp1(gpu_t* gpu, uint32_t value) {
    uint32_t opcode = value >> 24;

    switch(opcode) {
        case 0x00: {
            gp1_reset(gpu);

            break;
        }

        case 0x01: {
            gp1_clear_fifo(gpu);

            break;
        }

        case 0x02: {
            gp1_acknowledge_irq(gpu);

            break;
        }

        case 0x03: {
            gp1_set_display_disabled(gpu, value);

            break;
        }

        case 0x04: {
            gp1_set_dma_direction(gpu, value);

            break;
        }

        case 0x05: {
            gp1_set_display_addr_in_vram(gpu, value);

            break;
        }

        case 0x06: {
            gp1_set_display_horizontal_range(gpu, value);

            break;
        }

        case 0x07: {
            gp1_set_display_vertical_range(gpu, value);

            break;
        }

        case 0x08: {
            gp1_set_display_mode(gpu, value);

            break;
        }

        default: {
            ASSERT(false, "ILLEGAL GP1 OPCODE");

            break;
        }
    }
}

void ps1::gpu_save_state(gpu_t* gpu) {
    file::write32(gpu->stat.raw);

    file::write32(gpu->rect_texture_x_flip);
    file::write32(gpu->rect_texture_y_flip);
    file::write32(gpu->texture_window_x_mask);
    file::write32(gpu->texture_window_y_mask);
    file::write32(gpu->texture_window_x_offset);
    file::write32(gpu->texture_window_y_offset);
    file::write32(gpu->drawing_area_left);
    file::write32(gpu->drawing_area_top);
    file::write32(gpu->drawing_area_right);
    file::write32(gpu->drawing_area_bottom);
    file::write32(gpu->drawing_offset_x);
    file::write32(gpu->drawing_offset_y);
    file::write32(gpu->display_vram_x_start);
    file::write32(gpu->display_vram_y_start);
    file::write32(gpu->display_horiz_start);
    file::write32(gpu->display_horiz_end);
    file::write32(gpu->display_line_start);
    file::write32(gpu->display_line_end);

    file::write((uint8_t*)gpu->gp0_cmd_buffer.buffer, sizeof(gpu->gp0_cmd_buffer.buffer));
    file::write32(gpu->gp0_cmd_buffer.size);
    file::write32(gpu->gp0_fn_info.args_left);
    file::write32(gpu->gp0_cmd_opcode);
    file::write32((uint32_t)gpu->gp0_data_mode);
}

void ps1::gpu_load_state(gpu_t* gpu) {
    gpu->stat.raw = file::read32();
    
    gpu->rect_texture_x_flip = file::read32();
    gpu->rect_texture_y_flip = file::read32();
    gpu->texture_window_x_mask = file::read32();
    gpu->texture_window_y_mask = file::read32();
    gpu->texture_window_x_offset = file::read32();
    gpu->texture_window_y_offset = file::read32();
    gpu->drawing_area_left = file::read32();
    gpu->drawing_area_top = file::read32();
    gpu->drawing_area_right = file::read32();
    gpu->drawing_area_bottom = file::read32();
    gpu->drawing_offset_x = file::read32();
    gpu->drawing_offset_y = file::read32();
    gpu->display_vram_x_start = file::read32();
    gpu->display_vram_y_start = file::read32();
    gpu->display_horiz_start = file::read32();
    gpu->display_horiz_end = file::read32();
    gpu->display_line_start = file::read32();
    gpu->display_line_end = file::read32();
    
    file::read((uint8_t*)gpu->gp0_cmd_buffer.buffer, sizeof(gpu->gp0_cmd_buffer.buffer));
    gpu->gp0_cmd_buffer.size = file::read32();
    gpu->gp0_fn_info.args_left = file::read32();
    gpu->gp0_cmd_opcode = file::read32();
    gpu->gp0_data_mode = (gp0_data_mode_t)file::read32();

    if (gpu->gp0_fn_info.args_left > 0) {
        gpu->gp0_fn_info.fn = get_gp0_fn_info(gpu->gp0_cmd_opcode).fn;
    }
}