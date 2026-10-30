#include "SettingsPanel.hpp"
#include "NobleStyle.hpp"
#include "NobleUi.hpp"
#include "MenuSounds.hpp"
#include "audio/ReachAudio.hpp"
#include "UiArt.hpp"
#include "common/Version.hpp"
#include "InputPrompts.hpp"
#include "engine/KeyBindings.hpp"
#include "engine/CameraFov.hpp"
#include "engine/PadLayouts.hpp"
#include "network/MasterServerClient.hpp"

#include <windows.h>
#include <commdlg.h>
#include <shellapi.h>   // GetOpenFileNameW for IMPORT BACKGROUND
#include <dxgi.h>
#include <imgui.h>

#include <algorithm>
#include <cfloat>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iterator>
#include <string>
#include <fstream>
#include <vector>

namespace noble {

namespace {

using namespace NobleUi;

struct Resolution {
    int w, h;
};
const Resolution kResolutions[] = {
    {1280, 720}, {1366, 768}, {1600, 900}, {1920, 1080}, {2560, 1080}, {2560, 1440}, {3440, 1440}, {3840, 2160},
};
constexpr int kFpsLimits[] = { 30, 60, 120, 144, 165, 200, 0 };
const int kWheelActions[] = { 4, 1, 3, 5, 7, 2, 23, 6, 11, 12, 20, -1 };

std::string Upper(std::string s) {
    for (char& c : s) c = static_cast<char>(::toupper(static_cast<unsigned char>(c)));
    return s;
}
std::string Lower(std::string s) {
    for (char& c : s) c = static_cast<char>(::tolower(static_cast<unsigned char>(c)));
    return s;
}
std::string Format(const char* fmt, double v) {
    char buf[64];
    std::snprintf(buf, sizeof(buf), fmt, v);
    return buf;
}
std::string Percent(float v) { return Format("%.0f %%", v * 100.0f); }

struct CatInfo {
    const char* group;   // printed above the first category of a group
    const char* name;
    const char* footer;  // when this category's changes take effect
};
const CatInfo kCats[] = {
    { "DISPLAY & SOUND", "GRAPHICS", "Window size and display mode take effect the next time Noble starts; everything else when you apply changes." },
    { nullptr, "AUDIO", "Volumes take effect when you apply changes; a menu song plays as soon as you pick it." },
    { "GAME", "GAME OPTIONS", "Takes effect when you apply changes, also in a game you return to from the pause menu." },
    { nullptr, "PLAYER", "Saved when you apply changes; shown from the next game you start." },
    { "CONTROLS", "KEYBOARD & MOUSE", "Takes effect when you apply changes, also mid-game from the pause menu." },
    { nullptr, "CONTROLLER", "Takes effect when you apply changes, also mid-game from the pause menu." },
    { "ONLINE", "ONLINE SERVICES", "Saved when you apply changes." },
    { "SYSTEM", "SYSTEM", "Diagnostics and backups. These act straight away." },
};
static_assert(std::size(kCats) == 8);

const char* WhenText(int when) {
    switch (when) {
    case 0: return "Takes effect when you apply changes";
    case 1: return "Saved when you apply changes; takes effect the next time Noble starts";
    case 2: return "Saved and used as soon as you change it";
    default: return "Information";
    }
}

// The graphics card Windows uses for Noble (adapter 0), read once.
std::string GpuLine() {
    static std::string s_line;
    static bool s_done = false;
    if (s_done) return s_line;
    s_done = true;
    IDXGIFactory1* factory = nullptr;
    if (SUCCEEDED(CreateDXGIFactory1(__uuidof(IDXGIFactory1), reinterpret_cast<void**>(&factory))) && factory) {
        IDXGIAdapter1* adapter = nullptr;
        if (SUCCEEDED(factory->EnumAdapters1(0, &adapter)) && adapter) {
            DXGI_ADAPTER_DESC1 desc{};
            if (SUCCEEDED(adapter->GetDesc1(&desc))) {
                char name[128] = {};
                WideCharToMultiByte(CP_UTF8, 0, desc.Description, -1, name, sizeof(name), nullptr, nullptr);
                char buf[200];
                std::snprintf(buf, sizeof(buf), "%s (%.1f GB)", name, desc.DedicatedVideoMemory / (1024.0 * 1024.0 * 1024.0));
                s_line = buf;
            }
            adapter->Release();
        }
        factory->Release();
    }
    if (s_line.empty()) s_line = "Unknown";
    return s_line;
}

// First connected XInput controller (0..3), or -1. Polled at most once a second.
int ConnectedPad() {
    struct State { DWORD packet; WORD buttons; BYTE lt, rt; SHORT lx, ly, rx, ry; };
    using GetStateFn = DWORD(WINAPI*)(DWORD, State*);
    static GetStateFn s_get = [] {
        HMODULE m = LoadLibraryW(L"xinput1_4.dll");
        if (!m) m = LoadLibraryW(L"xinput9_1_0.dll");
        return m ? reinterpret_cast<GetStateFn>(GetProcAddress(m, "XInputGetState")) : nullptr;
    }();
    static int s_pad = -1;
    static double s_checked = -10.0;
    const double now = ImGui::GetTime();
    if (now - s_checked < 1.0) return s_pad;
    s_checked = now;
    s_pad = -1;
    if (!s_get) return s_pad;
    for (DWORD i = 0; i < 4; ++i) {
        State st{};
        if (s_get(i, &st) == ERROR_SUCCESS) { s_pad = static_cast<int>(i); break; }
    }
    return s_pad;
}

// Reach (Xbox 360) default controller layout on Noble's controller picture (assets\ui\noble\controller.png,
// by O7OF), or on MCC's own picture when Noble's isn't there. Listed top to bottom on each side.
struct Callout {
    float bx, by;
    bool right;
    const char* label;
};
const Callout kReach360[] = {
    { 0.22f, 0.07f, false, "Throw grenade (LT)" },
    { 0.20f, 0.19f, false, "Use armor ability (LB)" },
    { 0.39f, 0.50f, false, "Multiplayer scoreboard (Back)" },
    { 0.20f, 0.53f, false, "Move / crouch (click)" },
    { 0.35f, 0.74f, false, "Night vision, team chat (D-pad)" },
    { 0.77f, 0.07f, true, "Fire (RT)" },
    { 0.81f, 0.19f, true, "Melee (RB)" },
    { 0.80f, 0.39f, true, "Swap weapons (Y)" },
    { 0.87f, 0.49f, true, "Switch grenades (B)" },
    { 0.72f, 0.50f, true, "Action / reload (X)" },
    { 0.61f, 0.50f, true, "Game menu (Start)" },
    { 0.79f, 0.60f, true, "Jump (A)" },
    { 0.64f, 0.76f, true, "Look / zoom (click)" },
};

// Controller picture with straight leader lines to the labels, fitted into `width`.
void DrawControllerDiagram(float width, int padLayout) {
    UiTexture pad = UiArt::NobleControllerImage();
    if (!pad) pad = UiArt::ControllerImage();
    // Each label keeps its "(RT)"-style suffix for the reader, and the button it names is drawn as a glyph
    // in front of it (2026-10-30).
    auto splitSuffix = [padLayout](const char* label, std::string& text, InputPrompts::Button& button) {
        text = label;
        button = InputPrompts::Button::A;
        const size_t open = text.rfind('(');
        if (open == std::string::npos || text.back() != ')') return false;
        const std::string suffix = text.substr(open + 1, text.size() - open - 2);
        // Both "(click)" callouts name a stick click; the label says which one.
        if (text.find("Move / crouch") != std::string::npos) button = InputPrompts::Button::LStick;
        else if (text.find("Look / zoom") != std::string::npos) button = InputPrompts::Button::RStick;
        else if (!InputPrompts::ButtonFromSuffix(suffix.c_str(), button)) return false;
        // A selected preset moves these actions to other buttons: relabel from the layout table so the diagram
        // shows what the player will actually press.
        if (padLayout > 0) {
            struct Map { const char* key; int action; };
            static const Map kActions[] = {
                { "Throw grenade", PadLayouts::Grenades }, { "Use armor ability", PadLayouts::ArmorAbility },
                { "Fire", PadLayouts::Fire },              { "Melee", PadLayouts::Melee },
                { "Swap weapons", PadLayouts::SwapWeapons }, { "Switch grenades", PadLayouts::Grenades },
                { "Action / reload", PadLayouts::ActionReload }, { "Jump", PadLayouts::Jump },
                { "Look / zoom", PadLayouts::Zoom },
            };
            for (const Map& m : kActions) {
                if (text.find(m.key) == std::string::npos) continue;
                const PadLayouts::Source src = PadLayouts::SourceFor(padLayout, m.action);
                switch (src.bit) {
                case 0x1000: button = InputPrompts::Button::A; break;
                case 0x2000: button = InputPrompts::Button::B; break;
                case 0x4000: button = InputPrompts::Button::X; break;
                case 0x8000: button = InputPrompts::Button::Y; break;
                case 0x0100: button = InputPrompts::Button::LB; break;
                case 0x0200: button = InputPrompts::Button::RB; break;
                case 0x0080: button = InputPrompts::Button::RStick; break;
                default:
                    button = src.trigger == 1 ? InputPrompts::Button::LT
                           : src.trigger == 2 ? InputPrompts::Button::RT : button;
                    break;
                }
                break;
            }
        }
        text = text.substr(0, open);
        while (!text.empty() && text.back() == ' ') text.pop_back();
        return true;
    };
    const float glyphH = ImGui::GetTextLineHeight() * 1.05f;
    float leftWidth = 0.0f, rightWidth = 0.0f;
    for (const Callout& c : kReach360) {
        std::string text;
        InputPrompts::Button button{};
        const bool hasGlyph = splitSuffix(c.label, text, button);
        const float glyphW = hasGlyph ? InputPrompts::ButtonWidth(ImGui::GetFont(), glyphH, button) + 6.0f : 0.0f;
        float& side = c.right ? rightWidth : leftWidth;
        side = std::max(side, glyphW + ImGui::CalcTextSize(text.c_str()).x);
    }
    const float w = std::clamp(width - leftWidth - rightWidth - 70.0f, 160.0f, 440.0f);
    const float h = pad ? w * pad.height / pad.width : w * 0.62f;
    const ImVec2 origin = ImGui::GetCursorScreenPos();
    const float padX = origin.x + leftWidth + 30.0f, padY = origin.y + 16.0f;
    ImDrawList* dl = ImGui::GetWindowDrawList();
    if (pad) dl->AddImage(pad.id, ImVec2(padX, padY), ImVec2(padX + w, padY + h));
    else dl->AddRect(ImVec2(padX, padY), ImVec2(padX + w, padY + h), IM_COL32(200, 210, 220, 120), 30.0f);
    int leftRow = 0, rightRow = 0;
    const float leftStep = h / 5.0f, rightStep = h / 8.0f;
    const float lh = ImGui::GetTextLineHeight();
    for (const Callout& c : kReach360) {
        const ImVec2 btn(padX + c.bx * w, padY + c.by * h);
        const float rowY = c.right ? padY + (rightRow++ + 0.5f) * rightStep - lh * 0.5f
                                   : padY + (leftRow++ + 0.5f) * leftStep - lh * 0.5f;
        const ImVec2 elbow = c.right ? ImVec2(padX + w + 14.0f, rowY + lh * 0.5f) : ImVec2(padX - 10.0f, rowY + lh * 0.5f);
        std::string text;
        InputPrompts::Button button{};
        const bool hasGlyph = splitSuffix(c.label, text, button);
        const float glyphW = hasGlyph ? InputPrompts::ButtonWidth(ImGui::GetFont(), glyphH, button) + 6.0f : 0.0f;
        const float rowW = glyphW + ImGui::CalcTextSize(text.c_str()).x;
        const ImVec2 rowPos = c.right ? ImVec2(elbow.x + 8.0f, rowY)
                                      : ImVec2(elbow.x - 8.0f - rowW, rowY);
        dl->AddLine(btn, elbow, IM_COL32(200, 210, 220, 140), 1.2f);
        dl->AddCircleFilled(btn, 3.0f, kAccent);
        if (hasGlyph) {
            InputPrompts::DrawButton(dl, ImGui::GetFont(), ImVec2(rowPos.x, rowY - (glyphH - lh) * 0.5f), glyphH, button);
        }
        dl->AddText(ImVec2(rowPos.x + glyphW, rowY), kText, text.c_str());
    }
    ImGui::Dummy(ImVec2(leftWidth + 30.0f + w + 22.0f + rightWidth, h + 30.0f));
}

} // namespace

// ---------------------------------------------------------------------------------------------
// Frame
// ---------------------------------------------------------------------------------------------

void SettingsPanel::SetTab(int tab) {
    const int n = static_cast<int>(Cat::Count);
    m_cat = static_cast<Cat>(((tab % n) + n) % n);
    m_focus = 0;
}

void SettingsPanel::SyncBuffers() {
    m_edit.playerName = m_nameBuf;
    m_edit.serviceTag = m_tagBuf;
    m_edit.masterServerUrl = m_masterBuf;
}

void SettingsPanel::Render() {
    if (!m_loaded) {
        m_saved = Settings::Get().Snapshot();
        // Old saved menu-music values ("shuffle", piece names) play Reach's menu theme: treat them as
        // that track so they don't count as an unsaved change.
        {
            const std::vector<ReachAudio::TrackInfo> tracks = ReachAudio::MusicTracks();
            if (std::none_of(tracks.begin(), tracks.end(), [&](const ReachAudio::TrackInfo& t) { return t.id == m_saved.menuMusicTrack; }) &&
                std::any_of(tracks.begin(), tracks.end(), [](const ReachAudio::TrackInfo& t) { return t.id == "reach:menu"; })) {
                m_saved.menuMusicTrack = "reach:menu";
            }
        }
        m_edit = m_saved;
        std::snprintf(m_nameBuf, sizeof(m_nameBuf), "%s", m_edit.playerName.c_str());
        std::snprintf(m_tagBuf, sizeof(m_tagBuf), "%s", m_edit.serviceTag.c_str());
        std::snprintf(m_masterBuf, sizeof(m_masterBuf), "%s", m_edit.masterServerUrl.c_str());
        m_loaded = true;
    }
    PollRebind();
    SyncBuffers();

    // Shared key state for this frame (rows read it; text fields and rebinding take the keys).
    m_navAllowed = !ImGui::GetIO().WantTextInput && m_rebindAction < 0;
    m_keyLeft = m_navAllowed && (ImGui::IsKeyPressed(ImGuiKey_LeftArrow) || ImGui::IsKeyPressed(ImGuiKey_GamepadDpadLeft) || ImGui::IsKeyPressed(ImGuiKey_GamepadLStickLeft));
    m_keyRight = m_navAllowed && (ImGui::IsKeyPressed(ImGuiKey_RightArrow) || ImGui::IsKeyPressed(ImGuiKey_GamepadDpadRight) || ImGui::IsKeyPressed(ImGuiKey_GamepadLStickRight));
    m_keyAccept = m_navAllowed && (ImGui::IsKeyPressed(ImGuiKey_Enter, false) || ImGui::IsKeyPressed(ImGuiKey_KeypadEnter, false) ||
                                   ImGui::IsKeyPressed(ImGuiKey_GamepadFaceDown, false));
    if (m_navAllowed) {
        const int rows = std::max(1, m_lastRowCount);
        if (ImGui::IsKeyPressed(ImGuiKey_DownArrow) || ImGui::IsKeyPressed(ImGuiKey_GamepadDpadDown) || ImGui::IsKeyPressed(ImGuiKey_GamepadLStickDown)) {
            m_focus = (m_focus + 1) % rows; m_scrollToFocus = true; MenuSounds::Scroll();
        }
        if (ImGui::IsKeyPressed(ImGuiKey_UpArrow) || ImGui::IsKeyPressed(ImGuiKey_GamepadDpadUp) || ImGui::IsKeyPressed(ImGuiKey_GamepadLStickUp)) {
            m_focus = (m_focus + rows - 1) % rows; m_scrollToFocus = true; MenuSounds::Scroll();
        }
        const int n = static_cast<int>(Cat::Count);
        if (ImGui::IsKeyPressed(ImGuiKey_Q, false) || ImGui::IsKeyPressed(ImGuiKey_GamepadL1, false)) {
            m_cat = static_cast<Cat>((static_cast<int>(m_cat) + n - 1) % n); m_focus = 0; m_search[0] = 0; MenuSounds::Tab(-1);
        }
        if (ImGui::IsKeyPressed(ImGuiKey_E, false) || ImGui::IsKeyPressed(ImGuiKey_GamepadR1, false)) {
            m_cat = static_cast<Cat>((static_cast<int>(m_cat) + 1) % n); m_focus = 0; m_search[0] = 0; MenuSounds::Tab(1);
        }
        if (TotalChanges() > 0) {
            if (ImGui::IsKeyPressed(ImGuiKey_Y, false) || ImGui::IsKeyPressed(ImGuiKey_GamepadFaceUp, false)) Apply();
            else if (ImGui::IsKeyPressed(ImGuiKey_X, false) || ImGui::IsKeyPressed(ImGuiKey_GamepadFaceLeft, false)) Discard();
        }
    }

    const ImVec2 avail = ImGui::GetContentRegionAvail();
    const float s = Scale();
    const float gap = 14.0f * s;
    const float leftW = std::clamp(avail.x * 0.24f, 230.0f * s, 330.0f * s);
    const float rightW = std::clamp(avail.x * 0.28f, 250.0f * s, 430.0f * s);
    const float midW = std::max(200.0f, avail.x - leftW - rightW - gap * 2.0f);

    RenderCategories(leftW, avail.y);
    ImGui::SameLine(0, gap);
    RenderRows(midW, avail.y);
    ImGui::SameLine(0, gap);
    RenderDetails(rightW, avail.y);
}

// ---------------------------------------------------------------------------------------------
// Left column: search, categories, unsaved box
// ---------------------------------------------------------------------------------------------

void SettingsPanel::RenderCategories(float width, float height) {
    const float s = Scale();
    ImGui::BeginChild("##setcats", ImVec2(width, height), ImGuiChildFlags_None, ImGuiWindowFlags_NoScrollbar);
    ImDrawList* dl = ImGui::GetWindowDrawList();

    ImGui::SetNextItemWidth(width);
    if (ImGui::InputTextWithHint("##setsearch", "Search all settings", m_search, sizeof(m_search))) {
        m_focus = 0;
    }
    ImGui::Dummy(ImVec2(0, 4.0f * s));

    const int total = TotalChanges();
    const float boxH = total > 0 ? 132.0f * s : 52.0f * s;
    ImGui::BeginChild("##setcatlist", ImVec2(width, ImGui::GetContentRegionAvail().y - boxH), ImGuiChildFlags_None);
    for (int i = 0; i < static_cast<int>(Cat::Count); ++i) {
        const CatInfo& ci = kCats[i];
        if (ci.group) SectionHeader(ci.group, width - 8.0f * s);
        const ImVec2 p = ImGui::GetCursorScreenPos();
        const float w = ImGui::GetContentRegionAvail().x;
        const float h = 50.0f * s;
        ImGui::PushID(i);
        const bool clicked = ImGui::InvisibleButton("##cat", ImVec2(w, h));
        const bool hovered = ImGui::IsItemHovered();
        ImGui::PopID();
        const bool sel = static_cast<int>(m_cat) == i && m_search[0] == 0;
        RowBackground(dl, p, ImVec2(p.x + w, p.y + h), sel, hovered);
        ImDrawList* wdl = ImGui::GetWindowDrawList();
        Text(wdl, NobleStyle::TitleFont(), 19.0f * s, ImVec2(p.x + 14.0f * s, p.y + 5.0f * s), sel ? kText : kTextDim, ci.name);
        const std::string summary = Summary(static_cast<Cat>(i));
        Text(wdl, NobleStyle::SmallFont(), 14.0f * s, ImVec2(p.x + 14.0f * s, p.y + 28.0f * s), kTextDim, summary.c_str());
        if (ChangesIn(static_cast<Cat>(i)) > 0) ChangedDot(wdl, ImVec2(p.x + w - 12.0f * s, p.y + h * 0.5f), s);
        if (hovered) MenuSounds::Hover(ImGui::GetItemID());
        if (clicked) {
            if (static_cast<int>(m_cat) != i) MenuSounds::Tab(i > static_cast<int>(m_cat) ? 1 : -1);
            m_cat = static_cast<Cat>(i);
            m_focus = 0;
            m_search[0] = 0;
        }
    }
    ImGui::EndChild();

    // Bottom: unsaved-changes box, or RESTORE DEFAULTS for this category.
    const ImVec2 b0 = ImGui::GetCursorScreenPos();
    if (total > 0) {
        const ImVec2 b1(b0.x + width, b0.y + boxH - 6.0f * s);
        dl->AddRectFilled(b0, b1, IM_COL32(40, 30, 14, 200));
        dl->AddRect(b0, b1, IM_COL32(255, 186, 74, 120), 0.0f, 0, 1.0f);
        ChangedDot(dl, ImVec2(b0.x + 14.0f * s, b0.y + 16.0f * s), s);
        char head[64];
        std::snprintf(head, sizeof(head), "%d UNSAVED CHANGE%s", total, total == 1 ? "" : "S");
        Text(dl, NobleStyle::TitleFont(), 17.0f * s, ImVec2(b0.x + 24.0f * s, b0.y + 6.0f * s), kAmber, head);
        std::string where = "In ";
        bool first = true;
        for (int i = 0; i < static_cast<int>(Cat::Count); ++i) {
            if (ChangesIn(static_cast<Cat>(i)) == 0) continue;
            if (!first) where += ", ";
            std::string n = kCats[i].name;
            for (size_t k = 1; k < n.size(); ++k) if (n[k - 1] != ' ' && n[k - 1] != '&') n[k] = static_cast<char>(::tolower(static_cast<unsigned char>(n[k])));
            where += n;
            first = false;
        }
        Text(dl, NobleStyle::SmallFont(), 14.0f * s, ImVec2(b0.x + 24.0f * s, b0.y + 30.0f * s), kTextDim, where.c_str(), width - 30.0f * s);
        const float bw = (width - 30.0f * s) * 0.5f;
        ImGui::SetCursorScreenPos(ImVec2(b0.x + 10.0f * s, b1.y - 44.0f * s));
        if (NobleStyle::PrimaryButton("APPLY CHANGES", ImVec2(bw, 30.0f * s))) Apply();
        ImGui::SameLine(0, 10.0f * s);
        if (ImGui::Button("DISCARD", ImVec2(bw, 30.0f * s))) Discard();
        Text(dl, NobleStyle::SmallFont(), 13.0f * s, ImVec2(b0.x + 10.0f * s, b1.y - 12.0f * s), kTextDim, "Y apply   X discard");
    } else {
        // Greyed when this category already has its default values.
        const bool differs = CountDiff(m_edit, WithDefaults(m_edit, m_cat), m_cat) > 0;
        ImGui::SetCursorScreenPos(ImVec2(b0.x, b0.y + 8.0f * s));
        ImGui::BeginDisabled(!differs || m_search[0] != 0);
        if (ImGui::Button("RESTORE DEFAULTS", ImVec2(width, 32.0f * s))) {
            RestoreDefaults(m_cat);
            MenuSounds::Click();
        }
        ImGui::EndDisabled();
        if (!m_message.empty() && ImGui::GetTime() - m_messageTime < 4.0) {
            Text(dl, NobleStyle::SmallFont(), 14.0f * s, ImVec2(b0.x, b0.y + 44.0f * s), kAccent, m_message.c_str());
        }
    }
    ImGui::EndChild();
}

// ---------------------------------------------------------------------------------------------
// Middle column: the rows
// ---------------------------------------------------------------------------------------------

void SettingsPanel::RenderRows(float width, float height) {
    const float s = Scale();
    ImGui::BeginChild("##setrows", ImVec2(width, height), ImGuiChildFlags_None);
    m_rowCount = 0;
    m_hover = Details{};
    m_focusDetails = Details{};
    m_pendingSection = nullptr;
    m_pendingCat = -1;

    if (m_search[0] != 0) {
        ImGui::TextColored(ImGui::ColorConvertU32ToFloat4(kTextDim), "Results for \"%s\"", m_search);
        for (int i = 0; i < static_cast<int>(Cat::Count); ++i) {
            m_pendingCat = i;
            RenderCategory(static_cast<Cat>(i));
            m_pendingCat = -1;
        }
        if (m_rowCount == 0) {
            ImGui::Dummy(ImVec2(0, 10.0f * s));
            Paragraph("No setting matches. Try another word, or clear the search to browse the categories.");
        }
    } else {
        const CatInfo& ci = kCats[static_cast<int>(m_cat)];
        const ImVec2 p = ImGui::GetCursorScreenPos();
        Text(ImGui::GetWindowDrawList(), NobleStyle::TitleFont(), 26.0f * s, p, kText, ci.name);
        ImGui::Dummy(ImVec2(0, 30.0f * s));
        Paragraph(ci.footer);
        RenderCategory(m_cat);
    }
    m_lastRowCount = m_rowCount;
    if (m_rowCount > 0 && m_focus >= m_rowCount) m_focus = m_rowCount - 1;
    m_scrollToFocus = false;
    ImGui::Dummy(ImVec2(0, 20.0f * s));
    ImGui::EndChild();
}

void SettingsPanel::RenderCategory(Cat c) {
    switch (c) {
    case Cat::Graphics: RenderGraphics(); break;
    case Cat::Audio: RenderAudio(); break;
    case Cat::Game: RenderGame(); break;
    case Cat::Player: RenderPlayer(); break;
    case Cat::KeyboardMouse: RenderKeyboardMouse(); break;
    case Cat::Controller: RenderController(); break;
    case Cat::Online: RenderOnline(); break;
    case Cat::System: RenderSystem(); break;
    default: break;
    }
}

void SettingsPanel::Section(const char* text) {
    if (m_search[0] != 0) {
        m_pendingSection = text;   // printed only when a row under it matches
        return;
    }
    SectionHeader(text, ImGui::GetContentRegionAvail().x);
}

bool SettingsPanel::Row(const char* label, const char* description, When when, const std::string& defaultText,
                        bool changed, bool disabled, Preview preview) {
    const float s = Scale();
    if (m_search[0] != 0) {
        const std::string needle = Lower(m_search);
        if (Lower(label).find(needle) == std::string::npos && Lower(description).find(needle) == std::string::npos) return false;
        if (m_pendingCat >= 0) {
            ImGui::Dummy(ImVec2(0, 8.0f * s));
            const ImVec2 p = ImGui::GetCursorScreenPos();
            Text(ImGui::GetWindowDrawList(), NobleStyle::TitleFont(), 20.0f * s, p, kText, kCats[m_pendingCat].name);
            ImGui::Dummy(ImVec2(0, 24.0f * s));
            m_pendingCat = -1;
        }
        if (m_pendingSection) {
            SectionHeader(m_pendingSection, ImGui::GetContentRegionAvail().x);
            m_pendingSection = nullptr;
        }
    }
    const int index = m_rowCount++;
    m_curRow = index;
    const ImVec2 p = ImGui::GetCursorScreenPos();
    const float w = ImGui::GetContentRegionAvail().x;
    const float h = 40.0f * s;
    const ImVec2 q(p.x + w, p.y + h);
    const bool hovered = ImGui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem) && ImGui::IsMouseHoveringRect(p, q);
    if (hovered && ImGui::IsMouseClicked(0) && m_focus != index) {
        m_focus = index;
        MenuSounds::Scroll();
    }
    const bool focused = index == m_focus;
    if (focused && m_scrollToFocus) ImGui::SetScrollFromPosY(p.y - ImGui::GetWindowPos().y, 0.4f);

