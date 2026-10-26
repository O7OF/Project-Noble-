#include "WorkshopManager.hpp"
#include <imgui.h>
#include <windows.h>
#include <shellapi.h>
#include <fstream>

namespace noble {

WorkshopManager::WorkshopManager() = default;

static bool IsValidReachMap(const std::filesystem::path& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) return false;

    char header[8] = { 0 };
    file.read(header, 8);
    if (file.gcount() < 8) return false;

    // Check Blam header magic: 'daeh' (0x64616568) or 'head' (0x68656164)
    uint32_t magic = *reinterpret_cast<uint32_t*>(header);
    if (magic != 0x68656164 && magic != 0x64616568) {
        return false;
    }

    // Check Halo: Reach engine version (13 = 0xD for MCC Reach, 12 = 0xC for 360 Reach)
    uint32_t version = *reinterpret_cast<uint32_t*>(header + 4);
    return (version == 13 || version == 12);
}

void WorkshopManager::RefreshMods(const std::filesystem::path& workshopPath, const std::filesystem::path& localModsPath) {
    m_mods.clear();

    // 1. Scan Steam Workshop Directory (strictly Halo: Reach maps)
    if (std::filesystem::exists(workshopPath) && std::filesystem::is_directory(workshopPath)) {
        for (const auto& entry : std::filesystem::directory_iterator(workshopPath)) {
            if (entry.is_directory()) {
                ModItem mod;
                mod.id = entry.path().filename().string();
                mod.title = "Steam Workshop Item #" + mod.id;
                mod.folderPath = entry.path();
                mod.isFromWorkshop = true;
                mod.isEnabled = true;

                // Inspect .map files and ensure only Reach maps are registered
                for (const auto& f : std::filesystem::recursive_directory_iterator(entry.path())) {
                    if (f.is_regular_file() && f.path().extension() == ".map") {
                        if (IsValidReachMap(f.path())) {
                            mod.sizeBytes += f.file_size();
                            mod.mapFiles.push_back(f.path().filename().string());
                        }
                    }
                }

                // Only include if item contains valid Halo: Reach maps
                if (!mod.mapFiles.empty()) {
                    mod.title = mod.mapFiles[0] + " [Halo: Reach]";
                    m_mods.push_back(mod);
                }
            }
        }
    }

    // 2. Scan Local User Mods Directory (data/mods)
    if (std::filesystem::exists(localModsPath) && std::filesystem::is_directory(localModsPath)) {
        for (const auto& entry : std::filesystem::directory_iterator(localModsPath)) {
            if (entry.is_directory()) {
                ModItem mod;
                mod.id = entry.path().filename().string();
                mod.title = mod.id;
                mod.folderPath = entry.path();
                mod.isFromWorkshop = false;
                mod.isEnabled = true;

                for (const auto& f : std::filesystem::recursive_directory_iterator(entry.path())) {
                    if (f.is_regular_file() && f.path().extension() == ".map") {
                        if (IsValidReachMap(f.path())) {
                            mod.sizeBytes += f.file_size();
                            mod.mapFiles.push_back(f.path().filename().string());
                        }
                    }
                }

                if (!mod.mapFiles.empty()) {
                    mod.title = mod.id + " [Halo: Reach Custom]";
                    m_mods.push_back(mod);
                }
            }
        }
    }
}

void WorkshopManager::Render() {
    ImGui::TextColored(ImVec4(0.0f, 0.85f, 1.0f, 1.0f), "MOD MANAGEMENT // STEAM WORKSHOP & LOCAL PACKAGES");
    ImGui::Separator();
    ImGui::Spacing();

    // Top action bar
    if (ImGui::Button("Browse Steam Workshop...", ImVec2(190, 26))) {
        ShellExecuteA(nullptr, "open", "https://steamcommunity.com/app/976730/workshop/", nullptr, nullptr, SW_SHOWNORMAL);
    }
    ImGui::SameLine();
    if (ImGui::Button("Open Local Mods Folder", ImVec2(180, 26))) {
        ShellExecuteA(nullptr, "open", "data\\mods", nullptr, nullptr, SW_SHOWNORMAL);
    }
    ImGui::SameLine();
    if (ImGui::Button("Refresh List", ImVec2(110, 26))) {
        // Will be refreshed by UIManager passing paths
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // Mod Table
    static ImGuiTableFlags flags = 
        ImGuiTableFlags_RowBg | 
        ImGuiTableFlags_BordersInnerV | 
        ImGuiTableFlags_ScrollY;

    if (ImGui::BeginTable("ModsTable", 5, flags)) {
        ImGui::TableSetupColumn("Active", ImGuiTableColumnFlags_WidthFixed, 50.0f);
        ImGui::TableSetupColumn("Mod Title", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("Source", ImGuiTableColumnFlags_WidthFixed, 130.0f);
        ImGui::TableSetupColumn("Size", ImGuiTableColumnFlags_WidthFixed, 90.0f);
        ImGui::TableSetupColumn("Maps Included", ImGuiTableColumnFlags_WidthFixed, 150.0f);
        ImGui::TableHeadersRow();

        for (int i = 0; i < static_cast<int>(m_mods.size()); ++i) {
            auto& mod = m_mods[i];
            ImGui::TableNextRow();

            // Col 0: Active checkbox
            ImGui::TableSetColumnIndex(0);
            ImGui::PushID(i * 10);
            ImGui::Checkbox("##Active", &mod.isEnabled);
            ImGui::PopID();

            // Col 1: Title
            ImGui::TableSetColumnIndex(1);
            bool isSelected = (m_selectedModIndex == i);
            if (ImGui::Selectable(mod.title.c_str(), isSelected, ImGuiSelectableFlags_SpanAllColumns)) {
                m_selectedModIndex = i;
            }

            // Col 2: Source
            ImGui::TableSetColumnIndex(2);
            if (mod.isFromWorkshop) {
                ImGui::TextColored(ImVec4(0.2f, 0.8f, 1.0f, 1.0f), "Steam Workshop");
            } else {
                ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, 1.0f), "Local (data/mods)");
            }

            // Col 3: Size
            ImGui::TableSetColumnIndex(3);
            double sizeMb = static_cast<double>(mod.sizeBytes) / (1024.0 * 1024.0);
            ImGui::Text("%.1f MB", sizeMb);

            // Col 4: Maps
            ImGui::TableSetColumnIndex(4);
            ImGui::Text("%zu map file(s)", mod.mapFiles.size());
        }
        ImGui::EndTable();
    }
}

} // namespace noble
