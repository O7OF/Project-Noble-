#include "WorkshopManager.hpp"
#include "MenuSounds.hpp"
#include "UiPad.hpp"
#include <imgui.h>
#include <nlohmann/json.hpp>
#include <windows.h>
#include <shellapi.h>
#include <algorithm>
#include <cctype>
#include <fstream>

namespace noble {

WorkshopManager::WorkshopManager() = default;

namespace {

// Blam map header check, used only for local mods that have no ModInfo.json. It can't tell Reach from
// Halo 3 (Halo 3 MCC maps pass it too), so Workshop items are identified by their ModInfo.json instead.
bool IsBlamMap(const std::filesystem::path& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) return false;
    char header[8] = { 0 };
    file.read(header, 8);
    if (file.gcount() < 8) return false;
    const uint32_t magic = *reinterpret_cast<uint32_t*>(header);
    if (magic != 0x68656164 && magic != 0x64616568) return false;
    const uint32_t version = *reinterpret_cast<uint32_t*>(header + 4);
    return version == 13 || version == 12;
}

std::string Lower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return s;
}

// MCC localised text: { "Neutral": "..." } (other language keys may follow). Plain strings are accepted too.
std::string LocalisedText(const nlohmann::json& j, const char* key) {
    if (!j.contains(key)) return {};
    const auto& v = j[key];
    if (v.is_string()) return v.get<std::string>();
    if (v.is_object()) {
        if (v.contains("Neutral") && v["Neutral"].is_string()) return v["Neutral"].get<std::string>();
        for (const auto& item : v.items()) {
            if (item.value().is_string()) return item.value().get<std::string>();
        }
    }
    return {};
}

struct ModInfo {
    bool found = false;
    std::string engine;       // e.g. "Halo3", "HaloReach"
    std::string title;
    std::string description;
};

// MCC's mod manifest, <mod folder>\ModInfo.json.
ModInfo ReadModInfo(const std::filesystem::path& folder) {
    ModInfo info;
    std::ifstream file(folder / "ModInfo.json", std::ios::binary);
    if (!file.is_open()) return info;
    try {
        const nlohmann::json j = nlohmann::json::parse(file);
        info.found = true;
        if (j.contains("Engine") && j["Engine"].is_string()) info.engine = j["Engine"].get<std::string>();
        info.title = LocalisedText(j, "Title");
        info.description = LocalisedText(j, "Description");
    } catch (...) {
        info.found = false;
    }
    return info;
}

bool IsReachEngine(const std::string& engine) {
    return Lower(engine).find("reach") != std::string::npos;
}

void CollectMaps(const std::filesystem::path& folder, ModItem& mod, bool checkHeader) {
    std::error_code ec;
    for (auto it = std::filesystem::recursive_directory_iterator(folder, ec);
         !ec && it != std::filesystem::recursive_directory_iterator(); it.increment(ec)) {
        if (!it->is_regular_file(ec) || it->path().extension() != ".map") continue;
        if (checkHeader && !IsBlamMap(it->path())) continue;
        mod.sizeBytes += it->file_size(ec);
        mod.mapFiles.push_back(it->path().filename().string());
    }
}

std::string FormatSize(uintmax_t bytes) {
    char buf[32];
    const double mb = static_cast<double>(bytes) / (1024.0 * 1024.0);
    if (mb >= 1024.0) std::snprintf(buf, sizeof(buf), "%.1f GB", mb / 1024.0);
    else std::snprintf(buf, sizeof(buf), "%.0f MB", mb);
    return buf;
}

} // namespace