    ImDrawList* dl = ImGui::GetWindowDrawList();
    RowBackground(dl, p, q, focused, hovered);
    if (changed) ChangedDot(dl, ImVec2(p.x + 10.0f * s, p.y + h * 0.5f), s);
    Text(dl, NobleStyle::TitleFont(), 17.0f * s, ImVec2(p.x + 20.0f * s, p.y + h * 0.5f - 10.0f * s),
         disabled ? kTextOff : (focused ? kText : IM_COL32(190, 214, 232, 255)), label);
    ImGui::Dummy(ImVec2(w, h));

    m_valMin = ImVec2(p.x + w * 0.46f, p.y + 4.0f * s);
    m_valMax = ImVec2(q.x - 10.0f * s, q.y - 4.0f * s);
    m_rowFocused = focused;
    m_rowHovered = hovered;
    m_rowDisabled = disabled;

    Details d;
    d.set = true;
    d.title = label;
    d.effect = WhenText(static_cast<int>(when));
    d.text = description;
    if (!defaultText.empty()) d.facts = "DEFAULT   " + defaultText;
    d.preview = preview;
    if (hovered) m_hover = d;
    if (focused) m_focusDetails = d;
    return true;
}

bool SettingsPanel::SliderValue(float& v, float lo, float hi, float step, float def, const char* text) {
    const float s = Scale();
    bool changed = false;
    const ImVec2 save = ImGui::GetCursorScreenPos();
    const float textW = std::max(86.0f * s, TextWidth(nullptr, 17.0f * s, text) + 14.0f * s);
    const bool percent = std::strchr(text, '%') != nullptr && hi <= 1.0f;   // shown as 0-100%, stored as 0-1
    ImGui::SetCursorScreenPos(m_valMin);
    ImGui::PushID(m_curRow);
    ImGui::InvisibleButton("##slider", ImVec2(std::max(1.0f, m_valMax.x - m_valMin.x - textW), m_valMax.y - m_valMin.y));
    const bool active = ImGui::IsItemActive();
    // The value on the right is a box: click it and type a number (Enter to set, Esc to cancel).
    const ImVec2 box0(m_valMax.x - textW + 6.0f * s, m_valMin.y), box1 = m_valMax;
    ImGui::SetCursorScreenPos(box0);
    const bool typingHere = m_typeRow == m_curRow;
    if (!typingHere && ImGui::InvisibleButton("##value", ImVec2(box1.x - box0.x, box1.y - box0.y)) && !m_rowDisabled) {
        m_typeRow = m_curRow;
        m_typeFocus = true;
        m_focus = m_curRow;
        std::snprintf(m_typeBuf, sizeof(m_typeBuf), step >= 1.0f && !percent ? "%.0f" : "%.2f", percent ? v * 100.0f : v);
        MenuSounds::Click();
    }
    const bool valueHovered = !typingHere && ImGui::IsItemHovered() && !m_rowDisabled;
    if (!m_rowDisabled) {
        if (active) {
            m_focus = m_curRow;
            const float t = std::clamp((ImGui::GetIO().MousePos.x - m_valMin.x) / std::max(1.0f, m_valMax.x - m_valMin.x - textW), 0.0f, 1.0f);
            float nv = lo + t * (hi - lo);
            if (step > 0.0f) nv = std::round(nv / step) * step;
            nv = std::clamp(nv, lo, hi);
            if (std::fabs(nv - v) > 1e-6f) { v = nv; changed = true; }
        }
        if (m_rowFocused && (m_keyLeft || m_keyRight)) {
            const float st = step > 0.0f ? step : (hi - lo) / 50.0f;
            const float nv = std::clamp(v + (m_keyRight ? st : -st), lo, hi);
            if (std::fabs(nv - v) > 1e-6f) { v = nv; changed = true; MenuSounds::Change(); }
        }
    }
    ImDrawList* dl = ImGui::GetWindowDrawList();
    DrawSlider(dl, m_valMin, m_valMax, (v - lo) / (hi - lo), def >= lo && def <= hi ? (def - lo) / (hi - lo) : -1.0f,
               typingHere ? "" : text, active || m_rowFocused, m_rowDisabled, textW);
    if (valueHovered) dl->AddRect(box0, box1, IM_COL32(170, 220, 255, 110), 2.0f * s, 0, 1.0f);
    if (typingHere) {
        ImGui::SetCursorScreenPos(ImVec2(box0.x, (box0.y + box1.y) * 0.5f - ImGui::GetFrameHeight() * 0.5f));
        ImGui::SetNextItemWidth(box1.x - box0.x);
        if (m_typeFocus) {
            ImGui::SetKeyboardFocusHere();
            m_typeFocus = false;
        }
        const bool enter = ImGui::InputText("##type", m_typeBuf, sizeof(m_typeBuf),
                                            ImGuiInputTextFlags_CharsDecimal | ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_EnterReturnsTrue);
        const bool done = enter || ImGui::IsItemDeactivated();
        if (done) {
            if (!ImGui::IsKeyPressed(ImGuiKey_Escape, false)) {
                char* end = nullptr;
                float nv = std::strtof(m_typeBuf, &end);
                if (end != m_typeBuf) {
                    if (percent) nv /= 100.0f;
                    // Typed values keep two decimals (whole numbers for whole-number sliders), inside the slider's range.
                    nv = step >= 1.0f ? std::round(nv) : std::round(nv * 100.0f) / 100.0f;
                    nv = std::clamp(nv, lo, hi);
                    if (std::fabs(nv - v) > 1e-6f) { v = nv; changed = true; }
                }
            }
            m_typeRow = -1;
        }
    }
    ImGui::PopID();
    ImGui::SetCursorScreenPos(save);
    return changed;
}

