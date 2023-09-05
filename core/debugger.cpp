#include "debugger.h"
#include "bus.h"
#include "ps1.h"
#include "emulation.h"
#include "logger.h"

namespace {
    str_t byte_to_bin(uint8_t val) {
        return
            str_t(val & 0x80 ? "1" : "0")
            + (val & 0x40 ? "1" : "0")
            + (val & 0x20 ? "1" : "0")
            + (val & 0x10 ? "1" : "0")
            + (val & 0x08 ? "1" : "0")
            + (val & 0x04 ? "1" : "0")
            + (val & 0x02 ? "1" : "0")
            + (val & 0x01 ? "1" : "0");

    }

    str_t to_bin(uint32_t val) {
        return "0b" + byte_to_bin(val >> 24) + byte_to_bin(val >> 16) + byte_to_bin(val >> 8) + byte_to_bin(val);
    }

    const uint32_t COL_GREEN { IM_COL32(100, 255, 59, 70) };
    const uint32_t COL_YELLOW { IM_COL32(255, 235, 59, 70) };

    constexpr uint32_t scroll_step = 4;
    constexpr uint32_t scroll_max = 100;
}

namespace ps1 {
    void display_emulation_view(ps1_t* console, emulation_settings_t* settings) {
        static char save_state_path[128] = "saves/state.bin";
        static char load_state_path[128] = "saves/state.bin";

        ImGui::Begin("Emulation");

            if (ImGui::Button("Soft Reset")) {
                ps1_soft_reset(console);
            }

            if (ImGui::Button("Save State")) {
                ps1_save_state(console, save_state_path);
            }

            ImGui::SameLine();

            ImGui::SetNextItemWidth(-1);
            ImGui::InputText("##save_state_path", save_state_path, sizeof(save_state_path));

            if (ImGui::Button("Load State")) {
                ps1_load_state(console, load_state_path);
            }

            ImGui::SameLine();

            ImGui::SetNextItemWidth(-1);
            ImGui::InputText("##load_state_path", load_state_path, sizeof(load_state_path));

            ImGui::Spacing();
            ImGui::AlignTextToFramePadding();
            ImGui::Text("Instructions Per Frame");
            ImGui::SameLine();
            if (ImGui::InputInt("##instr_per_frame", &settings->instr_per_frame)) {
                settings->instr_per_frame = std::min(std::max(settings->instr_per_frame, 0), 30000);
            }

        ImGui::End();
    }

    void display_instr(cpu_t* cpu, mem_addr_t addr, uint32_t instr) {
        static char addr_buffer[12];
        static char instr_buffer[12];

        sprintf(addr_buffer, "0x%08X", addr);
        sprintf(instr_buffer, "0x%08X", instr);

        if (cpu->cpc == addr) {
            ImGui::PushStyleColor(ImGuiCol_TableRowBg, COL_GREEN);
            ImGui::PushStyleColor(ImGuiCol_TableRowBgAlt, COL_GREEN);
        }

        if (cpu->pc == addr) {
            ImGui::PushStyleColor(ImGuiCol_TableRowBg, COL_YELLOW);
            ImGui::PushStyleColor(ImGuiCol_TableRowBgAlt, COL_YELLOW);
        }

        ImGui::TableNextColumn();
        ImGui::TextWrapped(addr_buffer);
        
        ImGui::TableNextColumn();
        ImGui::TextWrapped(instr_buffer);
        
        ImGui::TableNextColumn();
        ImGui::TextWrapped(to_bin(instr).c_str());
        
        ImGui::TableNextRow();

        if (cpu->cpc == addr) {
            ImGui::PopStyleColor();
            ImGui::PopStyleColor();
        }

        if (cpu->pc == addr) {
            ImGui::PopStyleColor();
            ImGui::PopStyleColor();
        }
    }

