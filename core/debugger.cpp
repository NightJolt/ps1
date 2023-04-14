#include "debugger.h"

namespace ps1 {
    void display_instr(mem_addr_t addr, uint32_t instr) {
        static char addr_buffer[12];
        static char instr_buffer[12];

        sprintf(addr_buffer, "0x%08X", addr);
        sprintf(instr_buffer, "0x%08X", instr);

        ImGui::TableNextRow();

        ImGui::TableNextColumn();
        ImGui::TextWrapped(addr_buffer);
        
        ImGui::TableNextColumn();
        ImGui::TextWrapped(instr_buffer);
    }

    void display_reg(const str_t& reg_name, uint32_t reg_value) {
        static char hex_buffer[12];
        static char dec_buffer[12];

        sprintf(hex_buffer, "0x%08X", reg_value);
        sprintf(dec_buffer, "%u", reg_value);

        ImGui::TableNextRow();

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

            ImGui::TableNextRow();
            
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

            ImGui::TableNextRow();
            
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
                display_reg("REG_" + std::to_string(i), cpu->lo);
            }
            
            ImGui::EndTable();
        }

    ImGui::End();
}

void ps1::debugger::display_instr_view(cpu_t* cpu, bios_t* bios) {

}