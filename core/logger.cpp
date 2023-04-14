#include "logger.h"

namespace {
    auto channels = dyn_arr_t <str_t> ();
    auto logs = dyn_arr_t <dyn_arr_t <pair_t <str_t, ps1::logger::type_t>>> ();
    auto order = dyn_arr_t <pair_t <uint32_t, uint32_t>> ();

    uint32_t get_type_color(ps1::logger::type_t type) {
        return
            type == ps1::logger::type_t::error ? IM_COL32(211, 47, 47, 255) :
                            type == ps1::logger::type_t::warning ? IM_COL32(255, 179, 0, 255) :
                            type == ps1::logger::type_t::info ? IM_COL32(3, 155, 229, 255):
                            IM_COL32(224, 224, 224, 255);
    }

    bool filter[4] = { 1, 1, 1, 1 };
}

void ps1::logger::push(const str_t& msg, type_t type, const str_t& channel) {
    uint32_t ind = -1;

    for (uint32_t i = 0 ; i < channels.size(); i++) {
        if (channels[i] == channel) {
            ind = i;

            break;
        }
    }

    if (ind == -1) {
        channels.emplace_back(channel);
        ind = channels.size() - 1;
        logs.resize(logs.size() + 1);
    }

    logs[ind].emplace_back(msg, type);
    order.emplace_back(ind, logs[ind].size() - 1);
}

void ps1::logger::display() {
    ImGui::Begin("Debug Log");
        ImGui::PushStyleColor(ImGuiCol_Text, ::get_type_color(type_t::message));
        ImGui::Checkbox("message", ::filter);
        ImGui::PopStyleColor();

        ImGui::SameLine();
        
        ImGui::PushStyleColor(ImGuiCol_Text, ::get_type_color(type_t::info));
        ImGui::Checkbox("info", ::filter + 1);
        ImGui::PopStyleColor();

        ImGui::SameLine();
        
        ImGui::PushStyleColor(ImGuiCol_Text, ::get_type_color(type_t::warning));
        ImGui::Checkbox("warning", ::filter + 2);
        ImGui::PopStyleColor();

        ImGui::SameLine();
        
        ImGui::PushStyleColor(ImGuiCol_Text, ::get_type_color(type_t::error));
        ImGui::Checkbox("error", ::filter + 3);
        ImGui::PopStyleColor();

        ImGui::Spacing();

        ImGui::BeginTabBar("Channels");

        if (ImGui::BeginTabItem("all##tab")) {  
            ImGui::BeginChild("all##child");

            if (ImGui::BeginTable("all##table", 2, ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders | ImGuiTableFlags_SizingStretchProp)) {
                for (auto& [channel_index, message_index] : order) {
                    const str_t& msg = logs[channel_index][message_index].first;
                    const type_t type = logs[channel_index][message_index].second;

                    if (!::filter[(uint32_t)type]) continue;

                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();
                    ImGui::TextWrapped(channels[channel_index].c_str());
                    ImGui::TableNextColumn();
                    ImGui::PushStyleColor(ImGuiCol_Text, ::get_type_color(type));
                    ImGui::TextWrapped(msg.c_str());
                    ImGui::PopStyleColor();
                }

                ImGui::EndTable();
            }

            if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
                ImGui::SetScrollHereY(1.0f);

            ImGui::EndChild();
            ImGui::EndTabItem();
        }

        for (uint32_t i = 0 ; i < channels.size(); i++) {
            if (channels[i] == "all") continue;

            if (ImGui::BeginTabItem((channels[i] + "##tab").c_str())) {  
                ImGui::BeginChild((channels[i] + "##child").c_str());

                if (ImGui::BeginTable((channels[i] + "##table").c_str(), 1, ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders)) {
                    for (const auto& [msg, type] : logs[i]) {
                        if (!::filter[(uint32_t)type]) continue;

                        ImGui::TableNextRow();
                        ImGui::TableNextColumn();

                        ImGui::PushStyleColor(ImGuiCol_Text, ::get_type_color(type));
                        ImGui::TextWrapped(msg.c_str());
                        ImGui::PopStyleColor();
                    }

                    ImGui::EndTable();
                }

                if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
                    ImGui::SetScrollHereY(1.0f);

                ImGui::EndChild();
                ImGui::EndTabItem();
            }
        }

        ImGui::EndTabBar();
    ImGui::End();
}