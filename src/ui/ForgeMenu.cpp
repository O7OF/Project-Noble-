#include "ForgeMenu.hpp"
#include "NobleStyle.hpp"
#include "MenuSounds.hpp"
#include "PauseMenu.hpp"
#include "common/Logger.hpp"
#include "engine/BlamEngine.hpp"
#include "engine/CameraThirdPerson.hpp"

#include <imgui.h>
#include <nlohmann/json.hpp>

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <shellapi.h>

namespace noble {

bool ForgeMenu::s_open = false;
bool ForgeMenu::s_demo = false;
bool ForgeMenu::s_initialized = false;
bool ForgeMenu::s_demoOpened = false;
int ForgeMenu::s_tab = 0;
int ForgeMenu::s_item = 0;
std::vector<ForgeMenu::Tab> ForgeMenu::s_tabs;
std::string ForgeMenu::s_status;
unsigned long long ForgeMenu::s_statusAt = 0;
ForgeMenu::PlaceHandler ForgeMenu::s_place;

namespace {
constexpr ImGuiWindowFlags kPanelFlags =
    ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
    ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings;

const ImVec4 kBody = ImVec4(0.86f, 0.90f, 0.94f, 1.0f);
const ImVec4 kDim = ImVec4(0.55f, 0.62f, 0.70f, 1.0f);

bool HasDemoFlag() {
    int argc = 0;
    LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);
    bool demo = false;
    for (int i = 1; argv && i < argc; ++i) {
        if (_wcsicmp(argv[i], L"-forge-menu-demo") == 0) { demo = true; break; }
    }
    if (argv) LocalFree(argv);
    return demo;
}
} // namespace

void ForgeMenu::AddFallbackTabs() {
    Tab noble;
    noble.name = "NOBLE";
    noble.items = {
        { "PELICAN", "o7of_tags\\vehicles\\pelican_o7\\pelican_mp",
          "Noble Pelican. Driver seat uses the wraith_d label.", 16, 0.0f },
        { "PHANTOM", "objects\\vehicles\\covenant\\phantom\\phantom_mp",
          "Covenant Phantom dropship.", 16, 0.0f },
    };
    s_tabs = { noble };
}

void ForgeMenu::Initialize(const std::wstring& dataDir) {
    if (s_initialized) return;
    s_initialized = true;
    s_demo = HasDemoFlag();
    s_tabs.clear();

    std::ifstream file(std::filesystem::path(dataDir) / "forge_palette.json", std::ios::binary);
    if (file.is_open()) {
        try {
            const nlohmann::json j = nlohmann::json::parse(file);
            for (const auto& tabJson : j.value("tabs", nlohmann::json::array())) {
                Tab tab;
                tab.name = tabJson.value("name", "FORGE");
                for (const auto& itemJson : tabJson.value("items", nlohmann::json::array())) {
                    Item item;
                    item.name = itemJson.value("name", "ITEM");
                    item.tag = itemJson.value("tag", "");
                    item.description = itemJson.value("description", "");
                    item.maxAllowed = itemJson.value("max", 16);
                    item.price = itemJson.value("price", 0.0f);
                    if (!item.tag.empty()) tab.items.push_back(std::move(item));
                }
                if (!tab.items.empty()) s_tabs.push_back(std::move(tab));
            }
        } catch (const std::exception& e) {
            NOBLE_WARN("ForgeMenu: forge_palette.json parse failed: {}", e.what());
        }
    }
    if (s_tabs.empty()) {
        AddFallbackTabs();
        NOBLE_INFO("ForgeMenu: using built-in palette (data\\forge_palette.json not found)");
    } else {
        NOBLE_INFO("ForgeMenu: loaded {} tabs from forge_palette.json", s_tabs.size());
    }
    ResetSelection();
}

bool ForgeMenu::IsOpen() { return s_open; }
bool ForgeMenu::WantsPointer() { return s_open; }

void ForgeMenu::Close() {
    if (s_open) NOBLE_INFO("ForgeMenu: closed");
    s_open = false;
}

bool ForgeMenu::CanOpen() {
    auto* engine = BlamEngine::Active();
    if (!engine || !engine->IsForgeMode()) return false;
    // Forge starts in player mode (first person). The monitor/editor mode is the engine's own third-person state.
    return CameraThirdPerson::EngineWantsThirdPerson();
}

void ForgeMenu::Toggle() {
    if (s_tabs.empty()) return;
    if (!s_open && !s_demo && !CanOpen()) {
        NOBLE_INFO("ForgeMenu: open ignored (only available in Forge monitor mode)");
        return;
    }
    s_open = !s_open;
    if (s_open) {
        NOBLE_INFO("ForgeMenu: opened");
        ResetSelection();
        MenuSounds::Click();
    } else {
        NOBLE_INFO("ForgeMenu: closed");
    }
}