int SettingsPanel::CycleValue(const char* value, int index, int count, bool wraps) {
    const float s = Scale();
    const ImVec2 save = ImGui::GetCursorScreenPos();
    const float aw = 30.0f * s;
    const float h = m_valMax.y - m_valMin.y;
    int change = 0;
    ImGui::PushID(m_curRow);
    ImGui::SetCursorScreenPos(m_valMin);
    if (ImGui::InvisibleButton("##l", ImVec2(aw, h))) change = -1;
    const bool hl = ImGui::IsItemHovered();
    ImGui::SetCursorScreenPos(ImVec2(m_valMax.x - aw, m_valMin.y));
    if (ImGui::InvisibleButton("##r", ImVec2(aw, h))) change = 1;
    const bool hr = ImGui::IsItemHovered();
    ImGui::PopID();
    if (m_rowFocused) {
        if (m_keyLeft) change = -1;
        if (m_keyRight || m_keyAccept) change = 1;
    }
    if (m_rowDisabled) change = 0;
    if (change && !wraps) {
        if ((change < 0 && index <= 0) || (change > 0 && index >= count - 1)) change = 0;
    }
    if (change) {
        m_focus = m_curRow;
        MenuSounds::Change();
    }
    DrawCycle(ImGui::GetWindowDrawList(), m_valMin, m_valMax, value, index, count, wraps, hl, hr, m_rowDisabled);
    ImGui::SetCursorScreenPos(save);
    return change;
}

