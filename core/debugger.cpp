#include "debugger.h"
#include "bus.h"

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
}

void ps1::debugger::display_cpu_info(cpu_t* cpu) {
    ImGui::Begin("CPU");

        if (cpu->state == cpu_state_t::halted) {
            if (ImGui::Button("Revive")) {
                cpu_set_state(cpu, cpu_state_t::running);
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

        if (ImGui::BeginTable("delay_slot", 2, ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders | ImGuiTableFlags_SizingStretchProp)) {
            ImGui::TableSetupColumn(nullptr, 0, 1);
            ImGui::TableSetupColumn(nullptr, 0, 2);
            
            ImGui::TableNextColumn();
            ImGui::TextWrapped("Delay Slot");

            static char instr_buffer[12];
            sprintf(instr_buffer, "0x%08X", (uint32_t)cpu->delay_slot);
            
            ImGui::TableNextColumn();
            ImGui::TextWrapped(instr_buffer);
            
            ImGui::EndTable();
        }

        ImGui::Spacing();

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
            ImGui::TableSetupColumn(nullptr, 0, 1);
            ImGui::TableSetupColumn(nullptr, 0, 3);
            ImGui::TableSetupColumn(nullptr, 0, 3);
            
            for (int i = 0; i < 32; i++) {
                display_reg("REG_" + std::to_string(i), cpu->regs[i]);
            }
            
            ImGui::EndTable();
        }

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

        if (ImGui::BeginTable("InstructionsTable", 2, ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders | ImGuiTableFlags_SizingStretchProp)) {
            ImGui::TableSetupColumn(nullptr, 0, 1);
            ImGui::TableSetupColumn(nullptr, 0, 1);

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