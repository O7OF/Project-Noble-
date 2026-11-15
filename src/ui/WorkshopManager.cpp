#include "WorkshopManager.hpp"
#include "MenuSounds.hpp"
#include "NobleStyle.hpp"
#include "NobleUi.hpp"
#include "UiArt.hpp"
#include "UiPad.hpp"
#include "common/MapNames.hpp"
#include <imgui.h>
#include <nlohmann/json.hpp>
#include <windows.h>
#include <shellapi.h>
#include <algorithm>
#include <cctype>
#include <cfloat>
#include <cstdio>
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

std::string EllipsizeModText(ImFont* font, float pxSize, const std::string& text, float maxW) {
    if (maxW <= 4.0f || text.empty()) return "";
    ImFont* f = font ? font : ImGui::GetFont();
    if (f->CalcTextSizeA(pxSize, FLT_MAX, 0.0f, text.c_str()).x <= maxW) return text;
    const float dotsW = f->CalcTextSizeA(pxSize, FLT_MAX, 0.0f, "...").x;
    std::string out = text;
    while (!out.empty()) {
        out.pop_back();
        if (f->CalcTextSizeA(pxSize, FLT_MAX, 0.0f, out.c_str()).x + dotsW <= maxW) return out + "...";
    }
    return "...";
}

