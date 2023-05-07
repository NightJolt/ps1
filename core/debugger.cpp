#include "debugger.h"
#include "bus.h"

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
}

namespace ps1 {
    void display_instr(mem_addr_t addr, uint32_t instr, bool active = false) {
        static char addr_buffer[12];
        static char instr_buffer[12];

        sprintf(addr_buffer, "0x%08X", addr);
        sprintf(instr_buffer, "0x%08X", instr);

        if (active) {
            ImGui::PushStyleColor(ImGuiCol_TableRowBg, IM_COL32(255, 235, 59, 150));
            ImGui::PushStyleColor(ImGuiCol_TableRowBgAlt, IM_COL32(255, 235, 59, 150));
        }

        ImGui::TableNextColumn();
        ImGui::TextWrapped(addr_buffer);
        
        ImGui::TableNextColumn();
        ImGui::TextWrapped(instr_buffer);
        
        ImGui::TableNextColumn();
        ImGui::TextWrapped(to_bin(instr).c_str());
        
        ImGui::TableNextRow();

        if (active) {
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

    void display_ram(uint32_t addr, uint32_t value) {
        static char addr_buffer[12];
        static char hex_buffer[12];
        static char dec_buffer[12];

        sprintf(addr_buffer, "0x%08X", addr);
        sprintf(hex_buffer, "0x%08X", value);
        sprintf(dec_buffer, "%u", value);

        ImGui::TableNextColumn();
        ImGui::TextWrapped(addr_buffer);

        ImGui::TableNextColumn();
        ImGui::TextWrapped(hex_buffer);
        
        ImGui::TableNextColumn();
        ImGui::TextWrapped(dec_buffer);
    }
    
    str_t describe_reg(uint32_t i) {
        if (i == 0) return "($zero)";
        if (i == 1) return "($at)";
        if (i == 2) return "($v0)";
        if (i == 3) return "($v1)";
        if (i == 4) return "($a0)";
        if (i == 5) return "($a1)";
        if (i == 6) return "($a2)";
        if (i == 7) return "($a3)";
        if (i == 8) return "($t0)";
        if (i == 9) return "($t1)";
        if (i == 10) return "($t2)";
        if (i == 11) return "($t3)";
        if (i == 12) return "($t4)";
        if (i == 13) return "($t5)";
        if (i == 14) return "($t6)";
        if (i == 15) return "($t7)";
        if (i == 16) return "($s0)";
        if (i == 17) return "($s1)";
        if (i == 18) return "($s2)";
        if (i == 19) return "($s3)";
        if (i == 20) return "($s4)";
        if (i == 21) return "($s5)";
        if (i == 22) return "($s6)";
        if (i == 23) return "($s7)";
        if (i == 24) return "($t8)";
        if (i == 25) return "($t9)";
        if (i == 26) return "($k0)";
        if (i == 27) return "($k1)";
        if (i == 28) return "($gp)";
        if (i == 29) return "($sp)";
        if (i == 30) return "($fp)";
        if (i == 31) return "($ra)";

        return "";
    }
}

void ps1::debugger::display_cpu_info(cpu_t* cpu) {
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

            if (ImGui::Button("Jump")) {
                cpu_set_state(cpu, cpu_state_t::running);
                
                while (true) {
                    if (cpu->state != ps1::cpu_state_t::running) break;
                    
                    ps1::cpu_tick(cpu);
                }
            }

            ImGui::SameLine();

            if (ImGui::Button("Step")) {
                cpu_tick(cpu);
            }
        }

        if (ImGui::BeginTable("delay_slot", 2, ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders | ImGuiTableFlags_SizingStretchProp)) {
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
            ImGui::TableSetupColumn("Address", 0, 4);
            ImGui::TableSetupColumn("Value", 0, 13);

            ImGui::TableHeadersRow();
            
            {
                ImGui::TableNextColumn();
                ImGui::TextWrapped("Instruction");

                static char instr_buffer[12];
                sprintf(instr_buffer, "0x%08X", (uint32_t)cpu->instr_delay_slot);
                
                ImGui::TableNextColumn();
                ImGui::TextWrapped(instr_buffer);
                
                ImGui::TableNextColumn();
                ImGui::TextWrapped(to_bin((uint32_t)cpu->instr_delay_slot).c_str());
            }
            
            {
                ImGui::TableNextColumn();
                ImGui::TextWrapped("Load");
                
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

namespace ps1 {
    uint32_t bus_fetch32_debug(bus_t* bus, mem_addr_t mem_addr) {
        ASSERT(mem_addr % 4 == 0, "Unaligned memory access");

        for (auto& device_info : bus->devices) {
            if (device_info.mem_range.contains(mem_addr)) {
                return device_info.fetch32(device_info.device, device_info.mem_range.offset(mem_addr));
            }
        }

        return 0;
    }
}

void ps1::debugger::display_instr_view(cpu_t* cpu, bus_t* bus) {
    ImGui::Begin("Instructions");

        if (ImGui::BeginTable("InstructionsTable", 3, ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders | ImGuiTableFlags_SizingStretchProp)) {
            ImGui::TableSetupColumn(nullptr, 0, 4);
            ImGui::TableSetupColumn(nullptr, 0, 4);
            ImGui::TableSetupColumn(nullptr, 0, 13);

            static constexpr uint32_t instr_radius = 16;
            
            for (mem_addr_t addr = cpu->pc - instr_radius * sizeof(cpu_instr_t); addr < cpu->pc; addr += sizeof(cpu_instr_t)) {
                display_instr(addr, bus_fetch32_debug(bus, addr));
            }

            display_instr(cpu->pc, bus_fetch32_debug(bus, cpu->pc), true);
            
            for (mem_addr_t addr = cpu->pc + sizeof(cpu_instr_t); addr <= cpu->pc + instr_radius * sizeof(cpu_instr_t); addr += sizeof(cpu_instr_t)) {
                display_instr(addr, bus_fetch32_debug(bus, addr));
            }
            
            ImGui::EndTable();
        }

    ImGui::End();
}

void ps1::debugger::display_ram_view(ram_t* ram) {
    static int32_t addr_inp = 0;
    static int32_t addr = 0;
    bool update = false;

    ImGui::Begin("RAM");

        ImGui::SetNextItemWidth(-1);

        if (ImGui::InputInt(" ", &addr_inp, sizeof(cpu_instr_t), sizeof(cpu_instr_t) * 25)) {
            if (addr_inp >= 0 && addr_inp < RAM_SIZE && addr_inp % sizeof(cpu_instr_t) == 0) addr = addr_inp, update = true;
            else addr_inp = addr;
        }

        ImGui::BeginChild("ram_view");

            if (ImGui::BeginTable("ram", 3, ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders | ImGuiTableFlags_SizingStretchProp)) {
                ImGui::TableSetupColumn(nullptr, 0, 1);
                ImGui::TableSetupColumn(nullptr, 0, 1);
                ImGui::TableSetupColumn(nullptr, 0, 1);

                static int32_t radius = sizeof(cpu_instr_t) * 32;
                
                for (mem_addr_t offset = std::max(addr - radius, 0); offset <= std::min(addr + radius, (int32_t)RAM_SIZE); offset += sizeof(cpu_instr_t)) {
                    display_ram(offset, *(uint32_t*)(ram->data + offset));
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