bool SettingsPanel::ToggleValue(bool& v) {
    const ImVec2 save = ImGui::GetCursorScreenPos();
    ImGui::PushID(m_curRow);
    ImGui::SetCursorScreenPos(m_valMin);
    const bool clicked = ImGui::InvisibleButton("##toggle", ImVec2(m_valMax.x - m_valMin.x, m_valMax.y - m_valMin.y));
    const bool hovered = ImGui::IsItemHovered();
    ImGui::PopID();
    bool changed = false;
    if (!m_rowDisabled && (clicked || (m_rowFocused && (m_keyAccept || m_keyLeft || m_keyRight)))) {
        v = !v;
        changed = true;
        m_focus = m_curRow;
        MenuSounds::Change();
    }
    DrawToggle(ImGui::GetWindowDrawList(), m_valMin, m_valMax, v, hovered, m_rowDisabled);
    ImGui::SetCursorScreenPos(save);
    return changed;
}

void SettingsPanel::InfoValue(const char* text) {
    const float s = Scale();
    const float tw = TextWidth(nullptr, 17.0f * s, text);
    const float x = std::max(m_valMin.x, m_valMax.x - tw);
    ImDrawList* dl = ImGui::GetWindowDrawList();
    dl->PushClipRect(m_valMin, m_valMax, true);
    Text(dl, nullptr, 17.0f * s, ImVec2(x, (m_valMin.y + m_valMax.y) * 0.5f - 9.0f * s), m_rowDisabled ? kTextOff : kTextDim, text);
    dl->PopClipRect();
}

// ---------------------------------------------------------------------------------------------
// Right column: details
// ---------------------------------------------------------------------------------------------

void SettingsPanel::RenderDetails(float width, float height) {
    const float s = Scale();
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.02f, 0.04f, 0.06f, 0.55f));
    ImGui::BeginChild("##setdetails", ImVec2(width, height), ImGuiChildFlags_Borders);
    const Details& d = m_hover.set ? m_hover : m_focusDetails;
    ImDrawList* dl = ImGui::GetWindowDrawList();
    if (!d.set) {
        Paragraph("Pick a setting to see what it does, when it takes effect and its default value.");
        ImGui::EndChild();
        ImGui::PopStyleColor();
        return;
    }
    ImGui::Dummy(ImVec2(0, 4.0f * s));
    const ImVec2 p = ImGui::GetCursorScreenPos();
    const float w = ImGui::GetContentRegionAvail().x;
    Text(dl, NobleStyle::TitleFont(), 22.0f * s, p, kText, d.title.c_str(), w);
    ImGui::Dummy(ImVec2(0, 28.0f * s));
    ImGui::PushStyleColor(ImGuiCol_Text, ImGui::ColorConvertU32ToFloat4(kAccent));
    ImGui::PushTextWrapPos(0.0f);
    ImGui::TextUnformatted(d.effect.c_str());
    ImGui::PopTextWrapPos();
    ImGui::PopStyleColor();
    ImGui::Dummy(ImVec2(0, 8.0f * s));

    // Preview.
    const ImVec2 pv = ImGui::GetCursorScreenPos();
    switch (d.preview) {
    case Preview::Fov: {
        // Top-down view cone with the chosen angle.
        const float r = std::min(w * 0.42f, 110.0f * s);
        const ImVec2 eye(pv.x + w * 0.5f, pv.y + r + 6.0f * s);
        const float half = m_edit.fieldOfView * 0.5f * 3.14159265f / 180.0f;
        const float a0 = -1.5707963f - half, a1 = -1.5707963f + half;
        dl->PathLineTo(eye);
        dl->PathArcTo(eye, r, a0, a1, 32);
        dl->PathFillConvex(IM_COL32(110, 180, 225, 70));
        dl->AddLine(eye, ImVec2(eye.x + std::cos(a0) * r, eye.y + std::sin(a0) * r), kAccent, 1.5f);
        dl->AddLine(eye, ImVec2(eye.x + std::cos(a1) * r, eye.y + std::sin(a1) * r), kAccent, 1.5f);
        const float hd = 78.0f * 0.5f * 3.14159265f / 180.0f;   // Reach's own 78 degrees, dashed
        dl->AddLine(eye, ImVec2(eye.x + std::cos(-1.5707963f - hd) * r, eye.y + std::sin(-1.5707963f - hd) * r), IM_COL32(255, 255, 255, 70), 1.0f);
        dl->AddLine(eye, ImVec2(eye.x + std::cos(-1.5707963f + hd) * r, eye.y + std::sin(-1.5707963f + hd) * r), IM_COL32(255, 255, 255, 70), 1.0f);
        dl->AddCircleFilled(eye, 4.0f * s, kText);
        const std::string deg = Format("%.0f\xC2\xB0", m_edit.fieldOfView);
        Text(dl, NobleStyle::TitleFont(), 22.0f * s, ImVec2(eye.x - TextWidth(NobleStyle::TitleFont(), 22.0f * s, deg.c_str()) * 0.5f, eye.y + 6.0f * s), kText, deg.c_str());
        ImGui::Dummy(ImVec2(w, r + 44.0f * s));
        break;
    }
    case Preview::FrameLimit: {
        const std::string big = m_edit.fpsLimit == 0 ? std::string("NO LIMIT") : std::to_string(m_edit.fpsLimit) + " FPS";
        Text(dl, NobleStyle::TitleFont(), 28.0f * s, pv, kText, big.c_str());
        const float ms = m_edit.fpsLimit == 0 ? 0.0f : 1000.0f / static_cast<float>(m_edit.fpsLimit);
        const ImVec2 b0(pv.x, pv.y + 40.0f * s), b1(pv.x + w, pv.y + 52.0f * s);
        dl->AddRectFilled(b0, b1, IM_COL32(255, 255, 255, 25));
        if (ms > 0.0f) dl->AddRectFilled(b0, ImVec2(b0.x + w * std::min(ms / 100.0f, 1.0f), b1.y), kAccent);
        const std::string cap = ms > 0.0f ? Format("%.1f ms per frame (scale 0 - 100 ms)", ms) : std::string("As many frames as your PC can draw");
        Text(dl, NobleStyle::SmallFont(), 14.0f * s, ImVec2(pv.x, b1.y + 6.0f * s), kTextDim, cap.c_str());
        ImGui::Dummy(ImVec2(w, 82.0f * s));
        break;
    }
    case Preview::Volume: {
        const struct { const char* name; float v; } meters[] = {
            { "GAME", m_edit.volumeGame }, { "MUSIC", m_edit.volumeMusic }, { "VOICE / OTHER", m_edit.volumeOther } };
        float y = pv.y;
        for (const auto& m : meters) {
            Text(dl, NobleStyle::SmallFont(), 14.0f * s, ImVec2(pv.x, y), kTextDim, m.name);
            const ImVec2 b0(pv.x + 110.0f * s, y + 3.0f * s), b1(pv.x + w, y + 13.0f * s);
            dl->AddRectFilled(b0, b1, IM_COL32(255, 255, 255, 25));
            const int segs = 20;
            for (int i = 0; i < segs; ++i) {
                if ((i + 0.5f) / segs > m.v) break;
                const float x0 = b0.x + (b1.x - b0.x) * i / segs, x1 = b0.x + (b1.x - b0.x) * (i + 1) / segs - 2.0f;
                dl->AddRectFilled(ImVec2(x0, b0.y), ImVec2(x1, b1.y), i >= 17 ? kAmber : kAccent);
            }
            y += 24.0f * s;
        }
        ImGui::Dummy(ImVec2(w, y - pv.y + 6.0f * s));
        break;
    }
    case Preview::Background: {
        // The details panel already drew the title and the effect line; a preview starts at the cursor (pv).
        const std::string value = m_edit.menuBackground;
        // The picture the menu is actually using: it was chosen and uploaded at menu init (WarmMenuBackground),
        // so nothing decodes a texture here - doing that on the fallback draw thread hung the client.
        const UiTexture tex = UiArt::MenuBackground();
        const float bw = ImGui::GetContentRegionAvail().x;
        const float bh = bw * 9.0f / 16.0f;
        const ImVec2 a = ImGui::GetCursorScreenPos();
        const ImVec2 b(a.x + bw, a.y + bh);
        dl->AddRectFilled(a, b, IM_COL32(10, 15, 22, 255));
        if (tex) {
            UiArt::DrawCover(dl, tex, a, b);
        } else {
            const char* none = "NO PICTURE";
            const ImVec2 ts = ImGui::CalcTextSize(none);
            dl->AddText(ImVec2(a.x + (bw - ts.x) * 0.5f, a.y + (bh - ts.y) * 0.5f), IM_COL32(120, 135, 150, 200), none);
        }
        dl->AddRect(a, b, IM_COL32(150, 185, 215, 110), 0.0f, 0, 1.0f);
        ImGui::Dummy(ImVec2(bw, bh));
        ImGui::Dummy(ImVec2(0, 6.0f * Scale()));
        ImGui::PushStyleColor(ImGuiCol_Text, NobleStyle::TextMuted());
        if (value.rfind("file:", 0) == 0) ImGui::TextWrapped("Your picture: %s", value.substr(5).c_str());
        else if (value == "reach") ImGui::TextWrapped("A random Halo: Reach campaign scene from your MCC install.");
        else if (value == "noble") ImGui::TextWrapped("Noble's own background art.");
        else ImGui::TextWrapped("A random picture from your own files, Noble's art and Reach's scenes.");
        ImGui::TextWrapped("A new choice is used from the next start of Noble.");
        ImGui::PopStyleColor();
        break;
    }
    case Preview::Controller: {
        const int pad = ConnectedPad();
        const std::string line = pad >= 0 ? "Controller " + std::to_string(pad + 1) + " connected." : std::string("No controller found.");
        Text(dl, NobleStyle::TitleFont(), 18.0f * s, pv, pad >= 0 ? kAccent : kTextDim, line.c_str());
        ImGui::Dummy(ImVec2(w, 28.0f * s));
        if (UiTexture img = UiArt::NobleControllerImage()) {
            const ImVec2 i0 = ImGui::GetCursorScreenPos();
            const float iw = w, ih = iw * img.height / img.width;
            dl->AddImage(img.id, i0, ImVec2(i0.x + iw, i0.y + ih));
            ImGui::Dummy(ImVec2(iw, ih + 6.0f * s));
        }
        break;
    }
    case Preview::Gpu: {
        Text(dl, NobleStyle::SmallFont(), 14.0f * s, pv, kTextDim, "IN USE NOW");
        Text(dl, NobleStyle::TitleFont(), 18.0f * s, ImVec2(pv.x, pv.y + 18.0f * s), kText, GpuLine().c_str(), w);
        ImGui::Dummy(ImVec2(w, 64.0f * s));
        break;
    }
    default: break;
    }

    if (!d.text.empty()) {
        Paragraph(d.text.c_str(), IM_COL32(205, 214, 222, 255));
    }
    if (!d.facts.empty()) {
        ImGui::Dummy(ImVec2(0, 8.0f * s));
        const ImVec2 f = ImGui::GetCursorScreenPos();
        dl->AddLine(f, ImVec2(f.x + w, f.y), kRule, 1.0f);
        ImGui::Dummy(ImVec2(0, 6.0f * s));
        Paragraph(d.facts.c_str(), kTextDim);
    }
    ImGui::EndChild();
    ImGui::PopStyleColor();
}

