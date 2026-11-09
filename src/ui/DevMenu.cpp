#include "DevMenu.hpp"
#include "NobleStyle.hpp"
#include "engine/DevCheats.hpp"
#include "common/Logger.hpp"

#include <imgui.h>
#include <algorithm>
#include <cctype>
#include <string>

namespace noble {

bool DevMenu::s_open = false;
int DevMenu::s_categoryFilter = 0;
char DevMenu::s_searchQuery[64] = "";
std::string DevMenu::s_statusMessage = "";
unsigned long long DevMenu::s_statusTime = 0;

namespace {
constexpr ImGuiWindowFlags kPanelFlags =
    ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings;

const ImVec4 kBody = ImVec4(0.86f, 0.90f, 0.94f, 1.0f);
const ImVec4 kDim  = ImVec4(0.55f, 0.62f, 0.70f, 1.0f);
const ImVec4 kGood = ImVec4(0.35f, 0.85f, 0.45f, 1.0f);

bool CaseInsensitiveContains(const std::string& str, const std::string& query) {
    if (query.empty()) return true;
    auto it = std::search(
        str.begin(), str.end(),
        query.begin(), query.end(),
        [](char ch1, char ch2) { return std::tolower(static_cast<unsigned char>(ch1)) == std::tolower(static_cast<unsigned char>(ch2)); }
    );
    return (it != str.end());
}
} // namespace

bool DevMenu::IsOpen() {
    return s_open;
}

void DevMenu::Toggle() {
    s_open = !s_open;
    NOBLE_INFO("DevMenu: {} (key: +)", s_open ? "opened" : "closed");
}

void DevMenu::Open() {
    s_open = true;
}

void DevMenu::Close() {
    s_open = false;
}

bool DevMenu::WantsPointer() {
    return s_open;
}

void DevMenu::Draw() {
    if (!s_open) return;

    ImGuiIO& io = ImGui::GetIO();
    // Dim background slightly behind developer menu
    ImGui::GetBackgroundDrawList()->AddRectFilled(ImVec2(0, 0), io.DisplaySize, IM_COL32(0, 0, 0, 110));

    const float s = std::clamp(io.DisplaySize.y / 720.0f, 0.8f, 2.0f);
    const ImVec2 size(740.0f * s, 540.0f * s);
    const ImVec2 pos((io.DisplaySize.x - size.x) * 0.5f, (io.DisplaySize.y - size.y) * 0.5f);

    ImGui::SetNextWindowSize(size, ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowPos(pos, ImGuiCond_FirstUseEver);

    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.035f, 0.055f, 0.080f, 0.96f));
    ImGui::PushStyleColor(ImGuiCol_Border, NobleStyle::Accent());
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(16.0f * s, 14.0f * s));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 2.0f);

    if (!ImGui::Begin("DEVELOPER CHEAT & MOD MENU ##NobleDevMenu", &s_open, kPanelFlags)) {
        ImGui::End();
        ImGui::PopStyleVar(3);
        ImGui::PopStyleColor(2);
        return;
    }

    // Keep the Windows arrow visible while menu has pointer
    SetCursor(LoadCursorW(nullptr, IDC_ARROW));

    // Header info
    ImGui::PushFont(NobleStyle::TitleFont());
    ImGui::TextColored(NobleStyle::Accent(), "DEVELOPER CHEAT & MOD MENU");
    ImGui::PopFont();

    ImGui::SameLine();
    ImGui::TextColored(NobleStyle::TextMuted(), "   [+] Toggle  |  [ESC] Close");

    ImGui::Separator();
    ImGui::Spacing();

    // Tab Bar
    if (ImGui::BeginTabBar("##DevTabs", ImGuiTabBarFlags_None)) {

        // TAB 1: PLAYER CHEATS
        if (ImGui::BeginTabItem("PLAYER CHEATS")) {
            ImGui::Spacing();

            // Godmode
            bool god = DevCheats::IsGodmode();
            if (ImGui::Checkbox("Godmode (Invulnerability + Instant Shield Regen + Death Immunity)", &god)) {
                DevCheats::SetGodmode(god);
                s_statusMessage = god ? "Godmode: ENABLED" : "Godmode: DISABLED";
                s_statusTime = GetTickCount64();
            }
            ImGui::TextColored(kDim, "Blocks all health/shield damage and prevents death in all gamemodes.");

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            // Super Jump
            bool sjump = DevCheats::IsSuperJump();
            if (ImGui::Checkbox("Super Jump", &sjump)) {
                DevCheats::SetSuperJump(sjump);
            }
            ImGui::SameLine(220.0f * s);
            float jm = DevCheats::GetJumpMultiplier();
            ImGui::SetNextItemWidth(260.0f * s);
            if (ImGui::SliderFloat("##JumpMult", &jm, 1.0f, 10.0f, "Jump Multiplier: %.1fx")) {
                DevCheats::SetJumpMultiplier(jm);
            }

            ImGui::Spacing();

            // Super Speed
            bool sspeed = DevCheats::IsSuperSpeed();
            if (ImGui::Checkbox("Super Speed", &sspeed)) {
                DevCheats::SetSuperSpeed(sspeed);
            }
            ImGui::SameLine(220.0f * s);
            float sm = DevCheats::GetSpeedMultiplier();
            ImGui::SetNextItemWidth(260.0f * s);
            if (ImGui::SliderFloat("##SpeedMult", &sm, 1.0f, 8.0f, "Speed Multiplier: %.1fx")) {
                DevCheats::SetSpeedMultiplier(sm);
            }

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            // Noclip / Ghost Fly
            bool noclip = DevCheats::IsNoclip();
            if (ImGui::Checkbox("Noclip (Ghost Flight)", &noclip)) {
                DevCheats::SetNoclip(noclip);
                s_statusMessage = noclip ? "Noclip Flight: ENABLED" : "Noclip Flight: DISABLED";
                s_statusTime = GetTickCount64();
            }
            ImGui::SameLine(220.0f * s);
            float ncs = DevCheats::GetNoclipSpeed();
            ImGui::SetNextItemWidth(260.0f * s);
            if (ImGui::SliderFloat("##NoclipSpeed", &ncs, 5.0f, 50.0f, "Flight Speed: %.0f m/s")) {
                DevCheats::SetNoclipSpeed(ncs);
            }
            ImGui::TextColored(kDim, "Noclip Controls: WASD to fly, Space = Ascend, Ctrl/C = Descend, Shift = Boost");

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            // Freecam / Spectator Camera
            bool freecam = DevCheats::IsFreecam();
            if (ImGui::Checkbox("Freecam (Detached Camera)", &freecam)) {
                DevCheats::SetFreecam(freecam);
                s_statusMessage = freecam ? "Freecam: ENABLED" : "Freecam: DISABLED";
                s_statusTime = GetTickCount64();
            }
            ImGui::SameLine(220.0f * s);
            float fcs = DevCheats::GetFreecamSpeed();
            ImGui::SetNextItemWidth(260.0f * s);
            if (ImGui::SliderFloat("##FreecamSpeed", &fcs, 5.0f, 50.0f, "Camera Speed: %.0f m/s")) {
                DevCheats::SetFreecamSpeed(fcs);
            }
            ImGui::TextColored(kDim, "Freecam Controls: Arrow Keys to move, E = Ascend, Q = Descend, Shift = Boost");

            ImGui::EndTabItem();
        }

        // TAB 2: WEAPONS & AMMO
        if (ImGui::BeginTabItem("WEAPONS & AMMO")) {
            ImGui::Spacing();

            bool infAmmo = DevCheats::IsInfiniteAmmo();
            if (ImGui::Checkbox("Infinite Reserve Ammo (Always 999 Rounds)", &infAmmo)) {
                DevCheats::SetInfiniteAmmo(infAmmo);
                s_statusMessage = infAmmo ? "Infinite Ammo: ENABLED" : "Infinite Ammo: DISABLED";
                s_statusTime = GetTickCount64();
            }
            ImGui::TextColored(kDim, "Keeps reserves for all equipped primary and secondary weapons fully replenished.");

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            bool bottomless = DevCheats::IsBottomlessClip();
            if (ImGui::Checkbox("Bottomless Clip (Never Reload)", &bottomless)) {
                DevCheats::SetBottomlessClip(bottomless);
                s_statusMessage = bottomless ? "Bottomless Clip: ENABLED" : "Bottomless Clip: DISABLED";
                s_statusTime = GetTickCount64();
            }
            ImGui::TextColored(kDim, "Magazines stay locked at maximum capacity while firing.");

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            bool infEquip = DevCheats::IsInfiniteEquipment();
            if (ImGui::Checkbox("Infinite Equipment / Armor Ability Energy", &infEquip)) {
                DevCheats::SetInfiniteEquipment(infEquip);
                s_statusMessage = infEquip ? "Infinite Equipment: ENABLED" : "Infinite Equipment: DISABLED";
                s_statusTime = GetTickCount64();
            }
            ImGui::TextColored(kDim, "Armor abilities (Jetpack, Armor Lock, Sprint, Camo) can be used indefinitely.");

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            if (NobleStyle::PrimaryButton("REFILL AMMO & SHIELDS NOW", ImVec2(320.0f * s, 36.0f * s))) {
                // Instantly refill carried weapons
                uint32_t unitDatum = DevCheats::GetLocalPlayerUnitDatum();
                if (unitDatum != 0xFFFFFFFF) {
                    uint8_t* biped = DevCheats::GetObjectByDatum(unitDatum);
                    if (biped) {
                        for (int slot = 0; slot < 4; ++slot) {
                            uint32_t weaponDatum = *reinterpret_cast<const uint32_t*>(biped + 0x350 + slot * 4);
                            if (weaponDatum != 0xFFFFFFFF && weaponDatum != 0) {
                                uint8_t* weapon = DevCheats::GetObjectByDatum(weaponDatum);
                                if (weapon) {
                                    *reinterpret_cast<uint32_t*>(weapon + 0x3034) = 999;
                                    *reinterpret_cast<uint32_t*>(weapon + 0x3030) = 99;
                                }
                            }
                        }
                    }
                }
                s_statusMessage = "Refilled all carried weapons and restored shields!";
                s_statusTime = GetTickCount64();
            }

            ImGui::EndTabItem();
        }

        // TAB 3: ITEM & VEHICLE SPAWNER
        if (ImGui::BeginTabItem("ITEM & VEHICLE SPAWNER")) {
            ImGui::Spacing();

            // Search filter
            ImGui::SetNextItemWidth(320.0f * s);
            ImGui::InputTextWithHint("##SpawnerSearch", "Search weapons, equipment, vehicles...", s_searchQuery, sizeof(s_searchQuery));
            ImGui::SameLine();
            if (ImGui::Button("Clear")) {
                s_searchQuery[0] = '\0';
            }

            ImGui::Spacing();

            // Category filter pills
            const char* categories[] = { "ALL", "UNSC WEAPONS", "COVENANT WEAPONS", "ARMOR ABILITIES", "VEHICLES" };
            for (int i = 0; i < 5; ++i) {
                if (i > 0) ImGui::SameLine();
                const bool selected = (s_categoryFilter == i);
                if (selected) {
                    ImGui::PushStyleColor(ImGuiCol_Button, NobleStyle::Accent());
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.05f, 0.08f, 0.12f, 1.0f));
                } else {
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.10f, 0.15f, 0.22f, 0.7f));
                    ImGui::PushStyleColor(ImGuiCol_Text, kDim);
                }
                if (ImGui::Button(categories[i])) {
                    s_categoryFilter = i;
                }
                ImGui::PopStyleColor(2);
            }

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            // Item list table inside scrollable child window
            ImGui::BeginChild("##SpawnerList", ImVec2(0, -38.0f * s), true);

            const auto& catalog = DevCheats::GetCatalog();
            const std::string query(s_searchQuery);

            for (size_t i = 0; i < catalog.size(); ++i) {
                const auto& item = catalog[i];

                // Category filtering
                if (s_categoryFilter == 1 && item.category != "UNSC Weapons") continue;
                if (s_categoryFilter == 2 && item.category != "Covenant Weapons") continue;
                if (s_categoryFilter == 3 && item.category != "Armor Abilities") continue;
                if (s_categoryFilter == 4 && item.category != "Vehicles") continue;

                // Search query filtering
                if (!query.empty()) {
                    if (!CaseInsensitiveContains(item.name, query) &&
                        !CaseInsensitiveContains(item.category, query) &&
                        !CaseInsensitiveContains(item.description, query)) {
                        continue;
                    }
                }

                ImGui::PushID(static_cast<int>(i));

                ImGui::BeginGroup();
                ImGui::TextColored(NobleStyle::Accent(), "%s", item.name.c_str());
                ImGui::SameLine(180.0f * s);
                ImGui::TextColored(kDim, "[%s]", item.category.c_str());
                ImGui::TextColored(kBody, "%s", item.description.c_str());
                ImGui::EndGroup();

                ImGui::SameLine(ImGui::GetWindowWidth() - 150.0f * s);
                if (NobleStyle::PrimaryButton("SPAWN IN FRONT", ImVec2(135.0f * s, 26.0f * s))) {
                    DevCheats::SpawnItem(item.tagPath);
                    s_statusMessage = "Spawned " + item.name + " in front of player!";
                    s_statusTime = GetTickCount64();
                }

                ImGui::Separator();
                ImGui::PopID();
            }

            ImGui::EndChild();
            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }

    // Status message at bottom of window
    if (s_statusTime != 0 && (GetTickCount64() - s_statusTime < 4500)) {
        ImGui::Spacing();
        ImGui::TextColored(kGood, ">> %s", s_statusMessage.c_str());
    }

    ImGui::End();
    ImGui::PopStyleVar(3);
    ImGui::PopStyleColor(2);
}

} // namespace noble
