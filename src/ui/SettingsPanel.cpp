#include "SettingsPanel.hpp"
#include "NobleStyle.hpp"
#include "MenuSounds.hpp"
#include "UiArt.hpp"
#include "engine/KeyBindings.hpp"
#include "network/MasterServerClient.hpp"

#include <windows.h>
#include <imgui.h>

#include <algorithm>
#include <cstdio>
#include <string>

namespace noble {

namespace {

struct Resolution {
    int w, h;
};
const Resolution kResolutions[] = {
    {1280, 720}, {1366, 768}, {1600, 900}, {1920, 1080}, {2560, 1080}, {2560, 1440}, {3440, 1440}, {3840, 2160},
};

// MCC's settings look: light-blue upper-case labels, values on the right between arrows.
const ImVec4 kLabel = ImVec4(0.42f, 0.74f, 0.93f, 1.0f);
const ImVec4 kSection = ImVec4(0.80f, 0.86f, 0.90f, 1.0f);

void Section(const char* text) {
    ImGui::Dummy(ImVec2(0, 6));
    ImGui::TextColored(kSection, "%s", text);
    const ImVec2 p = ImGui::GetCursorScreenPos();
    ImGui::GetWindowDrawList()->AddLine(p, ImVec2(p.x + std::min(560.0f, ImGui::GetContentRegionAvail().x), p.y),
                                        IM_COL32(255, 255, 255, 50));
    ImGui::Dummy(ImVec2(0, 4));
}

// Starts a settings row: label in the left column, then the cursor is placed in the value column.
void RowLabel(const char* label) {
    ImGui::AlignTextToFramePadding();
    ImGui::TextColored(kLabel, "%s", label);
    ImGui::SameLine(300.0f);
}

// ‹ VALUE › selector. Returns -1/+1 when the player changes it.
int Cycle(const char* id, const char* value, float width = 260.0f) {
    int change = 0;
    ImGui::PushID(id);
    if (ImGui::ArrowButton("##l", ImGuiDir_Left)) change = -1;
    ImGui::SameLine();
    const ImVec2 p = ImGui::GetCursorScreenPos();
    const float h = ImGui::GetFrameHeight();
    const ImVec2 ts = ImGui::CalcTextSize(value);
    ImGui::Dummy(ImVec2(width, h));
    ImGui::GetWindowDrawList()->AddText(ImVec2(p.x + (width - ts.x) * 0.5f, p.y + (h - ts.y) * 0.5f),
                                        IM_COL32(236, 240, 244, 255), value);
    ImGui::SameLine();
    if (ImGui::ArrowButton("##r", ImGuiDir_Right)) change = 1;
    ImGui::PopID();
    if (change) MenuSounds::Tab();
    return change;
}

std::string Upper(std::string s) {
    for (char& c : s) c = static_cast<char>(::toupper(static_cast<unsigned char>(c)));
    return s;
}

// Reach (Xbox 360) default controller layout, labelled on MCC's controller picture.
struct Callout {
    float bx, by;      // button position on the 404x268 picture (0..1)
    bool right;        // label on the right side
    const char* label;
};
const Callout kReachDefault[] = {
    { 0.26f, 0.02f, false, "Use armor ability (LB)" },
    { 0.24f, 0.00f, false, "Throw grenade (LT)" },
    { 0.24f, 0.31f, false, "Move / crouch (click)" },
    { 0.37f, 0.54f, false, "Night vision, team chat (D-pad)" },
    { 0.43f, 0.30f, false, "Multiplayer scoreboard (Back)" },
    { 0.57f, 0.30f, true, "Game menu (Start)" },
    { 0.62f, 0.52f, true, "Look / zoom (click)" },
    { 0.76f, 0.02f, true, "Melee (RB)" },
    { 0.77f, 0.00f, true, "Fire (RT)" },
    { 0.76f, 0.18f, true, "Swap weapons (Y)" },
    { 0.82f, 0.29f, true, "Switch grenades (B)" },
    { 0.70f, 0.29f, true, "Action / reload (X)" },
    { 0.76f, 0.39f, true, "Jump (A)" },
};

void DrawControllerDiagram() {
    const UiTexture pad = UiArt::ControllerImage();
    const float w = 404.0f, h = pad ? w * pad.height / pad.width : 268.0f;
    const ImVec2 origin = ImGui::GetCursorScreenPos();
    const float padX = origin.x + 230.0f, padY = origin.y + 40.0f;
    ImDrawList* dl = ImGui::GetWindowDrawList();
    if (pad) {
        dl->AddImage(pad.id, ImVec2(padX, padY), ImVec2(padX + w, padY + h));
    } else {
        dl->AddRect(ImVec2(padX, padY), ImVec2(padX + w, padY + h), IM_COL32(200, 210, 220, 120), 30.0f);
    }
    int leftRow = 0, rightRow = 0;
    for (const Callout& c : kReachDefault) {
        const ImVec2 btn(padX + c.bx * w, padY + c.by * h);
        const float rowY = origin.y + 20.0f + (c.right ? rightRow++ : leftRow++) * 30.0f;
        const ImVec2 textPos = c.right ? ImVec2(padX + w + 40.0f, rowY) : ImVec2(origin.x, rowY);
        const ImVec2 elbow = c.right ? ImVec2(padX + w + 30.0f, rowY + 8.0f) : ImVec2(padX - 20.0f, rowY + 8.0f);
        dl->AddLine(btn, ImVec2(elbow.x, btn.y), IM_COL32(200, 210, 220, 110));
        dl->AddLine(ImVec2(elbow.x, btn.y), elbow, IM_COL32(200, 210, 220, 110));
        dl->AddCircleFilled(btn, 2.5f, IM_COL32(143, 216, 255, 255));
        dl->AddText(textPos, IM_COL32(236, 240, 244, 255), c.label);
    }
    ImGui::Dummy(ImVec2(w + 460.0f, std::max(h + 60.0f, 430.0f)));
}

} // namespace

void SettingsPanel::Render() {
    if (!m_loaded) {
        m_edit = Settings::Get().Snapshot();
        std::snprintf(m_nameBuf, sizeof(m_nameBuf), "%s", m_edit.playerName.c_str());
        std::snprintf(m_masterBuf, sizeof(m_masterBuf), "%s", m_edit.masterServerUrl.c_str());
        m_loaded = true;
    }
    PollRebind();

    // MCC-style tab row: plain upper-case words, the selected one underlined.
    static const char* const kTabs[] = { "CONTROLS", "KEY BINDINGS", "VIDEO", "AUDIO", "GAMEPLAY", "NETWORK" };
    ImGui::Dummy(ImVec2(0, 2));
    for (int i = 0; i < 6; ++i) {
        if (i) ImGui::SameLine(0, 26.0f);
        const bool sel = m_tab == i;
        ImGui::PushStyleColor(ImGuiCol_Text, sel ? ImVec4(1, 1, 1, 1) : kLabel);
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1, 1, 1, 0.06f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(1, 1, 1, 0.10f));
        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0, 0, 0, 0));
        if (ImGui::Button(kTabs[i])) {
            if (m_tab != i) MenuSounds::Tab();
            m_tab = i;
        }
        ImGui::PopStyleColor(5);
        if (sel) {
            const ImVec2 a = ImGui::GetItemRectMin(), b = ImGui::GetItemRectMax();
            ImGui::GetWindowDrawList()->AddLine(ImVec2(a.x + 6, b.y), ImVec2(b.x - 6, b.y), IM_COL32(143, 216, 255, 255), 2.0f);
        }
    }
    // Q / E switch tabs like LB / RB in MCC.
    if (!ImGui::GetIO().WantTextInput && m_rebindAction < 0) {
        if (ImGui::IsKeyPressed(ImGuiKey_Q, false) || ImGui::IsKeyPressed(ImGuiKey_GamepadL1, false)) { m_tab = (m_tab + 5) % 6; MenuSounds::Tab(); }
        if (ImGui::IsKeyPressed(ImGuiKey_E, false) || ImGui::IsKeyPressed(ImGuiKey_GamepadR1, false)) { m_tab = (m_tab + 1) % 6; MenuSounds::Tab(); }
    }
    ImGui::Separator();

    ImGui::BeginChild("##settingsBody", ImVec2(0, ImGui::GetContentRegionAvail().y - 52.0f), ImGuiChildFlags_None);
    switch (m_tab) {
    case 0: RenderControls(); break;
    case 1: RenderKeyBindings(); break;
    case 2: RenderDisplay(); break;
    case 3: RenderAudio(); break;
    case 4: RenderGameplay(); break;
    default: RenderNetwork(); break;
    }
    ImGui::EndChild();

    ImGui::Separator();
    ImGui::BeginDisabled(!m_dirty);
    if (NobleStyle::PrimaryButton("APPLY", ImVec2(140, 34))) {
        m_edit.playerName = m_nameBuf;
        m_edit.masterServerUrl = m_masterBuf;
        Settings::Get().Update(m_edit);
        MasterServerClient::Get().SetUrl(m_edit.masterServerUrl);
        m_edit = Settings::Get().Snapshot();  // clamped values
        m_dirty = false;
        m_message = "Saved. Display size, controls and player name apply the next time a game starts.";
    }
    ImGui::SameLine();
    if (ImGui::Button("UNDO CHANGES", ImVec2(150, 34))) {
        m_loaded = false;
        m_dirty = false;
        m_rebindAction = -1;
        m_message = nullptr;
    }
    ImGui::EndDisabled();
    ImGui::SameLine();
    if (ImGui::Button("RESTORE DEFAULTS", ImVec2(180, 34))) {
        const std::string master = m_edit.masterServerUrl;
        const auto favourites = m_edit.favorites;
        const uint64_t skulls = m_edit.campaignSkulls;
        m_edit = SettingsData{};
        m_edit.masterServerUrl = master;      // keep network entries and skull choices
        m_edit.favorites = favourites;
        m_edit.campaignSkulls = skulls;
        std::snprintf(m_nameBuf, sizeof(m_nameBuf), "%s", m_edit.playerName.c_str());
        m_dirty = true;
    }
    if (m_message) {
        ImGui::SameLine();
        ImGui::TextColored(NobleStyle::Accent(), "%s", m_message);
    }
}