// ---------------------------------------------------------------------------------------------
// Categories
// ---------------------------------------------------------------------------------------------

void SettingsPanel::RenderGraphics() {
    const SettingsData def;
    Section("DISPLAY");
    if (Row("GRAPHICS CARD", "The graphics card Windows gave Noble. Choosing another card here will come later.",
            When::Info, "", false, false, Preview::Gpu)) {
        InfoValue(GpuLine().c_str());
    }
    if (Row("DISPLAY MODE", "Windowed: a fixed-size window you can move. Borderless full screen: a window that covers the whole display (use it for full screen; the engine can't change resolution while it runs).",
            When::Restart, def.borderless ? "Borderless" : "Windowed", m_edit.borderless != m_saved.borderless)) {
        if (CycleValue(m_edit.borderless ? "BORDERLESS FULL SCREEN" : "WINDOWED", m_edit.borderless ? 1 : 0, 2)) m_edit.borderless = !m_edit.borderless;
    }
    {
        int current = 0;
        const int n = static_cast<int>(std::size(kResolutions));
        for (int i = 0; i < n; ++i) if (kResolutions[i].w == m_edit.width && kResolutions[i].h == m_edit.height) current = i;
        char label[32];
        if (m_edit.borderless) std::snprintf(label, sizeof(label), "Fills your display");
        else std::snprintf(label, sizeof(label), "%d x %d", m_edit.width, m_edit.height);
        const bool changed = m_edit.width != m_saved.width || m_edit.height != m_saved.height;
        if (Row("WINDOW SIZE", "Size of the game window in windowed mode. In borderless full screen the window fills your display, so this is greyed out.",
                When::Restart, "1280 x 720", changed, m_edit.borderless)) {
            if (const int d = CycleValue(label, current, n, false)) {
                current = std::clamp(current + d, 0, n - 1);
                m_edit.width = kResolutions[current].w;
                m_edit.height = kResolutions[current].h;
            }
        }
    }
    Section("FRAME RATE");
    if (Row("VERTICAL SYNC", "Waits for your display before showing each frame: no tearing, but a little more input delay.",
            When::Apply, def.verticalSync ? "On" : "Off", m_edit.verticalSync != m_saved.verticalSync)) {
        ToggleValue(m_edit.verticalSync);
    }
    {
        const int n = static_cast<int>(std::size(kFpsLimits));
        int current = n - 1;
        for (int i = 0; i < n; ++i) if (kFpsLimits[i] == m_edit.fpsLimit) current = i;
        const std::string label = m_edit.fpsLimit == 0 ? std::string("NO LIMIT") : std::to_string(m_edit.fpsLimit) + " FPS";
        if (Row("FRAME-RATE LIMIT", "Highest frame rate Noble draws. A limit at your display's refresh rate keeps the PC cooler and quieter; menus and loading screens keep the display's own rate.",
                When::Apply, "No limit", m_edit.fpsLimit != m_saved.fpsLimit, false, Preview::FrameLimit)) {
            if (const int d = CycleValue(label.c_str(), current, n, false)) m_edit.fpsLimit = kFpsLimits[std::clamp(current + d, 0, n - 1)];
        }
    }
    if (Row("FPS COUNTER", "Shows the frame rate in the upper-right corner while you play.",
            When::Apply, "Off", m_edit.showFps != m_saved.showFps)) {
        ToggleValue(m_edit.showFps);
    }
    Section("ADVANCED GRAPHICS");
    static const char* const kLater[] = { "TEXTURE FILTERING", "ANTIALIASING", "MOTION BLUR", "SHADOWS", "DRAW DISTANCE" };
    for (const char* name : kLater) {
        if (Row(name, "Needs the matching setting to be found inside Halo: Reach first; it will light up here when it works.",
                When::Info, "", false, true)) {
            InfoValue("Not available yet");
        }
    }
}

void SettingsPanel::RenderAudio() {
    const SettingsData def;
    Section("VOLUME");
    auto volume = [&](const char* label, const char* text, float& v, float saved) {
        if (Row(label, text, When::Apply, "100 %", std::fabs(v - saved) > 1e-4f, false, Preview::Volume)) {
            SliderValue(v, 0.0f, 1.0f, 0.01f, 1.0f, Percent(v).c_str());
        }
    };
    volume("GAME", "Gunfire, vehicles, footsteps and everything else in the world.", m_edit.volumeGame, m_saved.volumeGame);
    volume("MUSIC", "Halo: Reach's score in game, and the menu music.", m_edit.volumeMusic, m_saved.volumeMusic);
    volume("VOICE / OTHER", "Dialogue, radio chatter, announcer and menu sounds.", m_edit.volumeOther, m_saved.volumeOther);
    Section("MENUS");
    if (Row("MENU SOUNDS", "Reach's own clicks and swooshes when you move through the menus.",
            When::Apply, "On", m_edit.menuSounds != m_saved.menuSounds)) {
        ToggleValue(m_edit.menuSounds);
    }
    if (Row("MENU MUSIC", "The song the menus play. Pick one in the list below: it plays at once; APPLY CHANGES keeps it, DISCARD goes back. Also plays in the pre-game lobby.",
            When::Instant, "Halo: Reach - Main Menu Theme", m_edit.menuMusicTrack != m_saved.menuMusicTrack)) {
        InfoValue(ReachAudio::TrackLabel(m_edit.menuMusicTrack).c_str());
    }
    if (m_search[0] == 0) RenderMusicPicker();
    ReachAudio::SetMusicVolume(m_edit.volumeMusic);
    const std::string problem = ReachAudio::Problem();
    if (!problem.empty() && m_search[0] == 0) {
        ImGui::Dummy(ImVec2(0, 4));
        Paragraph(("Reach menu audio is off: " + problem).c_str());
    }
    Section("MORE");
    if (Row("MASTER MUTE", "Silences Noble's own audio: the menu music and the menu clicks. The volumes below are kept, so turning it back off restores them. (Reach's in-game sound is the engine's own and has its own settings.)",
            When::Instant, "Off", m_edit.masterMute != m_saved.masterMute)) {
        ToggleValue(m_edit.masterMute);
    }
    if (Row("MUTE IN BACKGROUND", "Also silences Noble's audio whenever Noble is not the active window, so alt-tabbing to a video or a call leaves the game quiet.",
            When::Instant, "Off", m_edit.muteInBackground != m_saved.muteInBackground)) {
        ToggleValue(m_edit.muteInBackground);
    }
}

void SettingsPanel::RenderGame() {
    Section("CAMERA");
    if (Row("FIELD OF VIEW", "How wide you see, in horizontal degrees (70 - 120). Halo: Reach's own value is 78 (the faint lines in the picture). Also changes mid-match from the pause menu.",
            When::Apply, "78\xC2\xB0", std::fabs(m_edit.fieldOfView - m_saved.fieldOfView) > 0.01f, false, Preview::Fov)) {
        SliderValue(m_edit.fieldOfView, 70.0f, 120.0f, 1.0f, 78.0f, Format("%.0f\xC2\xB0", m_edit.fieldOfView).c_str());
    }
    if (Row("IN-VEHICLE FOV", "A separate field of view while driving or flying. Needs the vehicle camera to be found in the engine first.",
            When::Info, "", false, true)) {
        InfoValue("Not available yet");
    }
    Section("HUD");
    if (Row("SUBTITLES", "Subtitles for cutscenes and radio dialogue.", When::Apply, "On", m_edit.subtitles != m_saved.subtitles)) {
        ToggleValue(m_edit.subtitles);
    }
    if (Row("HIDE HUD", "Turns the whole HUD off, for screenshots and videos. Needs HUD research first.", When::Info, "", false, true)) {
        InfoValue("Not available yet");
    }
    Section("MENUS");
    // Menu background: the three pools plus every picture the player has (their own files are "file:<name>"),
    // with the chosen picture previewed in the details column and an IMPORT button next to it.
    static const char* const kPoolValues[] = { "random", "noble", "reach" };
    static const char* const kPoolLabels[] = { "RANDOM", "NOBLE ART", "HALO: REACH (MCC)" };
    const std::vector<std::string> ownFiles = UiArt::MenuBackgroundFiles();
    std::vector<std::string> values(kPoolValues, kPoolValues + 3);
    std::vector<std::string> labels(kPoolLabels, kPoolLabels + 3);
    for (const std::string& f : ownFiles) {
        values.push_back("file:" + f);
        labels.push_back(f);
    }
    int choice = 0;
    for (size_t i = 0; i < values.size(); ++i) if (m_edit.menuBackground == values[i]) choice = static_cast<int>(i);
    if (Row("MENU BACKGROUND", "The picture behind every menu. Pick one of the pools or one of your own images (data\\ui\\custom\\backgrounds); IMPORT adds a new one.",
            When::Restart, "Random", m_edit.menuBackground != m_saved.menuBackground, false, Preview::Background)) {
        if (const int d = CycleValue(labels[static_cast<size_t>(choice)].c_str(), choice, static_cast<int>(values.size()))) {
            m_edit.menuBackground = values[static_cast<size_t>((choice + d + static_cast<int>(values.size())) % static_cast<int>(values.size()))];
        }
    }
    // Action row: Row() is a layout helper that always returns true, so the file dialog must be opened from
    // an explicit click on the row (or Accept while it is focused), not from the return value. Opening it every
    // frame hung the client, because the dialog waits for input (2026-10-30).
    const std::string ownCount = ownFiles.empty() ? std::string("No own pictures")
                                                  : std::to_string(ownFiles.size()) + " own picture(s)";
    if (Row("IMPORT BACKGROUND...", "Pick an image (PNG, JPG, BMP or WebP). It is copied into data\\ui\\custom\\backgrounds and selected here; the menu uses it from the next start.",
            When::Restart, ownCount.c_str(), false)) {
        InfoValue(ownFiles.empty() ? "No own pictures" : ownCount.c_str());
        const bool clicked = m_rowHovered && ImGui::IsMouseClicked(0);
        const bool accepted = m_rowFocused && (ImGui::IsKeyPressed(ImGuiKey_Enter, false) ||
                                               ImGui::IsKeyPressed(ImGuiKey_GamepadFaceDown, false));
        if (clicked || accepted) {
            wchar_t file[MAX_PATH] = {};
            OPENFILENAMEW ofn{};
            ofn.lStructSize = sizeof(ofn);
            ofn.hwndOwner = GetActiveWindow();
            ofn.lpstrFilter = L"Images\0*.png;*.jpg;*.jpeg;*.bmp;*.webp\0All files\0*.*\0";
            ofn.lpstrFile = file;
            ofn.nMaxFile = MAX_PATH;
            ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR;
            if (GetOpenFileNameW(&ofn)) {
                std::string name;
                if (UiArt::ImportMenuBackgroundImage(std::filesystem::path(file), name)) {
                    m_edit.menuBackground = "file:" + name;
                    MenuSounds::Click();
                } else {
                    MenuSounds::Error();
                }
            }
        }
    }
}

