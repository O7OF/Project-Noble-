#include "NobleStyle.hpp"
#include "MenuSounds.hpp"

#include <windows.h>
#include <filesystem>
#include <string>

namespace noble {
namespace NobleStyle {

namespace {

ImFont* s_titleFont = nullptr;
ImFont* s_heroFont = nullptr;
ImFont* s_smallFont = nullptr;

// Reach-like palette.
const ImVec4 kBg        = ImVec4(0.035f, 0.050f, 0.070f, 0.97f);
const ImVec4 kPanel     = ImVec4(0.065f, 0.090f, 0.120f, 0.92f);
const ImVec4 kPanelHi   = ImVec4(0.100f, 0.135f, 0.175f, 0.95f);
const ImVec4 kBorder    = ImVec4(0.300f, 0.420f, 0.520f, 0.55f);
const ImVec4 kAccent    = ImVec4(0.560f, 0.850f, 1.000f, 1.00f);
const ImVec4 kAccentDim = ImVec4(0.220f, 0.470f, 0.620f, 1.00f);
const ImVec4 kText      = ImVec4(0.900f, 0.940f, 0.970f, 1.00f);
const ImVec4 kMuted     = ImVec4(0.520f, 0.600f, 0.660f, 1.00f);

std::string FontPath(const char* file) {
    wchar_t windir[MAX_PATH] = {};
    GetWindowsDirectoryW(windir, MAX_PATH);
    const std::filesystem::path p = std::filesystem::path(windir) / "Fonts" / file;
    std::error_code ec;
    return std::filesystem::exists(p, ec) ? p.string() : std::string();
}

void LoadFonts() {
    ImGuiIO& io = ImGui::GetIO();
    // Body font: Bahnschrift (Windows 10+), else Segoe UI, else ImGui's built-in font.
    std::string body = FontPath("bahnschrift.ttf");
    if (body.empty()) body = FontPath("segoeui.ttf");
    // The menu renders at up to 1.5x the display and the UI is drawn in back-buffer pixels, so glyphs
    // are used at up to ~2.4x their logical size. Rasterise the atlas at 2.5x density while keeping the
    // logical metrics: text stays crisp at every UI scale instead of being stretched from an 18 px atlas.
    ImFontConfig fontConfig;
    fontConfig.RasterizerDensity = 2.5f;
    if (!body.empty() && io.Fonts->AddFontFromFileTTF(body.c_str(), 18.0f, &fontConfig)) {
        s_titleFont = io.Fonts->AddFontFromFileTTF(body.c_str(), 30.0f, &fontConfig);
        s_heroFont = io.Fonts->AddFontFromFileTTF(body.c_str(), 56.0f, &fontConfig);
        s_smallFont = io.Fonts->AddFontFromFileTTF(body.c_str(), 15.0f, &fontConfig);
    } else {
        io.Fonts->AddFontDefault(&fontConfig);
        s_titleFont = nullptr;
        s_heroFont = nullptr;
        s_smallFont = nullptr;
    }
}

} // namespace

ImVec4 Accent() { return kAccent; }
ImVec4 AccentDim() { return kAccentDim; }
ImVec4 TextMuted() { return kMuted; }
ImFont* TitleFont() { return s_titleFont; }
ImFont* HeroFont() { return s_heroFont; }
ImFont* SmallFont() { return s_smallFont; }

void Apply() {
    ImGuiStyle& style = ImGui::GetStyle();
    // Reach's menus are angular: no rounding, thin borders, generous spacing.
    style.WindowRounding = 0.0f;
    style.ChildRounding = 0.0f;
    style.FrameRounding = 0.0f;
    style.PopupRounding = 0.0f;
    style.ScrollbarRounding = 0.0f;
    style.GrabRounding = 0.0f;
    style.TabRounding = 0.0f;
    style.WindowBorderSize = 1.0f;
    style.ChildBorderSize = 1.0f;
    style.FrameBorderSize = 1.0f;
    style.PopupBorderSize = 1.0f;
    style.TabBorderSize = 0.0f;
    style.WindowPadding = ImVec2(16.0f, 14.0f);
    style.FramePadding = ImVec2(10.0f, 6.0f);
    style.ItemSpacing = ImVec2(10.0f, 8.0f);
    style.ItemInnerSpacing = ImVec2(8.0f, 6.0f);
    style.ScrollbarSize = 12.0f;
    style.GrabMinSize = 12.0f;
    style.WindowTitleAlign = ImVec2(0.0f, 0.5f);

    ImVec4* c = style.Colors;
    c[ImGuiCol_Text]                 = kText;
    c[ImGuiCol_TextDisabled]         = kMuted;
    c[ImGuiCol_WindowBg]             = kBg;
    c[ImGuiCol_ChildBg]              = kPanel;
    c[ImGuiCol_PopupBg]              = ImVec4(0.05f, 0.07f, 0.10f, 0.98f);
    c[ImGuiCol_Border]               = kBorder;
    c[ImGuiCol_BorderShadow]         = ImVec4(0, 0, 0, 0);
    c[ImGuiCol_FrameBg]              = ImVec4(0.08f, 0.11f, 0.15f, 0.90f);
    c[ImGuiCol_FrameBgHovered]       = kPanelHi;
    c[ImGuiCol_FrameBgActive]        = ImVec4(0.14f, 0.24f, 0.32f, 1.00f);
    c[ImGuiCol_TitleBg]              = kPanel;
    c[ImGuiCol_TitleBgActive]        = kPanelHi;
    c[ImGuiCol_TitleBgCollapsed]     = kPanel;
    c[ImGuiCol_MenuBarBg]            = kPanel;
    c[ImGuiCol_ScrollbarBg]          = ImVec4(0.03f, 0.04f, 0.06f, 0.60f);
    c[ImGuiCol_ScrollbarGrab]        = ImVec4(0.22f, 0.30f, 0.38f, 1.00f);
    c[ImGuiCol_ScrollbarGrabHovered] = kAccentDim;
    c[ImGuiCol_ScrollbarGrabActive]  = kAccent;
    c[ImGuiCol_CheckMark]            = kAccent;
    c[ImGuiCol_SliderGrab]           = kAccentDim;
    c[ImGuiCol_SliderGrabActive]     = kAccent;
    c[ImGuiCol_Button]               = ImVec4(0.10f, 0.14f, 0.19f, 1.00f);
    c[ImGuiCol_ButtonHovered]        = ImVec4(0.18f, 0.30f, 0.40f, 1.00f);
    c[ImGuiCol_ButtonActive]         = ImVec4(0.26f, 0.44f, 0.56f, 1.00f);
    c[ImGuiCol_Header]               = ImVec4(0.14f, 0.24f, 0.32f, 1.00f);
    c[ImGuiCol_HeaderHovered]        = ImVec4(0.18f, 0.30f, 0.40f, 0.90f);
    c[ImGuiCol_HeaderActive]         = ImVec4(0.26f, 0.44f, 0.56f, 1.00f);
    c[ImGuiCol_Separator]            = kBorder;
    c[ImGuiCol_SeparatorHovered]     = kAccentDim;
    c[ImGuiCol_SeparatorActive]      = kAccent;
    c[ImGuiCol_ResizeGrip]           = ImVec4(0, 0, 0, 0);
    c[ImGuiCol_ResizeGripHovered]    = kAccentDim;
    c[ImGuiCol_ResizeGripActive]     = kAccent;
    c[ImGuiCol_Tab]                  = ImVec4(0.06f, 0.08f, 0.11f, 0.90f);
    c[ImGuiCol_TabHovered]           = ImVec4(0.18f, 0.30f, 0.40f, 1.00f);
    c[ImGuiCol_TabSelected]          = ImVec4(0.14f, 0.24f, 0.32f, 1.00f);
    c[ImGuiCol_TabDimmed]            = c[ImGuiCol_Tab];
    c[ImGuiCol_TabDimmedSelected]    = c[ImGuiCol_TabSelected];
    c[ImGuiCol_TableHeaderBg]        = ImVec4(0.08f, 0.11f, 0.15f, 1.00f);
    c[ImGuiCol_TableBorderStrong]    = kBorder;
    c[ImGuiCol_TableBorderLight]     = ImVec4(0.16f, 0.22f, 0.28f, 1.00f);
    c[ImGuiCol_TableRowBg]           = ImVec4(0, 0, 0, 0);
    c[ImGuiCol_TableRowBgAlt]        = ImVec4(1.0f, 1.0f, 1.0f, 0.025f);
    c[ImGuiCol_TextSelectedBg]       = ImVec4(0.26f, 0.44f, 0.56f, 0.60f);
    c[ImGuiCol_NavCursor]         = kAccent;
    c[ImGuiCol_ModalWindowDimBg]     = ImVec4(0.0f, 0.0f, 0.0f, 0.65f);

    LoadFonts();
}

void Header(const char* text) {
    if (s_titleFont) ImGui::PushFont(s_titleFont);
    ImGui::TextColored(kAccent, "%s", text);
    if (s_titleFont) ImGui::PopFont();
    const ImVec2 p = ImGui::GetCursorScreenPos();
    const float w = ImGui::GetContentRegionAvail().x;
    ImGui::GetWindowDrawList()->AddLine(ImVec2(p.x, p.y), ImVec2(p.x + w, p.y), ImGui::GetColorU32(kAccentDim), 1.0f);
    ImGui::Dummy(ImVec2(0.0f, 6.0f));
}

void SectionGap() {
    ImGui::Dummy(ImVec2(0.0f, 4.0f));
    ImGui::Separator();
    ImGui::Dummy(ImVec2(0.0f, 4.0f));
}

static bool ColoredButton(const char* label, const ImVec2& size, const ImVec4& base, const ImVec4& hover,
                          const ImVec4& active, const ImVec4& text) {
    ImGui::PushStyleColor(ImGuiCol_Button, base);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, hover);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, active);
    ImGui::PushStyleColor(ImGuiCol_Text, text);
    ImGui::PushStyleColor(ImGuiCol_Border, hover);
    const bool pressed = ImGui::Button(label, size);
    ImGui::PopStyleColor(5);
    if (ImGui::IsItemHovered() && ImGui::IsItemActivated() == false) MenuSounds::Hover(ImGui::GetItemID());
    if (pressed) MenuSounds::Click();
    return pressed;
}

bool PrimaryButton(const char* label, const ImVec2& size) {
    return ColoredButton(label, size, ImVec4(0.16f, 0.34f, 0.46f, 1.0f), ImVec4(0.26f, 0.50f, 0.66f, 1.0f),
                         ImVec4(0.40f, 0.68f, 0.86f, 1.0f), ImVec4(1, 1, 1, 1));
}

bool DangerButton(const char* label, const ImVec2& size) {
    return ColoredButton(label, size, ImVec4(0.32f, 0.10f, 0.10f, 1.0f), ImVec4(0.50f, 0.16f, 0.16f, 1.0f),
                         ImVec4(0.66f, 0.22f, 0.22f, 1.0f), ImVec4(1, 1, 1, 1));
}

} // namespace NobleStyle
} // namespace noble