void SettingsPanel::RenderControls() {
    Section("KEYBOARD & MOUSE");
    RowLabel("LOOK SENSITIVITY");
    ImGui::SetNextItemWidth(260);
    if (ImGui::SliderFloat("##sens", &m_edit.mouseSensitivity, 0.1f, 5.0f, "%.2fx", ImGuiSliderFlags_Logarithmic)) m_dirty = true;
    RowLabel("LOOK INVERSION");
    if (Cycle("invert", m_edit.invertLook ? "INVERTED" : "NOT INVERTED")) {
        m_edit.invertLook = !m_edit.invertLook;
        m_dirty = true;
    }
    ImGui::TextDisabled("1.00x is Noble's default. The -mouse-scale command-line option still overrides this.");

    Section("GAMEPAD");
    ImGui::TextDisabled("Controller input isn't supported in Noble yet. This is Halo: Reach's default layout,");
    ImGui::TextDisabled("which Noble will use once it is.");
    ImGui::Dummy(ImVec2(0, 4));
    DrawControllerDiagram();
}

void SettingsPanel::PollRebind() {
    if (m_rebindAction < 0) return;
    // Wait a moment so the click that started the rebind isn't taken as the new binding.
    if (ImGui::GetTime() - m_rebindStarted < 0.25) return;
    if (GetAsyncKeyState(VK_ESCAPE) & 0x8000) {
        m_rebindAction = -1;
        return;
    }
    for (int vk = 1; vk < 256; ++vk) {
        if (vk == VK_ESCAPE || vk == VK_SHIFT || vk == VK_CONTROL || vk == VK_MENU) continue;  // use the L/R variants
        if (GetAsyncKeyState(vk) & 0x8000) {
            if (vk == KeyBindings::DefaultKey(m_rebindAction)) m_edit.keyOverrides.erase(m_rebindAction);
            else m_edit.keyOverrides[m_rebindAction] = vk;
            m_rebindAction = -1;
            m_dirty = true;
            MenuSounds::Click();
            return;
        }
    }
}