void SettingsPanel::RenderPlayer() {
    const float s = Scale();
    Section("IDENTITY");
    auto textRow = [&](const char* label, const char* text, const char* def, bool changed, char* buf, size_t size, ImGuiInputTextFlags flags) {
        if (!Row(label, text, When::Apply, def, changed)) return;
        const ImVec2 save = ImGui::GetCursorScreenPos();
        ImGui::SetCursorScreenPos(ImVec2(m_valMin.x, (m_valMin.y + m_valMax.y) * 0.5f - ImGui::GetFrameHeight() * 0.5f));
        ImGui::SetNextItemWidth(m_valMax.x - m_valMin.x);
        ImGui::PushID(m_curRow);
        if (m_rowFocused && m_keyAccept) ImGui::SetKeyboardFocusHere();
        ImGui::InputText("##text", buf, size, flags);
        if (ImGui::IsItemActive()) m_focus = m_curRow;
        ImGui::PopID();
        ImGui::SetCursorScreenPos(save);
        (void)s;
    };
    textRow("PLAYER NAME", "Your name on scoreboards, in the party box and (later) in chat. Up to 15 characters.",
            "NobleSix", m_edit.playerName != m_saved.playerName, m_nameBuf, 16, ImGuiInputTextFlags_None);
    textRow("SERVICE TAG", "Up to 4 letters or digits, shown next to your name in game (Reach's service tag).",
            "NOBL", m_edit.serviceTag != m_saved.serviceTag, m_tagBuf, 5, ImGuiInputTextFlags_CharsUppercase | ImGuiInputTextFlags_CharsNoBlank);
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
            MenuSounds::Click();
            return;
        }
    }
}

void SettingsPanel::RenderKeyboardMouse() {
    const float s = Scale();
    Section("MOUSE");
    if (Row("LOOK SENSITIVITY", "How far you turn for each bit of mouse movement. 1.00x is Noble's default.",
            When::Apply, "1.00x", std::fabs(m_edit.mouseSensitivity - m_saved.mouseSensitivity) > 1e-4f)) {
        SliderValue(m_edit.mouseSensitivity, 0.1f, 5.0f, 0.05f, 1.0f, Format("%.2fx", m_edit.mouseSensitivity).c_str());
    }
    if (Row("VERTICAL SENSITIVITY", "Up/down speed compared with left/right. Below 1 makes aiming up and down calmer.",
            When::Apply, "1.00x", std::fabs(m_edit.mouseSensitivityY - m_saved.mouseSensitivityY) > 1e-4f)) {
        SliderValue(m_edit.mouseSensitivityY, 0.25f, 4.0f, 0.05f, 1.0f, Format("%.2fx", m_edit.mouseSensitivityY).c_str());
    }
    if (Row("INVERT LOOK", "Inverted: moving the mouse forward looks down, as in flight controls.",
            When::Apply, "Off", m_edit.invertLook != m_saved.invertLook)) {
        ToggleValue(m_edit.invertLook);
    }
    if (Row("MOUSE ACCELERATION", "Fast flicks turn further than slow movements, like most shooters' default. 0 % is raw input, which is what the best players use for consistency.",
            When::Apply, "Off", std::fabs(m_edit.mouseAcceleration - m_saved.mouseAcceleration) > 1e-4f)) {
        SliderValue(m_edit.mouseAcceleration, 0.0f, 1.0f, 0.01f, 0.0f,
                    m_edit.mouseAcceleration < 0.001f ? "Off" : Percent(m_edit.mouseAcceleration).c_str());
    }
    if (Row("MOUSE SMOOTHING", "Averages the mouse's movement over a few frames. It can steady a shaky hand, but it adds latency, so Off is the default.",
            When::Apply, "Off", std::fabs(m_edit.mouseSmoothing - m_saved.mouseSmoothing) > 1e-4f)) {
        SliderValue(m_edit.mouseSmoothing, 0.0f, 0.9f, 0.05f, 0.0f,
                    m_edit.mouseSmoothing < 0.001f ? "Off" : Percent(m_edit.mouseSmoothing).c_str());
    }
    Section("MOUSE WHEEL");
    auto wheelName = [](int action) -> std::string {
        if (action < 0) return "NOTHING";
        const char* n = KeyBindings::ActionName(action);
        return n ? Upper(n) : "SLOT " + std::to_string(action);
    };
    auto wheelRow = [&](const char* label, int& action, int saved) {
        if (!Row(label, "The action the mouse wheel triggers. With both directions on CHANGE WEAPON (the default) either way switches weapons.",
                 When::Apply, "Change weapon", action != saved)) return;
        const int n = static_cast<int>(std::size(kWheelActions));
        int idx = 0;
        for (int i = 0; i < n; ++i) if (kWheelActions[i] == action) idx = i;
        if (const int d = CycleValue(wheelName(action).c_str(), idx, n)) action = kWheelActions[(idx + d + n) % n];
    };
    wheelRow("WHEEL UP", m_edit.wheelUpAction, m_saved.wheelUpAction);
    wheelRow("WHEEL DOWN", m_edit.wheelDownAction, m_saved.wheelDownAction);

    using KeyBindings::ActionGroup;
    const ActionGroup groups[] = { ActionGroup::Movement, ActionGroup::Actions, ActionGroup::Vehicle, ActionGroup::Interface,
                                   ActionGroup::Forge, ActionGroup::Theater };
    // ActionGroup::Other (bindings MCC shares with its other Halo games) is not listed: Reach doesn't
    // use them. They still get their default keys in the profile.
    for (ActionGroup g : groups) {
        const std::vector<int> actions = KeyBindings::ActionsIn(g);
        if (actions.empty()) continue;
        Section(Upper(KeyBindings::GroupName(g)).c_str());
        for (int action : actions) {
            const int def = KeyBindings::DefaultKey(action);
            auto it = m_edit.keyOverrides.find(action);
            const int current = it != m_edit.keyOverrides.end() ? it->second : def;
            auto sit = m_saved.keyOverrides.find(action);
            const int saved = sit != m_saved.keyOverrides.end() ? sit->second : def;
            const char* name = KeyBindings::ActionName(action);
            const std::string label = name ? Upper(name) : "SLOT " + std::to_string(action);
            const std::string defText = def ? KeyBindings::KeyName(def) : std::string("-");
            if (!Row(label.c_str(), "Click (or press Enter), then press the key or mouse button you want. Esc cancels, right-click unbinds, Delete puts the default back.",
                     When::Apply, defText, current != saved)) continue;
            std::string keyText = m_rebindAction == action ? std::string("PRESS A KEY...")
                                : current == 0 ? std::string("\xE2\x80\x94") : KeyBindings::KeyName(current);
            const ImVec2 save = ImGui::GetCursorScreenPos();
            const float bw = std::min(220.0f * s, m_valMax.x - m_valMin.x);
            const ImVec2 b0(m_valMax.x - bw, m_valMin.y), b1(m_valMax.x, m_valMax.y);
            ImGui::SetCursorScreenPos(b0);
            ImGui::PushID(m_curRow);
            const bool clicked = ImGui::InvisibleButton("##key", ImVec2(bw, b1.y - b0.y), ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonRight);
            const bool hovered = ImGui::IsItemHovered();
            const bool right = hovered && ImGui::IsMouseReleased(ImGuiMouseButton_Right);
            ImGui::PopID();
            if (right) {
                if (def == 0) m_edit.keyOverrides.erase(action);
                else m_edit.keyOverrides[action] = 0;
            } else if ((clicked && !ImGui::IsMouseReleased(ImGuiMouseButton_Right)) || (m_rowFocused && m_keyAccept)) {
                m_rebindAction = action;
                m_rebindStarted = ImGui::GetTime();
                m_focus = m_curRow;
            }
            if (m_rowFocused && m_navAllowed && ImGui::IsKeyPressed(ImGuiKey_Delete, false)) m_edit.keyOverrides.erase(action);
            DrawBox(ImGui::GetWindowDrawList(), b0, b1, keyText.c_str(), hovered, current != def || m_rebindAction == action, false);
            ImGui::SetCursorScreenPos(save);
        }
    }
}

void SettingsPanel::RenderController() {
    const SettingsData def;
    const float s = Scale();
    Section("STATUS");
    {
        const int pad = ConnectedPad();
        const std::string line = pad >= 0 ? "Controller " + std::to_string(pad + 1) + " connected" : std::string("No controller found");
        if (Row("CONTROLLER", "Xbox and other XInput controllers work as soon as Windows sees them. Noble switches between the controller and keyboard + mouse by whichever you used last; Start opens the pause menu.",
                When::Info, "", false, false, Preview::Controller)) {
            InfoValue(line.c_str());
        }
    }
    if (Row("BUTTON LAYOUT", "Reach's layouts (Default, Southpaw, Boxer, Green Thumb, Bumper Jumper, Recon). Only Default for now; the others come in a later update.",
            When::Info, "Default", false, true, Preview::Controller)) {
        InfoValue("DEFAULT");
    }
    Section("STICKS");
    auto speed = [&](const char* label, const char* text, int& v, int saved) {
        if (!Row(label, text, When::Apply, "19", v != saved)) return;
        float f = static_cast<float>(v);
        if (SliderValue(f, 1.0f, 19.0f, 1.0f, 19.0f, std::to_string(v).c_str())) v = static_cast<int>(std::lround(f));
    };
    speed("LOOK SENSITIVITY", "How fast the right stick turns you left and right (Reach's own setting; 19 is the stock profile's value).", m_edit.padLookX, m_saved.padLookX);
    speed("VERTICAL SENSITIVITY", "How fast the right stick looks up and down.", m_edit.padLookY, m_saved.padLookY);
    if (Row("INVERT LOOK", "Pushing the right stick forward looks down.", When::Apply, "Off", m_edit.padInvertLook != m_saved.padInvertLook)) {
        ToggleValue(m_edit.padInvertLook);
    }
    if (Row("STICK LAYOUT", "Default puts movement on the left stick and looking on the right. Southpaw swaps them, for players who aim with the left thumb.",
            When::Apply, "Default", m_edit.padStickLayout != m_saved.padStickLayout)) {
        if (const int d = CycleValue(m_edit.padStickLayout == 1 ? "SOUTHPAW" : "DEFAULT", m_edit.padStickLayout, 2)) {
            m_edit.padStickLayout = (m_edit.padStickLayout + d + 2) % 2;
            MenuSounds::Change();
        }
    }
    auto deadZone = [&](const char* label, float& v, float saved) {
        if (!Row(label, "Raise a dead zone if a worn stick drifts. 0 keeps the game's own dead zone.", When::Apply, "Game default",
                 std::fabs(v - saved) > 1e-4f)) return;
        float percent = v * 100.0f;
        const std::string text = percent < 0.5f ? std::string("Game default") : Format("%.0f %%", percent);
        if (SliderValue(percent, 0.0f, 50.0f, 1.0f, 0.0f, text.c_str())) v = percent / 100.0f;
    };
    deadZone("LEFT STICK DEAD ZONE", m_edit.padLeftDeadZone, m_saved.padLeftDeadZone);
    deadZone("RIGHT STICK DEAD ZONE", m_edit.padRightDeadZone, m_saved.padRightDeadZone);
    Section("LAYOUT");
    // Halo: Reach's six button layouts (Halopedia / the 360 manual), applied by Noble to the pad state.
    {
        const int layout = std::clamp(m_edit.padLayout, 0, static_cast<int>(PadLayouts::Count) - 1);
        if (Row("BUTTON LAYOUT", PadLayouts::Description(layout),
                When::Apply, "Default", m_edit.padLayout != m_saved.padLayout)) {
            if (const int d = CycleValue(PadLayouts::Name(layout), m_edit.padLayout,
                                         static_cast<int>(PadLayouts::Count))) {
                m_edit.padLayout = std::clamp(m_edit.padLayout, 0, static_cast<int>(PadLayouts::Count) - 1);
                MenuSounds::Change();
            }
        }
    }

    Section("VIBRATION");
    if (Row("VIBRATION", "Rumble from the game (explosions, vehicles, melee) on Xbox-style controllers.", When::Apply,
            def.padVibration ? "On" : "Off", m_edit.padVibration != m_saved.padVibration)) {
        ToggleValue(m_edit.padVibration);
    }
    if (Row("VIBRATION STRENGTH", "How strong the rumble is.", When::Apply, "100 %",
            std::fabs(m_edit.padVibrationStrength - m_saved.padVibrationStrength) > 1e-4f, !m_edit.padVibration)) {
        SliderValue(m_edit.padVibrationStrength, 0.0f, 1.0f, 0.05f, 1.0f, Percent(m_edit.padVibrationStrength).c_str());
    }
    if (m_search[0] == 0) {
        {
            // Name the layout that is actually in use, so the labels below are not read as "Default" when
            // Southpaw or Boxer is selected.
            char title[96];
            std::snprintf(title, sizeof(title), "HALO: REACH BUTTON LAYOUT - %s",
                          PadLayouts::Name(std::clamp(m_edit.padLayout, 0, static_cast<int>(PadLayouts::Count) - 1)));
            Section(title);
        }
        DrawControllerDiagram(ImGui::GetContentRegionAvail().x, m_edit.padLayout);
    }
    (void)s;
}

