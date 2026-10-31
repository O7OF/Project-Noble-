#include "GamepadUi.hpp"
#include "UIManager.hpp"
#include "InputPrompts.hpp"
#include "NobleStyle.hpp"
#include "common/Version.hpp"
#include <imgui.h>
#include <imgui_impl_win32.h>
#include <imgui_impl_dx11.h>

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace noble {

bool UIManager::s_isInitialized = false;
UIView UIManager::s_activeView = UIView::MissionLauncher;
ID3D11Device* UIManager::s_device = nullptr;
ID3D11DeviceContext* UIManager::s_context = nullptr;
std::unique_ptr<MissionLauncher> UIManager::s_missionLauncher;
std::unique_ptr<ServerBrowser> UIManager::s_serverBrowser;
std::unique_ptr<ArmorCustomizer> UIManager::s_armorCustomizer;
std::unique_ptr<WorkshopManager> UIManager::s_workshopManager;

bool UIManager::Initialize(HWND hwnd, ID3D11Device* device, ID3D11DeviceContext* context) {
    if (s_isInitialized) return true;

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    // Controller navigation in the menus (D-pad / left stick to move, A to select, B to go back);
    // ImGui's Win32 backend reads the pad through XInput. Replaces the focus-neighbour code the
    // partner's settings_manager.gd needed in Godot.
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
    io.IniFilename = nullptr;  // don't write imgui.ini next to the exe

    // One shared look for all Noble screens (ui/NobleStyle.cpp).
    NobleStyle::Apply();

    if (!ImGui_ImplWin32_Init(hwnd)) {
        return false;
    }

    if (!ImGui_ImplDX11_Init(device, context)) {
        ImGui_ImplWin32_Shutdown();
        return false;
    }

    s_device = device;
    s_context = context;
    s_missionLauncher = std::make_unique<MissionLauncher>();
    s_serverBrowser = std::make_unique<ServerBrowser>();
    s_armorCustomizer = std::make_unique<ArmorCustomizer>();
    s_workshopManager = std::make_unique<WorkshopManager>();

    s_isInitialized = true;
    return true;
}

void UIManager::Shutdown() {
    if (!s_isInitialized) return;

    s_serverBrowser.reset();
    s_armorCustomizer.reset();
    s_workshopManager.reset();

    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    s_device = nullptr;
    s_context = nullptr;
    s_isInitialized = false;
}

void UIManager::ApplyReachTheme() {
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 4.0f;
    style.ChildRounding = 3.0f;
    style.FrameRounding = 3.0f;
    style.PopupRounding = 3.0f;
    style.ScrollbarRounding = 4.0f;
    style.GrabRounding = 3.0f;
    style.TabRounding = 3.0f;

    style.WindowBorderSize = 1.0f;
    style.FrameBorderSize = 1.0f;
    style.PopupBorderSize = 1.0f;

    style.WindowPadding = ImVec2(10.0f, 10.0f);
    style.FramePadding = ImVec2(6.0f, 4.0f);
    style.ItemSpacing = ImVec2(8.0f, 6.0f);

    ImVec4* colors = style.Colors;
    colors[ImGuiCol_Text]                  = ImVec4(0.92f, 0.94f, 0.96f, 1.00f);
    colors[ImGuiCol_TextDisabled]          = ImVec4(0.48f, 0.52f, 0.58f, 1.00f);
    colors[ImGuiCol_WindowBg]              = ImVec4(0.06f, 0.08f, 0.11f, 0.96f);
    colors[ImGuiCol_ChildBg]               = ImVec4(0.09f, 0.12f, 0.16f, 0.85f);
    colors[ImGuiCol_PopupBg]               = ImVec4(0.07f, 0.09f, 0.13f, 0.98f);
    colors[ImGuiCol_Border]                = ImVec4(0.18f, 0.25f, 0.34f, 0.70f);
    colors[ImGuiCol_BorderShadow]          = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_FrameBg]               = ImVec4(0.11f, 0.15f, 0.20f, 0.80f);
    colors[ImGuiCol_FrameBgHovered]        = ImVec4(0.16f, 0.22f, 0.30f, 0.90f);
    colors[ImGuiCol_FrameBgActive]         = ImVec4(0.00f, 0.60f, 0.80f, 0.50f);
    colors[ImGuiCol_TitleBg]               = ImVec4(0.08f, 0.11f, 0.15f, 1.00f);
    colors[ImGuiCol_TitleBgActive]         = ImVec4(0.12f, 0.18f, 0.25f, 1.00f);
    colors[ImGuiCol_MenuBarBg]             = ImVec4(0.08f, 0.11f, 0.15f, 1.00f);
    colors[ImGuiCol_ScrollbarBg]           = ImVec4(0.06f, 0.08f, 0.10f, 0.60f);
    colors[ImGuiCol_ScrollbarGrab]         = ImVec4(0.20f, 0.28f, 0.36f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabHovered]  = ImVec4(0.25f, 0.35f, 0.45f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabActive]   = ImVec4(0.00f, 0.80f, 1.00f, 1.00f);
    colors[ImGuiCol_CheckMark]             = ImVec4(0.00f, 0.85f, 1.00f, 1.00f);
    colors[ImGuiCol_SliderGrab]            = ImVec4(0.00f, 0.80f, 1.00f, 0.80f);
    colors[ImGuiCol_SliderGrabActive]      = ImVec4(0.00f, 0.95f, 1.00f, 1.00f);
    colors[ImGuiCol_Button]                = ImVec4(0.13f, 0.19f, 0.26f, 1.00f);
    colors[ImGuiCol_ButtonHovered]         = ImVec4(0.00f, 0.65f, 0.85f, 1.00f);
    colors[ImGuiCol_ButtonActive]          = ImVec4(0.00f, 0.85f, 1.00f, 1.00f);
    colors[ImGuiCol_Header]                = ImVec4(0.14f, 0.20f, 0.28f, 1.00f);
    colors[ImGuiCol_HeaderHovered]         = ImVec4(0.00f, 0.55f, 0.75f, 0.80f);
    colors[ImGuiCol_HeaderActive]          = ImVec4(0.00f, 0.75f, 0.95f, 1.00f);
    colors[ImGuiCol_Separator]             = ImVec4(0.18f, 0.25f, 0.34f, 0.60f);
    colors[ImGuiCol_SeparatorHovered]      = ImVec4(0.00f, 0.85f, 1.00f, 0.78f);
    colors[ImGuiCol_SeparatorActive]       = ImVec4(0.00f, 0.85f, 1.00f, 1.00f);
    colors[ImGuiCol_ResizeGrip]            = ImVec4(0.20f, 0.28f, 0.36f, 0.50f);
    colors[ImGuiCol_ResizeGripHovered]     = ImVec4(0.00f, 0.85f, 1.00f, 0.70f);
    colors[ImGuiCol_ResizeGripActive]      = ImVec4(0.00f, 0.95f, 1.00f, 1.00f);
    colors[ImGuiCol_Tab]                   = ImVec4(0.10f, 0.14f, 0.19f, 0.90f);
    colors[ImGuiCol_TabHovered]            = ImVec4(0.00f, 0.65f, 0.85f, 0.80f);
    colors[ImGuiCol_TabActive]             = ImVec4(0.00f, 0.55f, 0.75f, 1.00f);
    colors[ImGuiCol_TableHeaderBg]         = ImVec4(0.10f, 0.14f, 0.19f, 1.00f);
    colors[ImGuiCol_TableBorderStrong]     = ImVec4(0.18f, 0.25f, 0.34f, 1.00f);
    colors[ImGuiCol_TableBorderLight]      = ImVec4(0.14f, 0.19f, 0.26f, 1.00f);
    colors[ImGuiCol_TableRowBg]            = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_TableRowBgAlt]         = ImVec4(0.08f, 0.11f, 0.15f, 0.50f);
}