void ForgeMenu::SetPlaceHandler(PlaceHandler handler) { s_place = std::move(handler); }

void ForgeMenu::ResetSelection() {
    s_tab = std::clamp(s_tab, 0, std::max(0, static_cast<int>(s_tabs.size()) - 1));
    if (!s_tabs.empty()) s_item = std::clamp(s_item, 0, std::max(0, static_cast<int>(s_tabs[s_tab].items.size()) - 1));
}

void ForgeMenu::Tick(bool loading) {
    if (!s_initialized) return;
    if (s_demo && !s_demoOpened && !loading && !PauseMenu::IsOpen()) {
        s_demoOpened = true;
        s_open = true;
        NOBLE_INFO("ForgeMenu: demo palette opened");
    }
}

void ForgeMenu::DrawOverlay() {
    if (s_open) Draw();
}

void ForgeMenu::Draw() {
    ImGuiIO& io = ImGui::GetIO();
    ImGui::GetBackgroundDrawList()->AddRectFilled(ImVec2(0, 0), io.DisplaySize, IM_COL32(0, 0, 0, 90));

    const float s = std::clamp(io.DisplaySize.y / 720.0f, 0.8f, 2.0f);
    const ImVec2 size(640.0f * s, 330.0f * s);
    const ImVec2 pos(io.DisplaySize.x - size.x - 28.0f * s, io.DisplaySize.y - size.y - 28.0f * s);
    ImGui::SetNextWindowSize(size, ImGuiCond_Always);
    ImGui::SetNextWindowPos(pos, ImGuiCond_Always);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.035f, 0.060f, 0.085f, 0.94f));
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.24f, 0.42f, 0.58f, 0.9f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(16.0f * s, 12.0f * s));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1.0f);
    if (!ImGui::Begin("##NobleForgeMenu", nullptr, kPanelFlags)) {
        ImGui::End();
        ImGui::PopStyleVar(2);
        ImGui::PopStyleColor(2);
        return;
    }

    // Header: small, Reach-like, no giant title.
    ImGui::PushFont(NobleStyle::TitleFont());
    ImGui::TextUnformatted("NOBLE FORGE");
    ImGui::PopFont();
    ImGui::SameLine();
    ImGui::PushStyleColor(ImGuiCol_Text, NobleStyle::TextMuted());
    ImGui::TextUnformatted("  custom objects");
    ImGui::PopStyleColor();
    ImGui::SameLine(ImGui::GetWindowWidth() - 44.0f * s);
    if (ImGui::SmallButton("X")) Close();
    ImGui::Separator();
    // Keep the Windows arrow visible while this menu owns the mouse.
    SetCursor(LoadCursorW(nullptr, IDC_ARROW));
    ImGui::SetMouseCursor(ImGuiMouseCursor_Arrow);

    const float bodyH = 222.0f * s;
    const float tabW = 150.0f * s;
    const float gap = 12.0f * s;

    // Category list (only the tabs in forge_palette.json: our custom stuff).
    ImGui::BeginChild("##forgecats", ImVec2(tabW, bodyH), ImGuiChildFlags_Borders);
    for (int i = 0; i < static_cast<int>(s_tabs.size()); ++i) {
        ImGui::PushID(100 + i);
        const bool focused = i == s_tab;
        ImGui::PushStyleColor(ImGuiCol_Text, focused ? NobleStyle::Accent() : kBody);
        if (ImGui::Selectable(s_tabs[i].name.c_str(), focused, 0, ImVec2(tabW - 12.0f * s, 30.0f * s))) {
            s_tab = i;
            s_item = 0;
            MenuSounds::Click();
        }
        if (ImGui::IsItemHovered()) MenuSounds::Hover(ImGui::GetItemID());
        ImGui::PopStyleColor();
        ImGui::PopID();
    }
    ImGui::EndChild();

    ImGui::SameLine(0.0f, gap);
    const float listW = 210.0f * s;
    ImGui::BeginGroup();
    ImGui::BeginChild("##forgeitems", ImVec2(listW, bodyH), ImGuiChildFlags_Borders);
    static const std::vector<Item> kEmpty;
    const auto& items = s_tabs.empty() ? kEmpty : s_tabs[s_tab].items;
    for (int i = 0; i < static_cast<int>(items.size()); ++i) {
        ImGui::PushID(200 + i);
        const bool focused = i == s_item;
        ImGui::PushStyleColor(ImGuiCol_Text, focused ? NobleStyle::Accent() : kBody);
        if (ImGui::Selectable(items[i].name.c_str(), focused, 0, ImVec2(listW - 12.0f * s, 30.0f * s))) {
            s_item = i;
            MenuSounds::Click();
        }
        if (ImGui::IsItemHovered()) {
            MenuSounds::Hover(ImGui::GetItemID());
            s_item = i;
        }
        ImGui::PopStyleColor();
        ImGui::PopID();
    }
    ImGui::EndChild();
    ImGui::EndGroup();

    // Details + place button, on the right of the item list.
    ImGui::SameLine(0.0f, gap);
    ImGui::BeginGroup();
    ImGui::BeginChild("##forgedetails", ImVec2(0, bodyH), ImGuiChildFlags_Borders);
    if (!items.empty()) {
        const Item& item = items[std::clamp(s_item, 0, static_cast<int>(items.size()) - 1)];
        ImGui::PushFont(NobleStyle::TitleFont());
        ImGui::TextUnformatted(item.name.c_str());
        ImGui::PopFont();
        ImGui::PushStyleColor(ImGuiCol_Text, kBody);
        ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + 220.0f * s);
        ImGui::TextWrapped("%s", item.description.empty() ? "No description." : item.description.c_str());
        ImGui::PopTextWrapPos();
        ImGui::PopStyleColor();
        ImGui::Spacing();
        ImGui::PushStyleColor(ImGuiCol_Text, kDim);
        ImGui::Text("MAX %d   COST %.0f", item.maxAllowed, item.price);
        ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + 220.0f * s);
        ImGui::TextWrapped("%s", item.tag.c_str());
        ImGui::PopTextWrapPos();
        ImGui::PopStyleColor();
        ImGui::Spacing();
        if (NobleStyle::PrimaryButton("PLACE", ImVec2(180.0f * s, 38.0f * s))) {
            if (s_place) {
                const bool queued = s_place(item);
                s_status = queued ? "Placement queued." : "Engine create hook pending.";
            } else {
                s_status = "No placement handler installed.";
            }
            s_statusAt = GetTickCount64();
            MenuSounds::Click();
        }
        if (!s_status.empty() && GetTickCount64() - s_statusAt < 6000) {
            ImGui::PushStyleColor(ImGuiCol_Text, kDim);
            ImGui::TextWrapped("%s", s_status.c_str());
            ImGui::PopStyleColor();
        }
    } else {
        ImGui::TextUnformatted("No custom items in this category.");
    }
    ImGui::EndChild();
    ImGui::EndGroup();

    // Footer hints, controller and mouse.
    ImGui::PushStyleColor(ImGuiCol_Text, kDim);
    ImGui::TextUnformatted("INSERT close   A place   B close   D-pad navigate   mouse");
    ImGui::PopStyleColor();

    // Controller navigation through ImGui's gamepad keys (fed by UiPad).
    const bool navUp = ImGui::IsKeyPressed(ImGuiKey_GamepadDpadUp, true) || ImGui::IsKeyPressed(ImGuiKey_UpArrow, true);
    const bool navDown = ImGui::IsKeyPressed(ImGuiKey_GamepadDpadDown, true) || ImGui::IsKeyPressed(ImGuiKey_DownArrow, true);
    const bool navLeft = ImGui::IsKeyPressed(ImGuiKey_GamepadDpadLeft, true) || ImGui::IsKeyPressed(ImGuiKey_LeftArrow, true);
    const bool navRight = ImGui::IsKeyPressed(ImGuiKey_GamepadDpadRight, true) || ImGui::IsKeyPressed(ImGuiKey_RightArrow, true);
    if (!items.empty()) {
        if (navDown) { s_item = (s_item + 1) % static_cast<int>(items.size()); MenuSounds::Scroll(); }
        if (navUp) { s_item = (s_item + static_cast<int>(items.size()) - 1) % static_cast<int>(items.size()); MenuSounds::Scroll(); }
    }
    if (!s_tabs.empty()) {
        if (navRight) { s_tab = (s_tab + 1) % static_cast<int>(s_tabs.size()); s_item = 0; MenuSounds::Tab(1); }
        if (navLeft) { s_tab = (s_tab + static_cast<int>(s_tabs.size()) - 1) % static_cast<int>(s_tabs.size()); s_item = 0; MenuSounds::Tab(-1); }
    }
    if (ImGui::IsKeyPressed(ImGuiKey_GamepadFaceDown, false) && !items.empty() && s_place) {
        s_status = s_place(items[s_item]) ? "Placement queued." : "Engine create hook pending.";
        s_statusAt = GetTickCount64();
        MenuSounds::Click();
    }
    if (ImGui::IsKeyPressed(ImGuiKey_GamepadFaceRight, false)) Close();
    if (ImGui::IsKeyPressed(ImGuiKey_Escape, false)) Close();

    ImGui::End();
    ImGui::PopStyleVar(2);
    ImGui::PopStyleColor(2);
}

} // namespace noble