void SettingsPanel::RenderOnline() {
    Section("INVITES");
    if (Row("INVITE NOTIFICATIONS", "Show a notification with a JOIN button when a friend invites you to a game, on any menu screen. Turn it off to see invites only in the COMMUNITY screen.",
            When::Apply, "On", m_edit.inviteNotifications != m_saved.inviteNotifications)) {
        ToggleValue(m_edit.inviteNotifications);
    }

    Section("SERVERS");
    if (!Row("MASTER SERVER", "The list of online games for the GAMES screen. Leave it empty to only see games on your LAN.",
             When::Apply, "Noble's test server", m_edit.masterServerUrl != m_saved.masterServerUrl)) return;
    const ImVec2 save = ImGui::GetCursorScreenPos();
    ImGui::SetCursorScreenPos(ImVec2(m_valMin.x, (m_valMin.y + m_valMax.y) * 0.5f - ImGui::GetFrameHeight() * 0.5f));
    ImGui::SetNextItemWidth(m_valMax.x - m_valMin.x);
    ImGui::PushID(m_curRow);
    if (m_rowFocused && m_keyAccept) ImGui::SetKeyboardFocusHere();
    ImGui::InputTextWithHint("##master", "empty = LAN only", m_masterBuf, sizeof(m_masterBuf));
    if (ImGui::IsItemActive()) m_focus = m_curRow;
    ImGui::PopID();
    ImGui::SetCursorScreenPos(save);
}

// Menu music picker: the games on the left, the chosen game's tracks on the right (or every game's
// tracks matching the search text). Clicking a track plays it straight away.
void SettingsPanel::RenderMusicPicker() {
    const float s = Scale();
    const std::vector<ReachAudio::TrackInfo> tracks = ReachAudio::MusicTracks();
    std::vector<std::string> games;
    std::vector<int> counts;
    for (const ReachAudio::TrackInfo& t : tracks) {
        if (t.game.empty()) continue;
        auto it = std::find(games.begin(), games.end(), t.game);
        if (it == games.end()) { games.push_back(t.game); counts.push_back(1); }
        else ++counts[it - games.begin()];
    }
    if (m_musicGame < 0) {
        m_musicGame = 0;
        for (const ReachAudio::TrackInfo& t : tracks) {
            if (t.id == m_edit.menuMusicTrack && !t.game.empty()) {
                m_musicGame = static_cast<int>(std::find(games.begin(), games.end(), t.game) - games.begin());
            }
        }
    }
    if (m_musicGame >= static_cast<int>(games.size())) m_musicGame = 0;
    // Old saved values ("shuffle", piece names) play Reach's menu theme: show that one as chosen.
    if (std::none_of(tracks.begin(), tracks.end(), [&](const ReachAudio::TrackInfo& t) { return t.id == m_edit.menuMusicTrack; }) &&
        std::any_of(tracks.begin(), tracks.end(), [](const ReachAudio::TrackInfo& t) { return t.id == "reach:menu"; })) {
        m_edit.menuMusicTrack = "reach:menu";
    }
    auto pick = [&](const std::string& id) {
        m_edit.menuMusicTrack = id;
        MenuSounds::Click();
        ReachAudio::PlayMenuMusic(id);
    };

    ImGui::Dummy(ImVec2(0, 4.0f * s));
    const float total = ImGui::GetContentRegionAvail().x;
    const float leftW = std::max(150.0f * s, total * 0.40f);
    const float rightW = std::max(150.0f * s, total - leftW - ImGui::GetStyle().ItemSpacing.x);
    const float height = 300.0f * s;
    ImGui::BeginChild("##musicgames", ImVec2(leftW, height), ImGuiChildFlags_Borders);
    for (const ReachAudio::TrackInfo& t : tracks) {
        if (!t.game.empty()) continue;   // "Shuffle all games", "Off"
        const bool selected = t.id == m_edit.menuMusicTrack;
        if (ImGui::Selectable(t.label.c_str(), selected)) pick(t.id);
        if (ImGui::IsItemHovered()) MenuSounds::Hover(ImGui::GetItemID());
    }
    ImGui::Separator();
    for (int i = 0; i < static_cast<int>(games.size()); ++i) {
        char label[160];
        std::snprintf(label, sizeof(label), "%s  (%d)", games[i].c_str(), counts[i]);
        ImGui::PushID(i);
        const bool open = i == m_musicGame && m_musicSearch[0] == 0;
        if (open) ImGui::PushStyleColor(ImGuiCol_Text, NobleStyle::Accent());
        if (ImGui::Selectable(label, open)) {
            m_musicGame = i;
            m_musicSearch[0] = 0;
            MenuSounds::Scroll();
        }
        if (open) ImGui::PopStyleColor();
        if (ImGui::IsItemHovered()) MenuSounds::Hover(ImGui::GetItemID());
        ImGui::PopID();
    }
    ImGui::EndChild();

    ImGui::SameLine();
    ImGui::BeginGroup();
    ImGui::SetNextItemWidth(rightW);
    ImGui::InputTextWithHint("##musicsearch", "Search every game's tracks", m_musicSearch, sizeof(m_musicSearch));
    const std::string needle = Lower(m_musicSearch);
    ImGui::BeginChild("##musictracks", ImVec2(rightW, height - ImGui::GetFrameHeightWithSpacing()), ImGuiChildFlags_Borders);
    int shown = 0;
    std::string lastGame;
    for (const ReachAudio::TrackInfo& t : tracks) {
        if (t.game.empty()) continue;
        if (needle.empty()) {
            if (games.empty() || t.game != games[m_musicGame]) continue;
        } else {
            if (Lower(t.label + " " + t.game).find(needle) == std::string::npos) continue;
            if (t.game != lastGame) {
                lastGame = t.game;
                ImGui::TextColored(NobleStyle::Accent(), "%s", t.game.c_str());
            }
        }
        const bool selected = t.id == m_edit.menuMusicTrack;
        ImGui::PushID(t.id.c_str());
        if (ImGui::Selectable(t.label.c_str(), selected)) pick(t.id);
        if (ImGui::IsItemHovered()) MenuSounds::Hover(ImGui::GetItemID());
        if (selected && m_musicScrollToCurrent) {
            ImGui::SetScrollHereY(0.4f);
            m_musicScrollToCurrent = false;
        }
        ImGui::PopID();
        ++shown;
    }
    if (shown == 0) ImGui::TextDisabled(needle.empty() ? "No tracks found in the MCC install." : "No track matches.");
    ImGui::EndChild();
    ImGui::EndGroup();
}

// ---------------------------------------------------------------------------------------------
// Changes
// ---------------------------------------------------------------------------------------------

int SettingsPanel::ChangesIn(Cat c) const {
    return CountDiff(m_edit, m_saved, c);
}

int SettingsPanel::CountDiff(const SettingsData& a, const SettingsData& b, Cat c) {
    auto f = [](float x, float y) { return std::fabs(x - y) > 1e-4f ? 1 : 0; };
    switch (c) {
    case Cat::Graphics:
        return (a.width != b.width || a.height != b.height) + (a.borderless != b.borderless) + (a.verticalSync != b.verticalSync) +
               (a.fpsLimit != b.fpsLimit) + (a.showFps != b.showFps);
    case Cat::Audio:
        return f(a.volumeGame, b.volumeGame) + f(a.volumeMusic, b.volumeMusic) + f(a.volumeOther, b.volumeOther) +
               (a.masterMute != b.masterMute) + (a.muteInBackground != b.muteInBackground) +
               (a.menuSounds != b.menuSounds) + (a.menuMusicTrack != b.menuMusicTrack);
    case Cat::Game:
        return f(a.fieldOfView, b.fieldOfView) + (a.subtitles != b.subtitles) + (a.menuBackground != b.menuBackground);
    case Cat::Player:
        return (a.playerName != b.playerName) + (a.serviceTag != b.serviceTag);
    case Cat::KeyboardMouse: {
        int n = f(a.mouseSensitivity, b.mouseSensitivity) + f(a.mouseSensitivityY, b.mouseSensitivityY) + (a.invertLook != b.invertLook) +
                (a.wheelUpAction != b.wheelUpAction) + (a.wheelDownAction != b.wheelDownAction);
        for (int i = 0; i < KeyBindings::kActionCount; ++i) {
            auto ia = a.keyOverrides.find(i);
            auto ib = b.keyOverrides.find(i);
            const int ka = ia != a.keyOverrides.end() ? ia->second : KeyBindings::DefaultKey(i);
            const int kb = ib != b.keyOverrides.end() ? ib->second : KeyBindings::DefaultKey(i);
            if (ka != kb) ++n;
        }
        return n;
    }
    case Cat::Controller:
        return (a.padLookX != b.padLookX) + (a.padLookY != b.padLookY) + (a.padInvertLook != b.padInvertLook) +
               (a.padLayout != b.padLayout) + (a.padStickLayout != b.padStickLayout) +
               f(a.padLeftDeadZone, b.padLeftDeadZone) + f(a.padRightDeadZone, b.padRightDeadZone) +
               (a.padVibration != b.padVibration) + f(a.padVibrationStrength, b.padVibrationStrength);
    case Cat::Online:
        return (a.masterServerUrl != b.masterServerUrl) + (a.inviteNotifications != b.inviteNotifications);
    default:
        return 0;
    }
}