void CollectMaps(const std::filesystem::path& folder, ModItem& mod, bool checkHeader) {
    std::error_code ec;
    std::filesystem::path fallbackImg;
    for (auto it = std::filesystem::recursive_directory_iterator(folder, ec);
         !ec && it != std::filesystem::recursive_directory_iterator(); it.increment(ec)) {
        if (!it->is_regular_file(ec)) continue;
        const std::string ext = Lower(it->path().extension().string());
        if (ext == ".map") {
            if (checkHeader && !IsBlamMap(it->path())) continue;
            mod.sizeBytes += it->file_size(ec);
            mod.mapFiles.push_back(it->path().filename().string());
        } else if (ext == ".png" || ext == ".jpg" || ext == ".jpeg") {
            const std::string stem = Lower(it->path().stem().string());
            if (mod.previewImage.empty() &&
                (stem.find("preview") != std::string::npos || stem.find("thumb") != std::string::npos ||
                 stem.find("large") != std::string::npos || stem.find("loading") != std::string::npos)) {
                mod.previewImage = it->path();
            } else if (fallbackImg.empty()) {
                fallbackImg = it->path();
            }
        }
    }
    if (mod.previewImage.empty() && !fallbackImg.empty()) {
        mod.previewImage = fallbackImg;
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
    if (!m_mods.empty()) m_selectedModIndex = 0;
}

void WorkshopManager::Render() {
    const float sScale = NobleUi::Scale();
    auto U = [sScale](float px) { return px * sScale; };
    ImDrawList* dl = ImGui::GetWindowDrawList();
    ImFont* titleFont = NobleStyle::TitleFont() ? NobleStyle::TitleFont() : ImGui::GetFont();
    ImFont* bodyFont = ImGui::GetFont();

    // Header + action bar
    const ImVec2 topPos = ImGui::GetCursorScreenPos();
    const float fullW = ImGui::GetContentRegionAvail().x;
    dl->AddText(titleFont, 22.0f * sScale, topPos, IM_COL32(236, 244, 252, 255), "HALO: REACH MODS");
    char statsBuf[128];
    if (m_skippedOtherGames > 0) {
        std::snprintf(statsBuf, sizeof(statsBuf), "%zu INSTALLED   (%d FOR OTHER HALO GAMES HIDDEN)",
                      m_mods.size(), m_skippedOtherGames);
    } else {
        std::snprintf(statsBuf, sizeof(statsBuf), "%zu INSTALLED", m_mods.size());
    }
    const ImVec2 stSz = bodyFont->CalcTextSizeA(13.5f * sScale, FLT_MAX, 0.0f, statsBuf);
    dl->AddText(bodyFont, 13.5f * sScale, ImVec2(topPos.x + fullW - stSz.x, topPos.y + 4.0f * sScale),
                IM_COL32(143, 216, 255, 220), statsBuf);
    dl->AddLine(ImVec2(topPos.x, topPos.y + 28.0f * sScale), ImVec2(topPos.x + fullW, topPos.y + 28.0f * sScale),
                IM_COL32(72, 128, 175, 110), 1.0f * sScale);
    ImGui::Dummy(ImVec2(fullW, 34.0f * sScale));

    static char s_search[64] = {};
    const float pillH = U(28.0f);
    auto drawToolBtn = [&](const char* id, const char* label) -> bool {
        const ImVec2 ts = bodyFont->CalcTextSizeA(13.5f * sScale, FLT_MAX, 0.0f, label);
        const float w = ts.x + U(24.0f);
        const ImVec2 p0 = ImGui::GetCursorScreenPos();
        const ImVec2 p1(p0.x + w, p0.y + pillH);
        const bool clicked = ImGui::InvisibleButton(id, ImVec2(w, pillH));
        const bool hov = ImGui::IsItemHovered();
        dl->AddRectFilled(p0, p1, hov ? IM_COL32(34, 78, 112, 235) : IM_COL32(16, 34, 52, 215), 2.0f * sScale);
        dl->AddRect(p0, p1, hov ? NobleUi::kAmber : IM_COL32(84, 150, 200, 175), 2.0f * sScale);
        dl->AddText(bodyFont, 13.5f * sScale, ImVec2(p0.x + U(12.0f), p0.y + (pillH - ts.y) * 0.5f),
                    IM_COL32(244, 248, 252, 255), label);
        return clicked;
    };

    ImGui::SetNextItemWidth(U(230.0f));
    ImGui::InputTextWithHint("##modsearch", "Search installed mods...", s_search, sizeof(s_search));
    ImGui::SameLine(0.0f, U(8.0f));
    if (drawToolBtn("##modWorkshop", "BROWSE STEAM WORKSHOP")) {
        ShellExecuteA(nullptr, "open", "https://steamcommunity.com/app/976730/workshop/", nullptr, nullptr, SW_SHOWNORMAL);
        MenuSounds::Click();
    }
    ImGui::SameLine(0.0f, U(8.0f));
    if (drawToolBtn("##modLocalDir", "OPEN LOCAL MODS FOLDER")) {
        std::error_code ec;
        std::filesystem::create_directories(m_localModsPath, ec);
        ShellExecuteW(nullptr, L"open", m_localModsPath.wstring().c_str(), nullptr, nullptr, SW_SHOWNORMAL);
        MenuSounds::Click();
    }
    ImGui::SameLine(0.0f, U(8.0f));
    if (drawToolBtn("##modRefresh", UiPad::HasGamepad() ? "REFRESH [X]" : "REFRESH")) {
        RefreshMods(m_workshopPath, m_localModsPath);
        MenuSounds::Click();
    }

    const auto matches = [&](const ModItem& mod) {
        if (s_search[0] == '\0') return true;
        const std::string needle = Lower(s_search);
        if (Lower(mod.title).find(needle) != std::string::npos) return true;
        for (const auto& mf : mod.mapFiles) {
            if (Lower(mf).find(needle) != std::string::npos) return true;
        }
        return false;
    };

    std::vector<int> shown;
    shown.reserve(m_mods.size());
    for (int i = 0; i < static_cast<int>(m_mods.size()); ++i) {
        if (matches(m_mods[i])) shown.push_back(i);
    }

    // Controller navigation: D-pad / Stick picks a mod, X refreshes, Y opens mod folder.
    bool scrollToSelected = false;
    const bool inputFree = !ImGui::GetIO().WantTextInput;
    if (inputFree) {
        if (UiPad::ActionX()) {
            RefreshMods(m_workshopPath, m_localModsPath);
            MenuSounds::Click();
        }
        if (!shown.empty()) {
            const int count = static_cast<int>(shown.size());
            int pos = -1;
            for (int k = 0; k < count; ++k) {
                if (shown[k] == m_selectedModIndex) { pos = k; break; }
            }
            int next = (pos >= 0) ? pos : 0;
            if (UiPad::Down() || ImGui::IsKeyPressed(ImGuiKey_DownArrow, true)) next = (next + 1) % count;
            if (UiPad::Up() || ImGui::IsKeyPressed(ImGuiKey_UpArrow, true)) next = (next - 1 + count) % count;
            if (UiPad::PageDown() || ImGui::IsKeyPressed(ImGuiKey_PageDown)) next = std::min(next + 6, count - 1);
            if (UiPad::PageUp() || ImGui::IsKeyPressed(ImGuiKey_PageUp)) next = std::max(next - 6, 0);
            if (next != pos) {
                m_selectedModIndex = shown[next];
                scrollToSelected = (pos >= 0);
                if (pos >= 0) MenuSounds::Scroll();
            }
            if ((UiPad::ActionY() || UiPad::AcceptPressed()) && m_selectedModIndex >= 0 &&
                m_selectedModIndex < static_cast<int>(m_mods.size())) {
                if (UiPad::AcceptPressed()) UiPad::ConsumeAccept();
                ShellExecuteW(nullptr, L"open", m_mods[m_selectedModIndex].folderPath.wstring().c_str(), nullptr, nullptr, SW_SHOWNORMAL);
                MenuSounds::Click();
            }
        }
    }

    ImGui::Dummy(ImVec2(0.0f, U(6.0f)));
    const float bodyH = std::max(U(180.0f), ImGui::GetContentRegionAvail().y - U(4.0f));
    const float detailsW = std::clamp(fullW * 0.36f, U(300.0f), U(450.0f));
    const float gap = U(12.0f);
    const float listW = std::max(U(300.0f), fullW - detailsW - gap);

    ImGui::PushStyleColor(ImGuiCol_ChildBg, IM_COL32(8, 14, 22, 210));
    ImGui::PushStyleColor(ImGuiCol_Border, IM_COL32(45, 75, 102, 120));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(U(10.0f), U(10.0f)));

    // Left: Tactical Mod Card List
    ImGui::BeginChild("##modsCardList", ImVec2(listW, bodyH), ImGuiChildFlags_Borders | ImGuiChildFlags_AlwaysUseWindowPadding);
    UiPad::ScrollWindow(false, true);
    ImDrawList* ldl = ImGui::GetWindowDrawList();
    const float innerListW = ImGui::GetContentRegionAvail().x;
    if (shown.empty()) {
        const float emptyH = U(110.0f);
        const ImVec2 p0 = ImGui::GetCursorScreenPos();
        const ImVec2 p1(p0.x + innerListW, p0.y + emptyH);
        ldl->AddRectFilled(p0, p1, IM_COL32(12, 22, 34, 200), 2.0f * sScale);
        NobleUi::DrawTacticalFrame(ldl, p0, p1, sScale, false, false, false, false);
        const char* msg = m_mods.empty()
            ? "No Halo: Reach mods found. Subscribe on the Steam Workshop or place a mod folder in data\\mods."
            : "No installed mods match your search filter.";
        ldl->AddText(bodyFont, 14.5f * sScale, ImVec2(p0.x + U(16), p0.y + U(24)),
                     IM_COL32(180, 202, 222, 225), msg, nullptr, innerListW - U(32));
        ImGui::Dummy(ImVec2(innerListW, emptyH));
    } else {
        const float cardH = U(72.0f);
        const float pad = U(6.0f);
        const float thumbH = cardH - pad * 2.0f;
        const float thumbW = thumbH * (16.0f / 9.0f);
        for (int idx : shown) {
            const ModItem& mod = m_mods[idx];
            const bool selected = (m_selectedModIndex == idx);
            const ImVec2 c0 = ImGui::GetCursorScreenPos();
            const ImVec2 c1(c0.x + innerListW, c0.y + cardH);

            ImGui::PushID(idx);
            if (ImGui::InvisibleButton("##modCard", ImVec2(innerListW, cardH))) {
                m_selectedModIndex = idx;
                MenuSounds::Click();
            }
            const bool hov = ImGui::IsItemHovered();
            if (hov && ImGui::IsMouseDoubleClicked(0)) {
                ShellExecuteW(nullptr, L"open", mod.folderPath.wstring().c_str(), nullptr, nullptr, SW_SHOWNORMAL);
            }
            ImGui::PopID();

            if (scrollToSelected && selected) ImGui::SetScrollHereY(0.45f);

            const ImU32 bg = selected ? IM_COL32(20, 44, 66, 235) : hov ? IM_COL32(16, 33, 50, 220) : IM_COL32(11, 20, 31, 205);
            ldl->AddRectFilled(c0, c1, bg, 2.0f * sScale);

            const ImVec2 t0(c0.x + pad, c0.y + pad);
            const ImVec2 t1(t0.x + thumbW, t0.y + thumbH);
            ldl->AddRectFilled(t0, t1, IM_COL32(7, 12, 18, 255));
            UiTexture tex;
            if (!mod.previewImage.empty()) tex = UiArt::ImageFile(mod.previewImage);
            if (!tex && !mod.mapFiles.empty()) {
                const std::string stem = std::filesystem::path(mod.mapFiles.front()).stem().string();
                tex = UiArt::MapPreview(stem, true);
                if (!tex) tex = UiArt::MapPhoto(stem);
            }
            if (tex) {
                UiArt::DrawCoverCrisp(ldl, tex, t0, t1, IM_COL32(255, 255, 255, 255), 1.0f);
            } else {
                const char* tag = mod.isFromWorkshop ? "STEAM" : "LOCAL";
                const ImVec2 tsz = bodyFont->CalcTextSizeA(11.5f * sScale, FLT_MAX, 0.0f, tag);
                ldl->AddText(bodyFont, 11.5f * sScale, ImVec2(t0.x + (thumbW - tsz.x) * 0.5f, t0.y + (thumbH - tsz.y) * 0.5f),
                             IM_COL32(120, 148, 175, 200), tag);
            }
            ldl->AddRect(t0, t1, IM_COL32(90, 140, 180, 120), 0.0f, 0, 1.0f);

            const float tx = t1.x + U(14.0f);
            const float rightMetaW = U(125.0f);
            const float nameMaxW = std::max(U(100.0f), c1.x - tx - rightMetaW - U(10.0f));
            const std::string titleClip = EllipsizeModText(titleFont, 17.5f * sScale, mod.title, nameMaxW);
            ldl->AddText(titleFont, 17.5f * sScale, ImVec2(tx, c0.y + U(12.0f)),
                         selected ? IM_COL32(255, 250, 238, 255) : IM_COL32(228, 238, 248, 255), titleClip.c_str());

            // Source pill + map count + size
            const char* srcLabel = mod.isFromWorkshop ? "STEAM WORKSHOP" : "LOCAL MOD";
            const ImU32 srcBg = mod.isFromWorkshop ? IM_COL32(45, 130, 185, 230) : IM_COL32(60, 78, 96, 230);
            const ImVec2 ssz = bodyFont->CalcTextSizeA(11.5f * sScale, FLT_MAX, 0.0f, srcLabel);
            const float sy = c0.y + U(40.0f);
            ldl->AddRectFilled(ImVec2(tx, sy), ImVec2(tx + ssz.x + U(12.0f), sy + ssz.y + U(4.0f)), srcBg, 2.0f * sScale);
            ldl->AddText(bodyFont, 11.5f * sScale, ImVec2(tx + U(6.0f), sy + U(2.0f)), IM_COL32(242, 248, 255, 255), srcLabel);

            char metaBuf[64];
            std::snprintf(metaBuf, sizeof(metaBuf), "%zu map%s   |   %s",
                          mod.mapFiles.size(), mod.mapFiles.size() == 1 ? "" : "s", FormatSize(mod.sizeBytes).c_str());
            ldl->AddText(bodyFont, 13.0f * sScale, ImVec2(tx + ssz.x + U(22.0f), sy + U(1.5f)),
                         IM_COL32(162, 188, 210, 225), metaBuf);

            NobleUi::DrawTacticalFrame(ldl, c0, c1, sScale, selected, selected, hov, selected);
            ImGui::Dummy(ImVec2(innerListW, U(6.0f)));
        }
    }
    ImGui::EndChild();
    NobleUi::DrawTacticalFrame(dl, ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), sScale, true, true, false, false);

    ImGui::SameLine(0.0f, gap);

    // Right: Mod Details Pane
    ImGui::BeginChild("##modDetailPane", ImVec2(detailsW, bodyH), ImGuiChildFlags_Borders | ImGuiChildFlags_AlwaysUseWindowPadding);
    UiPad::ScrollWindow(false, true);
    ImDrawList* ddl = ImGui::GetWindowDrawList();
    const float detW = ImGui::GetContentRegionAvail().x;
    const ImVec2 dh0 = ImGui::GetCursorScreenPos();
    ddl->AddText(titleFont, 18.5f * sScale, dh0, IM_COL32(236, 244, 252, 255), "MOD DETAILS");
    ddl->AddLine(ImVec2(dh0.x, dh0.y + U(25.0f)), ImVec2(dh0.x + detW, dh0.y + U(25.0f)),
                 IM_COL32(78, 138, 185, 130), 1.0f * sScale);
    ImGui::Dummy(ImVec2(detW, U(30.0f)));

    if (m_selectedModIndex >= 0 && m_selectedModIndex < static_cast<int>(m_mods.size())) {
        const ModItem& mod = m_mods[m_selectedModIndex];
        UiTexture detTex;
        if (!mod.previewImage.empty()) detTex = UiArt::ImageFile(mod.previewImage);
        if (!detTex && !mod.mapFiles.empty()) {
            const std::string stem = std::filesystem::path(mod.mapFiles.front()).stem().string();
            detTex = UiArt::MapPhoto(stem);
            if (!detTex) detTex = UiArt::MapPreview(stem, true);
        }
        if (detTex) {
            const float ph = std::min(detW * 9.0f / 16.0f, U(190.0f));
            const ImVec2 p0 = ImGui::GetCursorScreenPos();
            const ImVec2 p1(p0.x + detW, p0.y + ph);
            ddl->AddRectFilled(p0, p1, IM_COL32(8, 13, 20, 255));
            UiArt::DrawCoverCrisp(ddl, detTex, p0, p1, IM_COL32(255, 255, 255, 255), 1.0f);
            NobleUi::DrawTacticalFrame(ddl, p0, p1, sScale, false, true, false, false);
            ImGui::Dummy(ImVec2(detW, ph + U(8.0f)));
        }

        ImGui::PushFont(titleFont);
        ImGui::TextWrapped("%s", mod.title.c_str());
        ImGui::PopFont();
        ImGui::TextColored(NobleStyle::Accent(), "%s",
                           mod.isFromWorkshop ? ("Steam Workshop ID: " + mod.id).c_str() : mod.folderPath.string().c_str());
        ImGui::TextDisabled("Total Map Size: %s   |   Map Files: %zu", FormatSize(mod.sizeBytes).c_str(), mod.mapFiles.size());
        ImGui::Spacing();

        if (!mod.mapFiles.empty()) {
            ImGui::TextDisabled("INCLUDED MAP FILES:");
            for (const std::string& mf : mod.mapFiles) {
                ImGui::BulletText("%s (%s)", MapNames::Display(mf).c_str(), mf.c_str());
            }
            ImGui::Spacing();
        }

        ImGui::TextDisabled("DESCRIPTION:");
        ImGui::TextWrapped("%s", mod.description.empty() ? "No description provided in ModInfo.json." : mod.description.c_str());
        ImGui::Spacing();

        if (NobleStyle::PrimaryButton("OPEN MOD FOLDER IN EXPLORER", ImVec2(-1, U(34)))) {
            ShellExecuteW(nullptr, L"open", mod.folderPath.wstring().c_str(), nullptr, nullptr, SW_SHOWNORMAL);
            MenuSounds::Click();
        }
        if (mod.isFromWorkshop) {
            ImGui::Spacing();
            if (ImGui::Button("OPEN WORKSHOP ITEM PAGE", ImVec2(-1, U(30)))) {
                const std::string url = "https://steamcommunity.com/sharedfiles/filedetails/?id=" + mod.id;
                ShellExecuteA(nullptr, "open", url.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
                MenuSounds::Click();
            }
        }
    } else {
        ImGui::PushStyleColor(ImGuiCol_Text, NobleStyle::TextMuted());
        ImGui::TextWrapped("Select an installed mod pack to inspect its map files, disk usage, and description.");
        ImGui::PopStyleColor();
    }
    ImGui::EndChild();
    NobleUi::DrawTacticalFrame(dl, ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), sScale, false, m_selectedModIndex >= 0, false, false);

    ImGui::PopStyleVar();
    ImGui::PopStyleColor(2);
}

} // namespace noble