void SettingsPanel::RenderKeyBindings() {
    ImGui::Dummy(ImVec2(0, 2));
    ImGui::TextDisabled("Click a key to change it, then press a key or mouse button (Esc cancels). Right-click to unbind.");
    using KeyBindings::ActionGroup;
    const ActionGroup groups[] = { ActionGroup::Movement, ActionGroup::Actions, ActionGroup::Vehicle, ActionGroup::Interface,
                                   ActionGroup::Forge, ActionGroup::Theater };
    // ActionGroup::Other (bindings MCC shares with its other Halo games) is not listed: Reach doesn't
    // use them. They still get their default keys in the profile.
    for (ActionGroup g : groups) {
        const std::vector<int> actions = KeyBindings::ActionsIn(g);
        if (actions.empty()) continue;
        Section(KeyBindings::GroupName(g));
        for (int action : actions) {
            const int def = KeyBindings::DefaultKey(action);
            auto it = m_edit.keyOverrides.find(action);
            const int current = it != m_edit.keyOverrides.end() ? it->second : def;
            ImGui::PushID(action);
            const char* name = KeyBindings::ActionName(action);
            char fallback[32];
            std::snprintf(fallback, sizeof(fallback), "SLOT %d", action);
            RowLabel(name ? Upper(name).c_str() : fallback);
            std::string keyText = m_rebindAction == action ? std::string("PRESS A KEY...") : KeyBindings::KeyName(current);
            if (current == 0 && m_rebindAction != action) keyText = "UNBOUND";
            const bool changed = current != def;
            ImGui::PushStyleColor(ImGuiCol_Text, changed ? NobleStyle::Accent() : ImVec4(0.92f, 0.94f, 0.96f, 1.0f));
            if (ImGui::Button(keyText.c_str(), ImVec2(180, 0))) {
                m_rebindAction = action;
                m_rebindStarted = ImGui::GetTime();
            }
            ImGui::PopStyleColor();
            if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
                if (def == 0) m_edit.keyOverrides.erase(action);
                else m_edit.keyOverrides[action] = 0;
                m_dirty = true;
            }
            if (changed) {
                ImGui::SameLine();
                ImGui::TextDisabled("default: %s", KeyBindings::KeyName(def).c_str());
                ImGui::SameLine();
                if (ImGui::SmallButton("RESET")) {
                    m_edit.keyOverrides.erase(action);
                    m_dirty = true;
                }
            }
            ImGui::PopID();
        }
    }
}