    void display_reg(const str_t& reg_name, uint32_t reg_value) {
        static char hex_buffer[12];
        static char dec_buffer[12];

        sprintf(hex_buffer, "0x%08X", reg_value);
        sprintf(dec_buffer, "%u", reg_value);

        ImGui::TableNextColumn();
        ImGui::TextWrapped(reg_name.c_str());

        ImGui::TableNextColumn();
        ImGui::TextWrapped(hex_buffer);
        
        ImGui::TableNextColumn();
        ImGui::TextWrapped(dec_buffer);
    }

    void display_memory(uint32_t addr, uint32_t value, bool highlight) {
        static char addr_buffer[12];
        static char hex_buffer[12];
        static char dec_buffer[12];

        sprintf(addr_buffer, "0x%08X", addr);
        sprintf(hex_buffer, "0x%08X", value);
        sprintf(dec_buffer, "%u", value);

        if (highlight) {
            ImGui::PushStyleColor(ImGuiCol_TableRowBg, COL_YELLOW);
            ImGui::PushStyleColor(ImGuiCol_TableRowBgAlt, COL_YELLOW);
        }

        ImGui::TableNextColumn();
        ImGui::TextWrapped(addr_buffer);

        ImGui::TableNextColumn();
        ImGui::TextWrapped(hex_buffer);
        
        ImGui::TableNextColumn();
        ImGui::TextWrapped(dec_buffer);
        
        ImGui::TableNextRow();

        if (highlight) {
            ImGui::PopStyleColor();
            ImGui::PopStyleColor();
        }
    }
    
    str_t describe_reg(uint32_t i) {
        if (i == 0) return "($zero)";   // * always 0
        if (i == 1) return "($at)";     // * reserved for assembler
        if (i == 2) return "($v0)";     // * stores results
        if (i == 3) return "($v1)";     // * stores results
        if (i == 4) return "($a0)";     // * stores arguments
        if (i == 5) return "($a1)";     // * stores arguments
        if (i == 6) return "($a2)";     // * stores arguments
        if (i == 7) return "($a3)";     // * stores arguments
        if (i == 8) return "($t0)";     // * unsaved temporary
        if (i == 9) return "($t1)";     // * unsaved temporary
        if (i == 10) return "($t2)";    // * unsaved temporary
        if (i == 11) return "($t3)";    // * unsaved temporary
        if (i == 12) return "($t4)";    // * unsaved temporary
        if (i == 13) return "($t5)";    // * unsaved temporary
        if (i == 14) return "($t6)";    // * unsaved temporary
        if (i == 15) return "($t7)";    // * unsaved temporary
        if (i == 16) return "($s0)";    // * callee saved
        if (i == 17) return "($s1)";    // * callee saved
        if (i == 18) return "($s2)";    // * callee saved
        if (i == 19) return "($s3)";    // * callee saved
        if (i == 20) return "($s4)";    // * callee saved
        if (i == 21) return "($s5)";    // * callee saved
        if (i == 22) return "($s6)";    // * callee saved
        if (i == 23) return "($s7)";    // * callee saved
        if (i == 24) return "($t8)";    // * unsaved temporary
        if (i == 25) return "($t9)";    // * unsaved temporary
        if (i == 26) return "($k0)";    // * reserved for kernel
        if (i == 27) return "($k1)";    // * reserved for kernel
        if (i == 28) return "($gp)";    // * global pointer
        if (i == 29) return "($sp)";    // * stack pointer
        if (i == 30) return "($fp)";    // * frame pointer
        if (i == 31) return "($ra)";    // * return address

        return "";
    }
}

namespace ps1 {
    uint32_t bus_fetch32_debug(bus_t* bus, mem_addr_t mem_addr) {
        ASSERT(mem_addr % 4 == 0, "Unaligned memory access");

        mem_addr = mask_addr(mem_addr);

        for (auto& device_info : bus->devices) {
            if (device_info.mem_range.contains(mem_addr)) {
                return device_info.fetch32(device_info.device, device_info.mem_range.offset(mem_addr));
            }
        }

        return 0;
    }