void WorkshopManager::RefreshMods(const std::filesystem::path& workshopPath, const std::filesystem::path& localModsPath) {
    m_workshopPath = workshopPath;
    m_localModsPath = localModsPath;
    m_mods.clear();
    m_skippedOtherGames = 0;
    m_selectedModIndex = -1;
    std::error_code ec;

    // 1. Steam Workshop items for MCC (app 976730). Only items whose ModInfo.json names Halo: Reach.
    if (std::filesystem::is_directory(workshopPath, ec)) {
        for (const auto& entry : std::filesystem::directory_iterator(workshopPath, ec)) {
            if (!entry.is_directory()) continue;
            const ModInfo info = ReadModInfo(entry.path());
            if (!info.found || !IsReachEngine(info.engine)) {
                ++m_skippedOtherGames;
                continue;
            }
            ModItem mod;
            mod.id = entry.path().filename().string();
            mod.title = info.title.empty() ? "Workshop item " + mod.id : info.title;
            mod.description = info.description;
            mod.folderPath = entry.path();
            mod.isFromWorkshop = true;
            mod.isEnabled = true;
            CollectMaps(entry.path(), mod, false);
            m_mods.push_back(std::move(mod));
        }
    }

    // 2. Noble's own data\mods folders. A ModInfo.json is used when present, otherwise the map headers.
    if (std::filesystem::is_directory(localModsPath, ec)) {
        for (const auto& entry : std::filesystem::directory_iterator(localModsPath, ec)) {
            if (!entry.is_directory()) continue;
            const ModInfo info = ReadModInfo(entry.path());
            if (info.found && !info.engine.empty() && !IsReachEngine(info.engine)) {
                ++m_skippedOtherGames;
                continue;
            }
            ModItem mod;
            mod.id = entry.path().filename().string();
            mod.title = info.title.empty() ? mod.id : info.title;
            mod.description = info.description;
            mod.folderPath = entry.path();
            mod.isFromWorkshop = false;
            mod.isEnabled = true;
            CollectMaps(entry.path(), mod, !info.found);
            if (!mod.mapFiles.empty() || info.found) m_mods.push_back(std::move(mod));
        }
    }

    std::sort(m_mods.begin(), m_mods.end(), [](const ModItem& a, const ModItem& b) {
        return Lower(a.title) < Lower(b.title);
    });
}