void SettingsPanel::RenderAudio() {
    Section("VOLUME");
    auto volume = [&](const char* label, const char* id, float& v) {
        RowLabel(label);
        ImGui::SetNextItemWidth(260);
        float percent = v * 100.0f;
        if (ImGui::SliderFloat(id, &percent, 0.0f, 100.0f, "%.0f%%")) {
            v = percent / 100.0f;
            m_dirty = true;
        }
    };
    volume("MASTER / GAME", "##vgame", m_edit.volumeGame);
    volume("MUSIC", "##vmusic", m_edit.volumeMusic);
    volume("VOICE / OTHER", "##vother", m_edit.volumeOther);
    Section("MENUS");
    RowLabel("MENU SOUNDS");
    if (Cycle("menusounds", m_edit.menuSounds ? "ON" : "OFF")) {
        m_edit.menuSounds = !m_edit.menuSounds;
        m_dirty = true;
    }
}

void SettingsPanel::RenderDisplay() {
    Section("DISPLAY");
    RowLabel("RESOLUTION");
    int current = 0;
    for (int i = 0; i < static_cast<int>(sizeof(kResolutions) / sizeof(kResolutions[0])); ++i) {
        if (kResolutions[i].w == m_edit.width && kResolutions[i].h == m_edit.height) current = i;
    }
    char label[32];
    std::snprintf(label, sizeof(label), "%d x %d", m_edit.width, m_edit.height);
    const int dr = Cycle("res", label);
    if (dr) {
        const int n = static_cast<int>(sizeof(kResolutions) / sizeof(kResolutions[0]));
        current = (current + dr + n) % n;
        m_edit.width = kResolutions[current].w;
        m_edit.height = kResolutions[current].h;
        m_dirty = true;
    }
    RowLabel("WINDOW MODE");
    if (Cycle("borderless", m_edit.borderless ? "BORDERLESS FULL SCREEN" : "WINDOWED")) {
        m_edit.borderless = !m_edit.borderless;
        m_dirty = true;
    }
    ImGui::TextDisabled("Borderless uses the monitor's size. Both apply the next time Noble starts.");
}

void SettingsPanel::RenderGameplay() {
    Section("GAMEPLAY");
    RowLabel("SUBTITLES");
    if (Cycle("subtitles", m_edit.subtitles ? "ON" : "OFF")) {
        m_edit.subtitles = !m_edit.subtitles;
        m_dirty = true;
    }
    Section("PLAYER");
    RowLabel("PLAYER NAME");
    ImGui::SetNextItemWidth(260);
    if (ImGui::InputText("##name", m_nameBuf, 16)) m_dirty = true;
    ImGui::TextDisabled("Shown in game, up to 15 characters.");
    Section("MENUS");
    RowLabel("MENU BACKGROUND");
    static const char* const kModes[] = { "random", "reach", "noble" };
    static const char* const kLabels[] = { "RANDOM", "HALO: REACH (MCC)", "NOBLE ART" };
    int mode = 0;
    for (int i = 0; i < 3; ++i) if (m_edit.menuBackground == kModes[i]) mode = i;
    const int dm = Cycle("menubg", kLabels[mode]);
    if (dm) {
        m_edit.menuBackground = kModes[(mode + dm + 3) % 3];
        m_dirty = true;
    }
    ImGui::TextDisabled("Your own pictures in data\\ui\\custom\\backgrounds are always used first.");
}

void SettingsPanel::RenderNetwork() {
    Section("NETWORK");
    RowLabel("MASTER SERVER");
    ImGui::SetNextItemWidth(360);
    if (ImGui::InputTextWithHint("##master", "http://host:8080 (empty = LAN only)", m_masterBuf, sizeof(m_masterBuf))) m_dirty = true;
    ImGui::TextDisabled("The list of online games for the GAMES screen. Leave empty to only see games on your LAN.");
}

} // namespace noble