    void display_cpu_view(cpu_t* cpu, bus_t* bus) {
        ImGui::Begin("CPU");

            if (cpu->state == cpu_state_t::halted) {
                if (ImGui::Button("Revive")) {
                    cpu_set_state(cpu, cpu_state_t::sleeping);
                }
            } else if (cpu->state == cpu_state_t::running) {
                if (ImGui::Button("Stop")) {
                    cpu_set_state(cpu, cpu_state_t::sleeping);
                }
            } else {
                if (ImGui::Button("Run")) {
                    cpu_set_state(cpu, cpu_state_t::running);
                }

                ImGui::SameLine();

                DEBUG_CODE(
                    if (ImGui::Button("Jump")) {
                        cpu_set_state(cpu, cpu_state_t::running);
                        
                        while (true) {
                            if (cpu->state != ps1::cpu_state_t::running) break;
                            
                            ps1::cpu_tick(cpu);
                        }
                    }

                    ImGui::SameLine();
                );

                if (ImGui::Button("Step")) {
                    cpu_tick(cpu);
                }
            }

            if (ImGui::BeginTable("cpu_status", 2, ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders | ImGuiTableFlags_SizingStretchProp)) {
                ImGui::TableSetupColumn(nullptr, 0, 1);
                ImGui::TableSetupColumn(nullptr, 0, 1);
                
                ImGui::TableNextColumn();
                ImGui::TextWrapped("Status");
                
                ImGui::TableNextColumn();

                if (cpu->state == cpu_state_t::halted) {
                    ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 0, 0, 255));
                    ImGui::TextWrapped("Halted");
                    ImGui::PopStyleColor();
                } else if (cpu->state == cpu_state_t::running) {
                    ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 255, 0, 255));
                    ImGui::TextWrapped("Running");
                    ImGui::PopStyleColor();
                } else {
                    ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 0, 255, 255));
                    ImGui::TextWrapped("Sleeping");
                    ImGui::PopStyleColor();
                }

                ImGui::EndTable();
            }

            ImGui::Spacing();

            ImGui::TextWrapped(("Instructions Executed: " + std::to_string(cpu->instr_exec_cnt)).c_str());

            ImGui::Spacing();

            if (ImGui::BeginTable("delay_slot", 3, ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders | ImGuiTableFlags_SizingStretchProp)) {
                ImGui::TableSetupColumn("Delay Slot", 0, 4);
                ImGui::TableSetupColumn("Addr/Val", 0, 4);
                ImGui::TableSetupColumn("Value", 0, 13);

                ImGui::TableHeadersRow();
                
                {
                    ImGui::TableNextColumn();
                    ImGui::TextWrapped("Instruction");

                    uint32_t instr = bus_fetch32_debug(bus, cpu->pc);
                    
                    ImGui::TableNextColumn();
                    ImGui::TextWrapped("0x%08X", instr);
                    
                    ImGui::TableNextColumn();
                    ImGui::TextWrapped(to_bin(instr).c_str());
                }
                
                {
                    ImGui::TableNextColumn();
                    ImGui::TextWrapped("Value");
                    
                    ImGui::TableNextColumn();
                    ImGui::TextWrapped(("R" + std::to_string(cpu->load_delay_target)).c_str());
                    
                    ImGui::TableNextColumn();
                    ImGui::TextWrapped(std::to_string(cpu->load_delay_value).c_str());
                }
                    
                ImGui::EndTable();
            }

            ImGui::Spacing();

            ImGui::BeginChild("regs");

                if (ImGui::BeginTable("spec_regs", 3, ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders | ImGuiTableFlags_SizingStretchProp)) {
                    ImGui::TableSetupColumn(nullptr, 0, 1);
                    ImGui::TableSetupColumn(nullptr, 0, 3);
                    ImGui::TableSetupColumn(nullptr, 0, 3);
                
                    display_reg("PC", cpu->pc);
                    display_reg("HI", cpu->hi);
                    display_reg("LO", cpu->lo);
                    
                    ImGui::EndTable();
                }

                if (ImGui::BeginTable("regs", 3, ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders | ImGuiTableFlags_SizingStretchProp)) {
                    ImGui::TableSetupColumn(nullptr, 0, 2);
                    ImGui::TableSetupColumn(nullptr, 0, 3);
                    ImGui::TableSetupColumn(nullptr, 0, 3);
                    
                    for (int i = 0; i < 32; i++) {
                        display_reg("R" + std::to_string(i) + " " + describe_reg(i), cpu->in_regs[i]);
                    }
                    
                    ImGui::EndTable();
                }

                if (ImGui::BeginTable("c0regs", 3, ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders | ImGuiTableFlags_SizingStretchProp)) {
                    ImGui::TableSetupColumn(nullptr, 0, 1);
                    ImGui::TableSetupColumn(nullptr, 0, 3);
                    ImGui::TableSetupColumn(nullptr, 0, 3);
                    
                    for (int i = 0; i < 32; i++) {
                        display_reg("C0R" + std::to_string(i), cpu->c0regs[i]);
                    }
                    
                    ImGui::EndTable();
                }

            ImGui::EndChild();

        ImGui::End();
    }

    void display_instr_view(cpu_t* cpu, bus_t* bus) {
        static mem_addr_t addr_inp = BIOS_ENTRY;
        static mem_addr_t addr = BIOS_ENTRY;
        static bool follow_pc = true;
        static bool auto_update = false;
        bool update = false;

        ImGui::Begin("Instructions");

            ImGui::Checkbox("Follow PC", &follow_pc);
            ImGui::SameLine();
            ImGui::Checkbox("Auto Update", &auto_update);

            ImGui::SameLine();
            ImGui::SetNextItemWidth(-1);
            if (ImGui::InputScalar("##input_addr", ImGuiDataType_U32, &addr_inp, &scroll_step, &scroll_max, "%x", ImGuiInputTextFlags_CharsHexadecimal)) {
                if (addr_inp >= BIOS_ENTRY && addr_inp < BIOS_ENTRY + BIOS_SIZE && addr_inp % sizeof(cpu_instr_t) == 0) {
                     addr = addr_inp;
                } else addr_inp = addr;
            }

            ImGui::BeginChild("instrcution_view");

                if (ImGui::BeginTable("instruction_table", 3, ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders | ImGuiTableFlags_SizingStretchProp)) {
                    ImGui::TableSetupColumn(nullptr, 0, 4);
                    ImGui::TableSetupColumn(nullptr, 0, 4);
                    ImGui::TableSetupColumn(nullptr, 0, 13);

                    static constexpr uint32_t instr_radius = 24;

                    if (follow_pc) {
                        addr = cpu->pc;
                        update = cpu->state == cpu_state_t::running;
                    }

                    if (auto_update) {
                        update = true;
                    }
                    
                    for (mem_addr_t offset = addr - instr_radius * sizeof(cpu_instr_t); offset < addr; offset += sizeof(cpu_instr_t)) {
                        display_instr(cpu, offset, bus_fetch32_debug(bus, offset));
                    }

                    display_instr(cpu, addr, bus_fetch32_debug(bus, addr));
                    
                    for (mem_addr_t offset = addr + sizeof(cpu_instr_t); offset <= addr + instr_radius * sizeof(cpu_instr_t); offset += sizeof(cpu_instr_t)) {
                        display_instr(cpu, offset, bus_fetch32_debug(bus, offset));
                    }
                    
                    ImGui::EndTable();
                }

                if (update) {
                    ImGui::SetScrollHereY(.5f);

                    update = false;
                }

            ImGui::EndChild();

        ImGui::End();
    }

    void display_memory_view(bus_t* bus) {
        static mem_addr_t addr_inp = 0;
        static mem_addr_t addr = 0;
        bool update = false;

        ImGui::Begin("Memory");

            ImGui::SetNextItemWidth(-1);

            if (ImGui::InputScalar("##input_addr", ImGuiDataType_U32, &addr_inp, &scroll_step, &scroll_max, "%x", ImGuiInputTextFlags_CharsHexadecimal)) {
                if (addr_inp >= 0 && addr_inp < BIOS_KSEG1 + BIOS_SIZE && addr_inp % sizeof(cpu_instr_t) == 0) {
                     addr = addr_inp;
                     update = true;
                } else addr_inp = addr;
            }

            ImGui::BeginChild("memory_view");

                if (ImGui::BeginTable("memory", 3, ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders | ImGuiTableFlags_SizingStretchProp)) {
                    ImGui::TableSetupColumn(nullptr, 0, 1);
                    ImGui::TableSetupColumn(nullptr, 0, 1);
                    ImGui::TableSetupColumn(nullptr, 0, 1);

                    static uint32_t radius = sizeof(cpu_instr_t) * 32;
                    
                    for (mem_addr_t offset = addr >= radius ? addr - radius : 0; offset <= std::min(addr + radius, (BIOS_KSEG1 + BIOS_SIZE - 1)); offset += sizeof(cpu_instr_t)) {
                        display_memory(offset, bus_fetch32_debug(bus, offset), offset == addr);
                    }
                    
                    ImGui::EndTable();
                }

                if (update) {
                    ImGui::SetScrollHereY(.5f);

                    update = false;
                }

            ImGui::EndChild();

        ImGui::End();
    }

    void display_breakpoints_view(cpu_t* cpu) {
        static mem_addr_t addr_inp = 0;
        static mem_addr_t erase_value = 0;
        static bool should_erase = false;

        ImGui::Begin("Breakpoints");

            ImGui::SetNextItemWidth(160);
            ImGui::InputScalar("##input_addr", ImGuiDataType_U32, &addr_inp, nullptr, nullptr, "%x", ImGuiInputTextFlags_CharsHexadecimal);
            ImGui::SameLine();
            if (ImGui::Button("Add")) {
                cpu->breakpoints.emplace(addr_inp);
            }
            
            ImGui::Spacing();

            ImGui::BeginChild("breakpoints_view");
            
            for (mem_addr_t addr : cpu->breakpoints) {
                ImGui::AlignTextToFramePadding();
                ImGui::Text("0x%08X", addr);
                ImGui::SameLine();
                if (ImGui::Button(("Remove##" + std::to_string(addr)).c_str())) {
                    erase_value = addr;
                    should_erase = true;
                }
            }

            if (should_erase) {
                cpu->breakpoints.erase(erase_value);
                should_erase = false;
            }

            ImGui::EndChild();

        ImGui::End();
    }

    void display_gpu_view(gpu_t* gpu) {
        ImGui::Begin("GPU");

            if (ImGui::TreeNode("GPUSTAT")) {
                uint32_t data = gpu->stat.get();
                gpu_stat_t stat = *(gpu_stat_t*)&data;

                ImGui::Text("texture page x base: %u", stat.texture_page_x_base);
                ImGui::Text("texture page y base 1: %u", stat.texture_page_y_base);
                ImGui::Text("semi transparency: %u", stat.semi_transparency);
                ImGui::Text("texture depth: %u", stat.texture_depth);
                ImGui::Text("dither: %u", stat.dither);
                ImGui::Text("draw to display: %u", stat.draw_to_display);
                ImGui::Text("set mask bit on draw: %u", stat.set_mask_bit_on_draw);
                ImGui::Text("preserve masked pixels: %u", stat.preserve_masked_pixels);
                ImGui::Text("interlance field: %u", stat.interlance_field);
                ImGui::Text("reverse flag: %u", stat.reverse_flag);
                ImGui::Text("texture page y base 2: %u", stat.texture_page_y_base_2);
                ImGui::Text("horizontal resolution 2: %u", stat.horizontal_resolution_2);
                ImGui::Text("horizontal resolution 1: %u", stat.horizontal_resolution_1);
                ImGui::Text("vertical resolution: %u", stat.vertical_resolution);
                ImGui::Text("video mode: %u", stat.video_mode);
                ImGui::Text("display area color depth: %u", stat.display_area_color_depth);
                ImGui::Text("vertical interlace: %u", stat.vertical_interlace);
                ImGui::Text("display disable: %u", stat.display_disable);
                ImGui::Text("interrupt request: %u", stat.interrupt_request);
                ImGui::Text("dma data request: %u", stat.dma_data_request);
                ImGui::Text("ready to receive cmd: %u", stat.ready_to_receive_cmd);
                ImGui::Text("ready to send vram to cpu: %u", stat.ready_to_send_vram_to_cpu);
                ImGui::Text("ready to recieve dma block: %u", stat.ready_to_recieve_dma_block);
                ImGui::Text("dma dir: %u", stat.dma_dir);
                ImGui::Text("draw mode: %u", stat.draw_mode);

                ImGui::TreePop();
            }

            ImGui::Text("rect texture x flip: %u", gpu->rect_texture_x_flip);
            ImGui::Text("rect texture y flip: %u", gpu->rect_texture_y_flip);
            ImGui::Text("texture window x mask: %u", gpu->texture_window_x_mask);
            ImGui::Text("texture window y mask: %u", gpu->texture_window_y_mask);
            ImGui::Text("texture window x offset: %u", gpu->texture_window_x_offset);
            ImGui::Text("texture window y offset: %u", gpu->texture_window_y_offset);
            ImGui::Text("drawing area left: %u", gpu->drawing_area_left);
            ImGui::Text("drawing area top: %u", gpu->drawing_area_top);
            ImGui::Text("drawing area right: %u", gpu->drawing_area_right);
            ImGui::Text("drawing area bottom: %u", gpu->drawing_area_bottom);
            ImGui::Text("drawing offset x: %d", gpu->drawing_offset_x);
            ImGui::Text("drawing offset y: %d", gpu->drawing_offset_y);
            ImGui::Text("display vram x start: %u", gpu->display_vram_x_start);
            ImGui::Text("display vram y start: %u", gpu->display_vram_y_start);
            ImGui::Text("display horizontal start: %u", gpu->display_horiz_start);
            ImGui::Text("display horizontal end: %u", gpu->display_horiz_end);
            ImGui::Text("display line start: %u", gpu->display_line_start);
            ImGui::Text("display line end: %u", gpu->display_line_end);

            if (ImGui::TreeNode("GP0 Command Buffer")) {
                ImGui::Text("opcode: %u", gpu->gp0_cmd_opcode);
                ImGui::Text("commands received: %u (max 16)", gpu->gp0_cmd_buffer.size);
                ImGui::Text("commands left: %u", gpu->gp0_fn_info.args_left);
                ImGui::Text("data mode: %s", gpu->gp0_data_mode == gp0_data_mode_t::command ? "command" : "data");

                if (ImGui::TreeNode("Commands")) {
                    for (uint32_t i = 0; i < gpu->gp0_cmd_buffer.size; i++) {
                        ImGui::Text("0x%08X", gpu->gp0_cmd_buffer.buffer[i]);
                    }

                    ImGui::TreePop();
                }

                ImGui::TreePop();
            }


        ImGui::End();
    }

    void dma_channel_view(dma_t::channel_t* channel, uint32_t ind) {
        static const char* channel_str[] = {
            "MDECin",
            "MDECout",
            "GPU",
            "CDROM",
            "SPU",
            "PIO",
            "OTC",
        };

        if (ImGui::TreeNode(channel_str[ind])) {
            ImGui::Text("Base: 0x%08X", channel->base);
            ImGui::Text("Block: 0x%08X", channel->block);

            if (ImGui::TreeNode("Control")) {
                ImGui::Text("Direction: %s", channel->control.direction == dma_t::channel_t::control_t::transfer_dir_t::device_to_ram ? "device to ram" : "ram to device");
                ImGui::Text("Addr Step: %s", channel->control.addr_step ? "-4" : "+4");
                ImGui::Text("Chopping Enable: %s", channel->control.chopping_enable ? "true" : "false");
                ImGui::Text("Sync Mode: %s", channel->control.sync_mode == dma_t::channel_t::control_t::sync_mode_t::immediate ? "immediate" : channel->control.sync_mode == dma_t::channel_t::control_t::sync_mode_t::request ? "request" : "linked list");
                ImGui::Text("Chopping DMA Window Size: %u", channel->control.chopping_dma_window_size);
                ImGui::Text("Chopping CPU Window Size: %u", channel->control.chopping_cpu_window_size);
                ImGui::Text("Start Busy: %s", channel->control.start_busy ? "true" : "false");
                ImGui::Text("Start Trigger: %s", channel->control.start_trigger ? "true" : "false");

                ImGui::TreePop();
            }

            ImGui::TreePop();
        }
    }

    void display_dma_view(dma_t* dma) {
        ImGui::Begin("DMA");

            if (ImGui::TreeNode("Channels")) {
                int channel_ind = 0;
                for (auto& channel : dma->channels) {
                    dma_channel_view(&channel, channel_ind++);
                }

                ImGui::TreePop();
            }
            
            ImGui::Text("Control: 0x%08X", dma->control);
            ImGui::Text("Interrupt: 0x%08X", dma->interrupt.get_raw());

        ImGui::End();
    }

    void display_vram_view(vram_t* vram) {
        ImGui::Begin("VRAM");

            ImGui::Image((ImTextureID) (intptr_t) vram->tbo, ImVec2(1024, 512));
        
        ImGui::End();
    }
}

