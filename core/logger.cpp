#include "logger.h"

namespace {
    static dyn_arr_t <str_t> channels = dyn_arr_t <str_t> ();
    static dyn_arr_t <dyn_arr_t <str_t>> logs = dyn_arr_t <dyn_arr_t <str_t>> ();
    static dyn_arr_t <pair_t <uint32_t, uint32_t>> order = dyn_arr_t <pair_t <uint32_t, uint32_t>> ();
}

void ps1::logger::push(const str_t& msg, const str_t& channel) {
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
    }

    if (logs.size() == ind) logs.push_back(dyn_arr_t <str_t> ());

    logs[ind].emplace_back(msg);
    order.emplace_back(ind, logs[ind].size() - 1);
}

void ps1::logger::display() {
    ImGui::Begin("Debug Log");
        ImGui::BeginTabBar("Channels");

        if (ImGui::BeginTabItem("all##tab")) {
                ImGui::BeginChild("all##child");

                for (auto& msg : order) {
                    ImGui::Text(("[" + channels[msg.first] + "] " + logs[msg.first][msg.second]).c_str());
                }

                ImGui::EndChild();
            ImGui::EndTabItem();
        }

        for (uint32_t i = 0 ; i < channels.size(); i++) {
            if (channels[i] == "all") continue;

            if (ImGui::BeginTabItem(channels[i].c_str())) {
                    ImGui::BeginChild(("##" + channels[i]).c_str());

                    for (auto& msg : logs[i]) {
                        ImGui::Text(msg.c_str());
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