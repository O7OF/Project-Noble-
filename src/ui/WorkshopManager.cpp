#include "WorkshopManager.hpp"
#include <imgui.h>
#include <windows.h>
#include <shellapi.h>

namespace noble {

WorkshopManager::WorkshopManager() = default;

void WorkshopManager::RefreshMods(const std::filesystem::path& workshopPath, const std::filesystem::path& localModsPath) {
    m_mods.clear();

    // 1. Scan Steam Workshop Directory (if exists)
    if (std::filesystem::exists(workshopPath) && std::filesystem::is_directory(workshopPath)) {
        for (const auto& entry : std::filesystem::directory_iterator(workshopPath)) {
            if (entry.is_directory()) {
                ModItem mod;
                mod.id = entry.path().filename().string();
                mod.title = "Steam Workshop Item #" + mod.id;
                mod.folderPath = entry.path();
                mod.isFromWorkshop = true;
                mod.isEnabled = true;

                // Count .map files
                for (const auto& f : std::filesystem::recursive_directory_iterator(entry.path())) {
                    if (f.is_regular_file()) {
                        mod.sizeBytes += f.file_size();
                        if (f.path().extension() == ".map") {
                            mod.mapFiles.push_back(f.path().filename().string());
                        }
                    }
                }

                if (!mod.mapFiles.empty()) {
                    mod.title = mod.mapFiles[0] + " (Workshop Mod)";
                }

                m_mods.push_back(mod);
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
                    if (f.is_regular_file()) {
                        mod.sizeBytes += f.file_size();
                        if (f.path().extension() == ".map") {
                            mod.mapFiles.push_back(f.path().filename().string());
                        }
                    }
                }

                m_mods.push_back(mod);
            }
        }
    }

    // Fallback demo mod if no workshop items are yet subscribed
    if (m_mods.empty()) {
        m_mods.push_back({
            "workshop_demo_01",
            "Reach Evolved Overhaul (Demo Mod)",
            "Complete sandbox overhaul restoring classic CE-style health, weapon balance, and re-tuned AI.",
            "TheVengefulVadam",
            1024 * 1024 * 342, // ~342 MB
            { "m10_crash.map", "m20_sword.map" },
            true, true, workshopPath
        });

        m_mods.push_back({
            "local_forge_palette",
            "Expanded Forge Palette - Extinction Map Pack",
            "Includes 250+ cross-palette objects unlocked for Forge World, Tempest, and Anchor 9.",
            "Community Modder",
            1024 * 1024 * 45, // ~45 MB
            { "forge_halo.map" },
            true, false, localModsPath
        });
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