namespace {
    static bool show_emulation_view = true;
    static bool show_cpu_view = true;
    static bool show_gpu_view = false;
    static bool show_dma_view = false;
    static bool show_vram_view = false;
    static bool show_instr_view = true;
    static bool show_memory_view = true;
    static bool show_breakpoints_view = true;
    static bool show_log_view = true;

    void display_nav_bar() {
        if (ImGui::BeginMainMenuBar()) {
            if (ImGui::BeginMenu("Window")) {
                ImGui::MenuItem("Emulation", nullptr, &show_emulation_view);
                ImGui::MenuItem("CPU", nullptr, &show_cpu_view);
                ImGui::MenuItem("GPU", nullptr, &show_gpu_view);
                ImGui::MenuItem("DMA", nullptr, &show_dma_view);
                ImGui::MenuItem("VRAM", nullptr, &show_vram_view);
                ImGui::MenuItem("Instructions", nullptr, &show_instr_view);
                ImGui::MenuItem("Memory", nullptr, &show_memory_view);
                ImGui::MenuItem("Breakpoints", nullptr, &show_breakpoints_view);
                ImGui::MenuItem("Log", nullptr, &show_log_view);

                ImGui::EndMenu();
            }

            ImGui::EndMainMenuBar();
        }
    }
}

void ps1::debugger::display(ps1_t* console, emulation_settings_t* settings) {
    display_nav_bar();

    if (show_emulation_view) display_emulation_view(console, settings);
    if (show_cpu_view) display_cpu_view(&console->cpu, &console->bus);
    if (show_gpu_view) display_gpu_view(&console->gpu);
    if (show_dma_view) display_dma_view(&console->dma);
    if (show_vram_view) display_vram_view(&console->vram);
    if (show_instr_view) display_instr_view(&console->cpu, &console->bus);
    if (show_memory_view) display_memory_view(&console->bus);
    if (show_breakpoints_view) display_breakpoints_view(&console->cpu);
    if (show_log_view) logger::display();
}
