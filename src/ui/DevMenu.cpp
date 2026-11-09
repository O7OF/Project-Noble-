#include "DevMenu.hpp"

#if defined(NOBLE_DEV_BUILD)

#include "NobleStyle.hpp"
#include "engine/DevCheats.hpp"
#include "host/SpawnTest.hpp"
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
    ImGui::GetBackgroundDrawList()->AddRectFilled(ImVec2(0, 0), io.DisplaySize, IM_COL32(0, 0, 0, 110));

    const float s = std::clamp(io.DisplaySize.y / 720.0f, 0.8f, 2.0f);
    const ImVec2 size(780.0f * s, 560.0f * s);
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

    SetCursor(LoadCursorW(nullptr, IDC_ARROW));

    ImGui::PushFont(NobleStyle::TitleFont());
    ImGui::TextColored(NobleStyle::Accent(), "DEVELOPER CHEAT & MOD MENU");
    ImGui::PopFont();

    ImGui::SameLine();
    ImGui::TextColored(NobleStyle::TextMuted(), "   [+] Toggle  |  [ESC] Close");

    ImGui::Separator();
    ImGui::Spacing();

    if (ImGui::BeginTabBar("##DevTabs", ImGuiTabBarFlags_None)) {

        // TAB 1: PLAYER CHEATS
        if (ImGui::BeginTabItem("PLAYER CHEATS")) {
            ImGui::Spacing();

            // Godmode
            bool god = DevCheats::IsGodmode();
            if (ImGui::Checkbox("Godmode (Invulnerability + Instant Shields + Death Immunity)", &god)) {
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
            ImGui::TextColored(kDim, "Freecam Controls: W/A/S/D = Fly, Space = Ascend, Ctrl/C = Descend, Shift = Boost.");
            ImGui::TextColored(kGood, "Player character is frozen in place while Freecam is active so your Spartan does not walk away.");

            ImGui::EndTabItem();
        }

        // TAB 2: WEAPONS & AMMO
        if (ImGui::BeginTabItem("WEAPONS & AMMO")) {
            ImGui::Spacing();

            bool infAmmo = DevCheats::IsInfiniteAmmo();
            if (ImGui::Checkbox("Infinite Reserve Ammo (Locked at 999)", &infAmmo)) {
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
            ImGui::TextColored(kDim, "Armor abilities (Jetpack, Armor Lock, Sprint, Camo, etc.) never deplete.");

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            if (NobleStyle::PrimaryButton("REFILL AMMO & SHIELDS NOW", ImVec2(320.0f * s, 36.0f * s))) {
                uint32_t unitDatum = DevCheats::GetLocalPlayerUnitDatum();
                if (unitDatum != 0xFFFFFFFF) {
                    uint8_t* biped = DevCheats::GetObjectByDatum(unitDatum);
                    if (biped) {
                        for (int slot = 0; slot < 4; ++slot) {
                            uint32_t wDatum = *reinterpret_cast<const uint32_t*>(biped + 0x350 + slot * 4);
                            if (wDatum != 0xFFFFFFFF && wDatum != 0) {
                                uint8_t* weapon = DevCheats::GetObjectByDatum(wDatum);
                                if (weapon) {
                                    *reinterpret_cast<uint32_t*>(weapon + 0x3034) = 999;
                                    *reinterpret_cast<uint32_t*>(weapon + 0x3030) = 99;
                                    *reinterpret_cast<float*>(weapon + 0x3038) = 1.0f;
                                    *reinterpret_cast<float*>(weapon + 0x303C) = 0.0f;
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

            // Native Blam! AI and squad triggers (Tyler's F6 / F7 / F8 / F9 hooks)
            ImGui::TextColored(NobleStyle::Accent(), "NATIVE BLAM! AI & SQUAD SPAWNERS (F6 / F7 / F8 / F9):");
            if (NobleStyle::PrimaryButton("SPAWN AI HORDE (F6)", ImVec2(165.0f * s, 28.0f * s))) {
                noble::spawn::TriggerAiHorde(25);
                s_statusMessage = "F6: Queued 25 AI squad actors around you!";
                s_statusTime = GetTickCount64();
            }
            ImGui::SameLine();
            if (ImGui::Button("PLACE ALL SQUADS (F7)", ImVec2(175.0f * s, 28.0f * s))) {
                noble::spawn::TriggerPlaceAllSquads();
                s_statusMessage = "F7: Placed all scenario squads!";
                s_statusTime = GetTickCount64();
            }
            ImGui::SameLine();
            if (ImGui::Button("SPARTAN CROWD (F8)", ImVec2(165.0f * s, 28.0f * s))) {
                noble::spawn::TriggerCrowdToggle();
                s_statusMessage = "F8: Toggled 128 Spartan crowd!";
                s_statusTime = GetTickCount64();
            }
            ImGui::SameLine();
            if (ImGui::Button("STRESS TEST (F9)", ImVec2(140.0f * s, 28.0f * s))) {
                noble::spawn::TriggerSpawnObjects(500);
                s_statusMessage = "F9: Queued 500 objects!";
                s_statusTime = GetTickCount64();
            }

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            ImGui::SetNextItemWidth(320.0f * s);
            ImGui::InputTextWithHint("##SpawnerSearch", "Search weapons, equipment, vehicles...", s_searchQuery, sizeof(s_searchQuery));
            ImGui::SameLine();
            if (ImGui::Button("Clear")) {
                s_searchQuery[0] = '\0';
            }

            ImGui::Spacing();

            const char* categories[] = { "ALL", "UNSC WEAPONS", "COVENANT WEAPONS", "ARMOR ABILITIES", "VEHICLES", "ACTIVE MAP ITEMS" };
            for (int i = 0; i < 6; ++i) {
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

            ImGui::BeginChild("##SpawnerList", ImVec2(0, -38.0f * s), true);

            std::vector<SpawnCatalogItem> itemsToDisplay;
            if (s_categoryFilter == 5) {
                itemsToDisplay = DevCheats::GetActiveMapItems();
                if (itemsToDisplay.empty()) {
                    ImGui::TextColored(kDim, "No active map entities indexed yet (start or join a match).");
                }
            } else {
                itemsToDisplay = DevCheats::GetCatalog();
            }

            const std::string query(s_searchQuery);

            for (size_t i = 0; i < itemsToDisplay.size(); ++i) {
                const auto& item = itemsToDisplay[i];

                if (s_categoryFilter == 1 && item.category != "UNSC Weapons") continue;
                if (s_categoryFilter == 2 && item.category != "Covenant Weapons") continue;
                if (s_categoryFilter == 3 && item.category != "Armor Abilities") continue;
                if (s_categoryFilter == 4 && item.category != "Vehicles") continue;

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
                ImGui::SameLine(220.0f * s);
                ImGui::TextColored(kDim, "[%s]", item.category.c_str());
                ImGui::TextColored(kBody, "%s", item.description.c_str());
                ImGui::EndGroup();

                const bool canGive = (item.category != "Vehicles");
                const float buttonW = canGive ? 90.0f * s : 140.0f * s;
                const float groupW = canGive ? (190.0f * s) : (145.0f * s);

                ImGui::SameLine(ImGui::GetWindowWidth() - groupW);

                if (canGive) {
                    if (NobleStyle::PrimaryButton("GIVE", ImVec2(buttonW, 26.0f * s))) {
                        DevCheats::QueueItemAction(item.name, item.category, item.tagPath, item.tagIndex, true /* give */);
                        s_statusMessage = "Gave " + item.name + " directly to player!";
                        s_statusTime = GetTickCount64();
                    }
                    ImGui::SameLine();
                }

                if (ImGui::Button("SPAWN IN FRONT", ImVec2(canGive ? 90.0f * s : 140.0f * s, 26.0f * s))) {
                    DevCheats::QueueItemAction(item.name, item.category, item.tagPath, item.tagIndex, false /* spawn */);
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

    std::string lastStat = DevCheats::GetLastStatus();
    if (!lastStat.empty()) {
        s_statusMessage = lastStat;
        s_statusTime = GetTickCount64();
    }

    if (s_statusTime != 0 && (GetTickCount64() - s_statusTime < 4500)) {
        ImGui::Spacing();
        ImGui::TextColored(kGood, ">> %s", s_statusMessage.c_str());
    }

    ImGui::End();
    ImGui::PopStyleVar(3);
    ImGui::PopStyleColor(2);
}

} // namespace noble

#endif // defined(NOBLE_DEV_BUILD)