void UIManager::BeginFrame() {
    if (!s_isInitialized) return;

    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    FeedGamepadToImGui();
    ImGui::NewFrame();
    InputPrompts::Update();   // keyboard/mouse or controller prompts
}

void UIManager::RenderTopNavigationBar() {
    float barHeight = 44.0f;
    ImGuiViewport* vp = ImGui::GetMainViewport();

    ImGui::SetNextWindowPos(vp->Pos);
    ImGui::SetNextWindowSize(ImVec2(vp->Size.x, barHeight));

    ImGuiWindowFlags flags = 
        ImGuiWindowFlags_NoDecoration | 
        ImGuiWindowFlags_NoMove | 
        ImGuiWindowFlags_NoScrollWithMouse | 
        ImGuiWindowFlags_NoSavedSettings;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::Begin("TopNavBar", nullptr, flags);

    // Title / Logo
    ImGui::TextColored(ImVec4(0.0f, 0.88f, 1.0f, 1.0f), "PROJECT NOBLE // HALO: REACH CE");
    ImGui::SameLine();
    ImGui::TextDisabled("v%s", VERSION_STRING.data());

    ImGui::SameLine(360.0f);

    auto NavButton = [](const char* name, UIView view, float width = 130.0f) {
        bool isActive = (s_activeView == view);
        if (isActive) {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.00f, 0.55f, 0.75f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
        }

        if (ImGui::Button(name, ImVec2(width, 28))) {
            s_activeView = view;
        }

        if (isActive) {
            ImGui::PopStyleColor(2);
        }
        ImGui::SameLine();
    };

    NavButton("MISSIONS / PLAY", UIView::MissionLauncher, 140.0f);
    NavButton("SERVERS", UIView::ServerBrowser, 110.0f);
    NavButton("ARMOR LOCKER", UIView::ArmorCustomizer, 130.0f);
    NavButton("REACH MODS", UIView::WorkshopMods, 120.0f);
    NavButton("SETTINGS", UIView::Settings, 100.0f);

    ImGui::End();
    ImGui::PopStyleVar();
}

