#include "logger.h"

namespace {
    auto channels = dyn_arr_t <str_t> ();
    auto logs = dyn_arr_t <dyn_arr_t <pair_t <str_t, ps1::logger::type_t>>> ();
    auto order = dyn_arr_t <pair_t <uint32_t, uint32_t>> ();

    struct spam_item {
        str_t msg;
        spam_item* prev;
        spam_item* next;
    };

    spam_item* spam_list_head = nullptr;
    spam_item* spam_list_tail = nullptr;
    uint32_t spam_count = 0;
    constexpr uint32_t spam_max = 100;

    uint32_t get_type_color(ps1::logger::type_t type) {
        return
            type == ps1::logger::type_t::error ? IM_COL32(211, 47, 47, 255) :
                            type == ps1::logger::type_t::warning ? IM_COL32(255, 179, 0, 255) :
                            type == ps1::logger::type_t::info ? IM_COL32(3, 155, 229, 255):
                            IM_COL32(224, 224, 224, 255);
    }

    bool filter[4] = { 1, 1, 1, 1 };
    char search_buffer[256]  = { 0 };
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

void ps1::logger::spam(const str_t& msg) {
    spam_item* item = new spam_item { std::move(msg), nullptr, nullptr };
    
    spam_count++;

    if (!spam_list_head) {
        spam_list_head = item;
        spam_list_tail = item;
    } else {
        spam_list_tail->next = item;
        item->prev = spam_list_tail;
        spam_list_tail = item;
    }

    if (spam_count > spam_max) {
        spam_item* tmp = spam_list_head;

        spam_list_head = spam_list_head->next;
        spam_list_head->prev = nullptr;

        delete tmp;
    }
}

void ps1::logger::display() {
    ImGui::Begin("Debug Log");
        ImGui::PushStyleColor(ImGuiCol_Text, ::get_type_color(type_t::message));
        ImGui::Checkbox("message", filter);
        ImGui::PopStyleColor();

        ImGui::SameLine();
        
        ImGui::PushStyleColor(ImGuiCol_Text, ::get_type_color(type_t::info));
        ImGui::Checkbox("info", filter + 1);
        ImGui::PopStyleColor();

        ImGui::SameLine();
        
        ImGui::PushStyleColor(ImGuiCol_Text, ::get_type_color(type_t::warning));
        ImGui::Checkbox("warning", filter + 2);
        ImGui::PopStyleColor();

        ImGui::SameLine();
        
        ImGui::PushStyleColor(ImGuiCol_Text, ::get_type_color(type_t::error));
        ImGui::Checkbox("error", filter + 3);
        ImGui::PopStyleColor();

        ImGui::SameLine();

        if (ImGui::Button("Clear"))
        {
            channels.clear();
            logs.clear();
            order.clear();

            spam_item* item = spam_list_head;
            while (item) {
                spam_item* tmp = item;
                item = item->next;
                delete tmp;
            }
            spam_list_head = nullptr;
            spam_list_tail = nullptr;
            spam_count = 0;
        }

        ImGui::SameLine();

        ImGui::InputText("##SearchText", search_buffer, sizeof(search_buffer));

        ImGui::Spacing();

        ImGui::BeginTabBar("Channels", ImGuiTabBarFlags_FittingPolicyScroll);

        if (ImGui::BeginTabItem("all##tab")) {
            ImGui::BeginChild("all##child");

            if (ImGui::BeginTable("all##table", 2, ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders | ImGuiTableFlags_SizingStretchProp)) {
                ImGui::TableSetupColumn(nullptr, 0, 1);
                ImGui::TableSetupColumn(nullptr, 0, 6);

                for (auto& [channel_index, message_index] : order) {
                    const str_t& msg = logs[channel_index][message_index].first;
                    const type_t type = logs[channel_index][message_index].second;

                    if (!filter[(uint32_t)type] || (search_buffer[0] != '\0' && msg.find(search_buffer) == str_t::npos)) continue;

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
                        if (!filter[(uint32_t)type] || (search_buffer[0] != '\0' && msg.find(search_buffer) == str_t::npos)) continue;

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

        spam_item* item = spam_list_head;

        if (ImGui::BeginTabItem("spam##tab", nullptr, ImGuiTabItemFlags_Trailing)) {  
            ImGui::BeginChild("spam##child");

            if (ImGui::BeginTable("spam##table", 1, ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders)) {
                while (item) {
                    if (search_buffer[0] == '\0' || item->msg.find(search_buffer) != str_t::npos) {
                        ImGui::TableNextRow();
                        ImGui::TableNextColumn();

                        ImGui::PushStyleColor(ImGuiCol_Text, ::get_type_color(type_t::message));
                        ImGui::TextWrapped(item->msg.c_str());
                        ImGui::PopStyleColor();
                    }

                    item = item->next;
                }

                ImGui::EndTable();
            }

            if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
                ImGui::SetScrollHereY(1.0f);

            ImGui::EndChild();
            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    ImGui::End();
}