int SettingsPanel::TotalChanges() const {
    int n = 0;
    for (int i = 0; i < static_cast<int>(Cat::Count); ++i) n += ChangesIn(static_cast<Cat>(i));
    return n;
}

void SettingsPanel::RestoreDefaults(Cat c) {
    m_edit = WithDefaults(m_edit, c);
    std::snprintf(m_nameBuf, sizeof(m_nameBuf), "%s", m_edit.playerName.c_str());
    std::snprintf(m_tagBuf, sizeof(m_tagBuf), "%s", m_edit.serviceTag.c_str());
    std::snprintf(m_masterBuf, sizeof(m_masterBuf), "%s", m_edit.masterServerUrl.c_str());
}

SettingsData SettingsPanel::WithDefaults(SettingsData e, Cat c) {
    const SettingsData d;
    switch (c) {
    case Cat::Graphics:
        e.width = d.width; e.height = d.height; e.borderless = d.borderless; e.verticalSync = d.verticalSync;
        e.fpsLimit = d.fpsLimit; e.showFps = d.showFps;
        break;
    case Cat::Audio:
        e.volumeGame = d.volumeGame; e.volumeMusic = d.volumeMusic; e.volumeOther = d.volumeOther;
        e.menuSounds = d.menuSounds; e.menuMusicTrack = d.menuMusicTrack;
        break;
    case Cat::Game:
        e.fieldOfView = d.fieldOfView; e.subtitles = d.subtitles; e.menuBackground = d.menuBackground;
        break;
    case Cat::Player:
        e.playerName = d.playerName; e.serviceTag = d.serviceTag;
        break;
    case Cat::KeyboardMouse:
        e.mouseSensitivity = d.mouseSensitivity; e.mouseSensitivityY = d.mouseSensitivityY; e.invertLook = d.invertLook;
        e.wheelUpAction = d.wheelUpAction; e.wheelDownAction = d.wheelDownAction; e.keyOverrides.clear();
        break;
    case Cat::Controller:
        e.padLookX = d.padLookX; e.padLookY = d.padLookY; e.padInvertLook = d.padInvertLook;
        e.padLeftDeadZone = d.padLeftDeadZone; e.padRightDeadZone = d.padRightDeadZone;
        e.padVibration = d.padVibration; e.padVibrationStrength = d.padVibrationStrength;
        break;
    case Cat::Online:
        e.masterServerUrl = d.masterServerUrl;
        break;
    default:
        break;
    }
    return e;
}

void SettingsPanel::Apply() {
    SyncBuffers();
    Settings::Get().Update(m_edit);
    MasterServerClient::Get().SetUrl(m_edit.masterServerUrl);
    // Field of view goes straight into the running engine (haloreach+0xB42C10, degrees); the
    // per-frame CameraFov::Tick keeps it there and picks it up for the next game too.
    CameraFov::Apply(m_edit.fieldOfView);
    m_saved = Settings::Get().Snapshot();  // clamped values
    m_edit = m_saved;
    std::snprintf(m_nameBuf, sizeof(m_nameBuf), "%s", m_edit.playerName.c_str());
    std::snprintf(m_tagBuf, sizeof(m_tagBuf), "%s", m_edit.serviceTag.c_str());
    std::snprintf(m_masterBuf, sizeof(m_masterBuf), "%s", m_edit.masterServerUrl.c_str());
    m_message = "Saved.";
    m_messageTime = ImGui::GetTime();
    MenuSounds::Click();
}

void SettingsPanel::Discard() {
    m_loaded = false;          // reload from the saved settings next frame
    m_rebindAction = -1;
    ReachAudio::PlayMenuMusic(Settings::Get().Snapshot().menuMusicTrack);
    ReachAudio::SetMusicVolume(Settings::Get().Snapshot().volumeMusic);
    m_message = "Changes discarded.";
    m_messageTime = ImGui::GetTime();
    MenuSounds::Back();
}

std::string SettingsPanel::Summary(Cat c) const {
    const SettingsData& e = m_edit;
    char buf[96];
    switch (c) {
    case Cat::Graphics:
        if (e.borderless) std::snprintf(buf, sizeof(buf), "Borderless \xC2\xB7 %s", e.fpsLimit ? (std::to_string(e.fpsLimit) + " FPS").c_str() : "No limit");
        else std::snprintf(buf, sizeof(buf), "%d x %d \xC2\xB7 %s", e.width, e.height, e.fpsLimit ? (std::to_string(e.fpsLimit) + " FPS").c_str() : "No limit");
        return buf;
    case Cat::Audio:
        std::snprintf(buf, sizeof(buf), "Game %.0f %% \xC2\xB7 Music %s", e.volumeGame * 100.0f, e.menuMusicTrack == "off" ? "off" : "on");
        return buf;
    case Cat::Game:
        std::snprintf(buf, sizeof(buf), "%.0f\xC2\xB0 FOV \xC2\xB7 Subtitles %s", e.fieldOfView, e.subtitles ? "on" : "off");
        return buf;
    case Cat::Player:
        std::snprintf(buf, sizeof(buf), "%s [%s]", m_nameBuf, m_tagBuf);
        return buf;
    case Cat::KeyboardMouse: {
        int rebound = 0;
        for (const auto& [action, key] : e.keyOverrides) if (key != KeyBindings::DefaultKey(action)) ++rebound;
        std::snprintf(buf, sizeof(buf), "Sensitivity %.2f \xC2\xB7 %d rebound", e.mouseSensitivity, rebound);
        return buf;
    }
    case Cat::Controller: {
        const int pad = ConnectedPad();
        return pad >= 0 ? std::string("Controller connected") : std::string("No controller connected");
    }
    case Cat::Online:
        return e.masterServerUrl.empty() ? std::string("LAN only") : std::string("Master server set");
    case Cat::System:
        return std::string(VERSION_STRING);
    default:
        return {};
    }
}


void SettingsPanel::RenderSystem() {
    const std::string version(VERSION_STRING);
    const std::filesystem::path settingsFile = Settings::Get().FilePath();
    const std::filesystem::path exeDir = settingsFile.parent_path().parent_path().parent_path();
    const std::filesystem::path userData = settingsFile.parent_path();
    const std::filesystem::path backups = userData / "backups";
    static std::string s_note;

    // Action rows: Row() is a layout helper that always returns true, so the click test is explicit (see the
    // 2026-10-30 09:35 entry). The outcome of the last action is shown under the rows.
    auto action = [&](const char* label, const char* description, const char* value, auto&& run) {
        if (!Row(label, description, When::Instant, value, false)) return;
        InfoValue(value);
        const bool clicked = m_rowHovered && ImGui::IsMouseClicked(0);
        const bool accepted = m_rowFocused && (ImGui::IsKeyPressed(ImGuiKey_Enter, false) ||
                                               ImGui::IsKeyPressed(ImGuiKey_GamepadFaceDown, false));
        if (!clicked && !accepted) return;
        MenuSounds::Click();
        run();
    };
    auto copyFile = [](const std::filesystem::path& from, const std::filesystem::path& to) {
        std::error_code ec;
        if (!std::filesystem::is_regular_file(from, ec)) return false;
        std::filesystem::create_directories(to.parent_path(), ec);
        std::filesystem::copy_file(from, to, std::filesystem::copy_options::overwrite_existing, ec);
        return !ec;
    };

    Section("NOBLE");
    if (Row("VERSION", "The build you are running. Worth quoting when you report a problem.", When::Info, version, false, true)) {
        InfoValue(version.c_str());
    }
    if (Row("LOG FILE", "noble-client.log, next to the program. It is what a bug report needs.", When::Info, "noble-client.log", false, true)) {
        InfoValue("noble-client.log");
    }
    action("OPEN LOGS FOLDER", "Opens the folder that holds the log, the crash dump and this settings file.", "Explorer",
           [&] {
               ShellExecuteW(nullptr, L"open", exeDir.wstring().c_str(), nullptr, nullptr, SW_SHOWNORMAL);
               s_note = "Opened " + exeDir.string();
           });
    action("COPY DIAGNOSTICS", "Puts the version, the install path and the last 40 log lines on the clipboard, ready to paste into a report.",
           "Clipboard", [&] {
               std::string text = std::string("Project Noble ") + version + "\n" + exeDir.string() + "\n";
               std::ifstream log(exeDir / "noble-client.log");
               if (log) {
                   std::vector<std::string> lines;
                   for (std::string line; std::getline(log, line);) {
                       lines.push_back(line);
                       if (lines.size() > 40) lines.erase(lines.begin());
                   }
                   text += "\n--- last log lines ---\n";
                   for (const std::string& line : lines) text += line + "\n";
               }
               ImGui::SetClipboardText(text.c_str());
               s_note = "Diagnostics copied to the clipboard.";
           });

    Section("BACKUPS");
    const bool haveBackup = std::filesystem::is_directory(backups);
    action("BACK UP SETTINGS", "Copies settings.json and appearance.json into data\\userdata\\backups\\<date-time>, so they can be put back later.",
           haveBackup ? "Backups exist" : "None yet", [&] {
               SYSTEMTIME now{};
               GetLocalTime(&now);
               char stamp[64];
               std::snprintf(stamp, sizeof(stamp), "%04u-%02u-%02u_%02u%02u%02u", now.wYear, now.wMonth, now.wDay,
                             now.wHour, now.wMinute, now.wSecond);
               const std::filesystem::path dir = backups / stamp;
               int copied = 0;
               if (copyFile(userData / "settings.json", dir / "settings.json")) ++copied;
               if (copyFile(userData / "appearance.json", dir / "appearance.json")) ++copied;
               if (copied == 0) {
                   s_note = "Nothing to back up yet.";
                   MenuSounds::Error();
               } else {
                   s_note = "Backed up to " + dir.string();
               }
           });
    action("RESTORE NEWEST BACKUP", "Puts the most recent backup back and reloads it. Anything unsaved is lost.", "Reloads now", [&] {
        std::error_code ec;
        std::filesystem::path newest;
        for (const auto& entry : std::filesystem::directory_iterator(backups, ec)) {
            if (!entry.is_directory(ec)) continue;
            if (newest.empty() || entry.path().filename() > newest.filename()) newest = entry.path();
        }
        if (newest.empty()) {
            s_note = "No backup to restore yet.";
            MenuSounds::Error();
            return;
        }
        int restored = 0;
        if (copyFile(newest / "settings.json", userData / "settings.json")) ++restored;
        if (copyFile(newest / "appearance.json", userData / "appearance.json")) ++restored;
        if (restored == 0) {
            s_note = "That backup is empty.";
            MenuSounds::Error();
            return;
        }
        Settings::Get().Load();
        m_saved = Settings::Get().Snapshot();
        m_edit = m_saved;
        SyncBuffers();
        s_note = "Restored " + newest.filename().string() + " and reloaded it.";
    });
    action("OPEN BACKUP FOLDER", "Shows the backups on disk, so an old one can be copied back by hand.", "Explorer", [&] {
        std::error_code ec;
        std::filesystem::create_directories(backups, ec);
        ShellExecuteW(nullptr, L"open", backups.wstring().c_str(), nullptr, nullptr, SW_SHOWNORMAL);
        s_note = "Opened " + backups.string();
    });

    if (!s_note.empty()) {
        ImGui::Dummy(ImVec2(0, 8.0f * Scale()));
        ImGui::PushStyleColor(ImGuiCol_Text, NobleStyle::TextMuted());
        ImGui::TextWrapped("%s", s_note.c_str());
        ImGui::PopStyleColor();
    }
}

} // namespace noble