void UIManager::Render(BlamEngine* engine) {
    if (!s_isInitialized) return;

    RenderTopNavigationBar();

    // Main viewport window below nav bar
    ImGuiViewport* vp = ImGui::GetMainViewport();
    float navHeight = 44.0f;
    ImGui::SetNextWindowPos(ImVec2(vp->Pos.x, vp->Pos.y + navHeight));
    ImGui::SetNextWindowSize(ImVec2(vp->Size.x, vp->Size.y - navHeight));

    ImGuiWindowFlags flags = 
        ImGuiWindowFlags_NoDecoration | 
        ImGuiWindowFlags_NoMove | 
        ImGuiWindowFlags_NoBringToFrontOnFocus | 
        ImGuiWindowFlags_NoSavedSettings;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::Begin("MainWorkspace", nullptr, flags);

    switch (s_activeView) {
        case UIView::MissionLauncher:
            if (s_missionLauncher) s_missionLauncher->Render(engine);
            break;
        case UIView::ServerBrowser:
            if (s_serverBrowser) s_serverBrowser->Render();
            break;
        case UIView::ArmorCustomizer:
            if (s_armorCustomizer) s_armorCustomizer->Render(s_device, s_context);
            break;
        case UIView::WorkshopMods:
            if (s_workshopManager) s_workshopManager->Render();
            break;
        case UIView::Settings:
            ImGui::TextColored(ImVec4(0.0f, 0.85f, 1.0f, 1.0f), "SYSTEM, NETWORK & GRAPHICS SETTINGS");
            ImGui::Separator();
            ImGui::Spacing();
            ImGui::Text("Client Host: Project Noble (Halo: Reach Community Edition)");
            ImGui::Text("Engine Target: haloreach.dll (Blam! Engine MCC x64)");
            ImGui::Text("Resolution:  1280 x 720 (Hardware Scalable)");
            ImGui::Text("Renderer:    Direct3D 11 (Hardware Acceleration)");
            ImGui::Text("Networking:  Self-Hosted P2P with UPnP Gateway NAT Traversal");
            ImGui::Text("Anti-Cheat:  Bypassed (Zero EAC injection)");
            break;
    }

    ImGui::End();
    ImGui::PopStyleVar();
}

void UIManager::EndFrame() {
    if (!s_isInitialized) return;

    ImGui::Render();
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

LRESULT UIManager::HandleWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    if (s_isInitialized) {
        if (ImGui_ImplWin32_WndProcHandler(hwnd, uMsg, wParam, lParam)) {
            return 1;
        }
    }
    return 0;
}

} // namespace noble