void WorkshopManager::Render() {
    const float em = ImGui::GetFontSize();   // sizes scale with the UI font (fixed pixels were cut off at 4K)

    ImGui::TextColored(ImVec4(0.42f, 0.74f, 0.93f, 1.0f), "HALO: REACH MODS");
    ImGui::SameLine();
    ImGui::TextDisabled("  %zu installed", m_mods.size());
    if (m_skippedOtherGames > 0) {
        ImGui::SameLine();
        ImGui::TextDisabled("(%d for other Halo games not shown)", m_skippedOtherGames);
    }
    ImGui::Separator();
    ImGui::Spacing();

    if (ImGui::Button("Browse Steam Workshop")) {
        ShellExecuteA(nullptr, "open", "https://steamcommunity.com/app/976730/workshop/", nullptr, nullptr, SW_SHOWNORMAL);
    }
    ImGui::SameLine();
    if (ImGui::Button("Open Local Mods Folder")) {
        std::error_code ec;
        std::filesystem::create_directories(m_localModsPath, ec);
        ShellExecuteW(nullptr, L"open", m_localModsPath.wstring().c_str(), nullptr, nullptr, SW_SHOWNORMAL);
    }
    ImGui::SameLine();
    if (ImGui::Button("Refresh List")) {
        RefreshMods(m_workshopPath, m_localModsPath);
    }
    ImGui::SameLine();
    static char s_search[64] = {};
    ImGui::SetNextItemWidth(em * 16.0f);
    ImGui::InputTextWithHint("##modsearch", "Search mods", s_search, sizeof(s_search));
    const auto matches = [&](const ModItem& mod) {
        const auto lower = [](std::string s) { for (char& c : s) c = static_cast<char>(::tolower(static_cast<unsigned char>(c))); return s; };
        return lower(mod.title).find(lower(s_search)) != std::string::npos;
    };
    ImGui::Spacing();

    // Controller: D-pad or stick picks a mod, the triggers jump, X refreshes.
    bool scrollToSelected = false;
    if (!m_mods.empty() && !ImGui::GetIO().WantTextInput) {
        std::vector<int> shown;
        for (int i = 0; i < static_cast<int>(m_mods.size()); ++i) if (matches(m_mods[i])) shown.push_back(i);
        const int count = static_cast<int>(shown.size());
        int move = 0;
        if (UiPad::Down()) move = 1;
        if (UiPad::Up()) move = -1;
        if (UiPad::PageDown()) move = 8;
        if (UiPad::PageUp()) move = -8;
        if (move != 0 && count > 0) {
            int position = -1;
            for (int k = 0; k < count; ++k) if (shown[k] == m_selectedModIndex) position = k;
            m_selectedModIndex = shown[position < 0 ? 0 : std::clamp(position + move, 0, count - 1)];
            scrollToSelected = true;
            MenuSounds::Scroll();
        }
    }
    if (!ImGui::GetIO().WantTextInput && ImGui::IsKeyPressed(ImGuiKey_GamepadFaceLeft, false)) RefreshMods(m_workshopPath, m_localModsPath);

    // Description of the selected mod below the list.
    const float detailHeight = em * 6.5f;
    const ImGuiTableFlags flags = ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_ScrollY;
    if (ImGui::BeginTable("ModsTable", 4, flags, ImVec2(0.0f, -detailHeight))) {
        ImGui::TableSetupScrollFreeze(0, 1);
        ImGui::TableSetupColumn("Mod", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("Source", ImGuiTableColumnFlags_WidthFixed, em * 9.0f);
        ImGui::TableSetupColumn("Size", ImGuiTableColumnFlags_WidthFixed, em * 5.5f);
        ImGui::TableSetupColumn("Maps", ImGuiTableColumnFlags_WidthFixed, em * 4.5f);
        ImGui::TableHeadersRow();

        for (int i = 0; i < static_cast<int>(m_mods.size()); ++i) {
            const ModItem& mod = m_mods[i];
            if (!matches(mod)) continue;
            ImGui::PushID(i);   // titles can repeat; the row index keeps every widget ID unique
            ImGui::TableNextRow();

            ImGui::TableSetColumnIndex(0);
            if (ImGui::Selectable(mod.title.c_str(), m_selectedModIndex == i, ImGuiSelectableFlags_SpanAllColumns)) {
                m_selectedModIndex = i;
            }
            if (scrollToSelected && m_selectedModIndex == i) ImGui::SetScrollHereY(0.5f);

            ImGui::TableSetColumnIndex(1);
            if (mod.isFromWorkshop) ImGui::TextColored(ImVec4(0.42f, 0.74f, 0.93f, 1.0f), "Steam Workshop");
            else ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, 1.0f), "Local");

            ImGui::TableSetColumnIndex(2);
            ImGui::TextUnformatted(FormatSize(mod.sizeBytes).c_str());

            ImGui::TableSetColumnIndex(3);
            ImGui::Text("%zu", mod.mapFiles.size());
            ImGui::PopID();
        }
        ImGui::EndTable();
    }

    ImGui::Separator();
    if (m_mods.empty()) {
        ImGui::TextWrapped("No Halo: Reach mods found. Subscribe to Reach mods on the Steam Workshop, or put a mod "
                           "folder in Noble's data\\mods folder, then press Refresh List.");
    } else if (m_selectedModIndex >= 0 && m_selectedModIndex < static_cast<int>(m_mods.size())) {
        const ModItem& mod = m_mods[m_selectedModIndex];
        ImGui::BeginChild("##moddetail", ImVec2(0, 0), ImGuiChildFlags_None);
        ImGui::TextUnformatted(mod.title.c_str());
        ImGui::TextDisabled("%s", mod.isFromWorkshop ? ("Workshop item " + mod.id).c_str() : mod.folderPath.string().c_str());
        ImGui::TextWrapped("%s", mod.description.empty() ? "No description." : mod.description.c_str());
        ImGui::EndChild();
    } else {
        ImGui::TextDisabled("Select a mod to see its description.");
    }
}

} // namespace noble
