#pragma once

#include <windows.h>
#include <d3d11.h>
#include <memory>
#include "ServerBrowser.hpp"
#include "ArmorCustomizer.hpp"
#include "WorkshopManager.hpp"
#include "MissionLauncher.hpp"
#include "common/Types.hpp"

namespace noble {

class BlamEngine;

enum class UIView {
    MissionLauncher,
    ServerBrowser,
    ArmorCustomizer,
    WorkshopMods,
    Settings
};

class UIManager {
public:
    static bool Initialize(HWND hwnd, ID3D11Device* device, ID3D11DeviceContext* context);
    static void Shutdown();

    static void BeginFrame();
    static void Render(BlamEngine* engine = nullptr);
    static void EndFrame();

    static LRESULT HandleWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    static void SetActiveView(UIView view) { s_activeView = view; }
    static UIView GetActiveView() { return s_activeView; }

    static MissionLauncher& GetMissionLauncher() { return *s_missionLauncher; }
    static ServerBrowser& GetServerBrowser() { return *s_serverBrowser; }
    static ArmorCustomizer& GetArmorCustomizer() { return *s_armorCustomizer; }
    static bool HasArmorCustomizer() { return s_armorCustomizer != nullptr; }
    static WorkshopManager& GetWorkshopManager() { return *s_workshopManager; }

private:
    static void ApplyReachTheme();
    static void RenderTopNavigationBar();

    static bool s_isInitialized;
    static UIView s_activeView;
    static ID3D11Device* s_device;
    static ID3D11DeviceContext* s_context;
    static std::unique_ptr<MissionLauncher> s_missionLauncher;
    static std::unique_ptr<ServerBrowser> s_serverBrowser;
    static std::unique_ptr<ArmorCustomizer> s_armorCustomizer;
    static std::unique_ptr<WorkshopManager> s_workshopManager;
};

} // namespace noble
