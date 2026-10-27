#include "PauseMenu.hpp"
#include "NobleStyle.hpp"
#include "UiArt.hpp"
#include "career/Career.hpp"
#include "common/Logger.hpp"
#include "common/Relaunch.hpp"

#include <d3d11.h>
#include <dxgi.h>
#include <imgui.h>
#include <imgui_impl_dx11.h>
#include <atomic>
#include <cmath>
#include <cstdio>
#include <mutex>
#include <string>

namespace noble {

namespace {

std::atomic<bool> s_open{false};
std::atomic<bool> s_gameOver{false};
std::mutex s_mapMutex;
std::string s_loadingMap;
std::string s_loadingMapName;

struct MapName { const char* scenario; const char* name; };

// Display names for the loading screen (same names as the front-end menu).
const MapName kMapNames[] = {
    { "50_panopticon", "Boardwalk" },   { "70_boneyard", "Boneyard" },        { "45_launch_station", "Countdown" },
    { "30_settlement", "Powerhouse" },  { "52_ivory_tower", "Reflection" },   { "35_island", "Spire" },
    { "20_sword_slayer", "Sword Base" }, { "45_aftship", "Zealot" },          { "dlc_slayer", "Anchor 9" },
    { "dlc_invasion", "Breakpoint" },   { "dlc_medium", "Tempest" },          { "condemned", "Condemned" },
    { "trainingpreserve", "Highlands" }, { "cex_beaver_creek", "Battle Canyon" }, { "cex_damnation", "Penance" },
    { "cex_timberland", "Ridgeline" },  { "cex_prisoner", "Solitary" },       { "cex_hangemhigh", "High Noon" },
    { "cex_headlong", "Breakneck" },    { "forge_halo", "Forge World" },
    { "m05", "Noble Actual" },          { "m10", "Winter Contingency" },     { "m20", "ONI: Sword Base" },
    { "m30", "Nightfall" },             { "m35", "Tip of the Spear" },        { "m45", "Long Night of Solace" },
    { "m50", "Exodus" },                { "m52", "New Alexandria" },          { "m60", "The Package" },
    { "m70", "The Pillar of Autumn" },  { "m70_bonus", "Lone Wolf" },
    { "ff10_prototype", "Overlook" },  { "ff20_courtyard", "Courtyard" },    { "ff30_waterfront", "Waterfront" },
    { "ff45_corvette", "Corvette" },    { "ff50_park", "Beachhead" },          { "ff60_airview", "Outpost" },
    { "ff60_icecave", "Glacier" },      { "ff70_holdout", "Holdout" },        { "ff_unearthed", "Unearthed" },
    { "cex_ff_halo", "Installation 04" },
};

std::string DisplayName(const std::string& scenario) {
    for (const MapName& m : kMapNames) {
        if (scenario == m.scenario) return m.name;
    }
    return scenario;
}

std::string Upper(std::string s) {
    for (char& c : s) c = static_cast<char>(::toupper(static_cast<unsigned char>(c)));
    return s;
}

// Starts a new Noble (empty args = front-end menu) and closes this one.
void RelaunchAndClose(HWND hwnd, const std::wstring& args, const char* what) {
    NOBLE_INFO("PauseMenu: {} requested", what);
    Relaunch::Start(args);
    PostMessageW(hwnd, WM_CLOSE, 0, 0);
}

ImVec2 CenteredWindow(float width) {
    const ImGuiIO& io = ImGui::GetIO();
    ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(width, 0.0f), ImGuiCond_Always);
    return ImVec2(-1.0f, 38.0f);
}

constexpr ImGuiWindowFlags kPanelFlags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
                                         ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize;

} // namespace

bool PauseMenu::IsOpen() { return s_open.load(); }

void PauseMenu::Toggle() {
    if (s_gameOver.load()) return;  // the post-game panel replaces the pause menu
    const bool nowOpen = !s_open.load();
    s_open.store(nowOpen);
    NOBLE_INFO("PauseMenu: {}", nowOpen ? "opened" : "closed");
}

void PauseMenu::Close() {
    if (s_open.exchange(false)) {
        NOBLE_INFO("PauseMenu: closed");
    }
}

void PauseMenu::ShowGameOver() {
    if (!s_gameOver.exchange(true)) {
        s_open.store(false);
        NOBLE_INFO("PauseMenu: game over, showing post-game panel");
    }
}

bool PauseMenu::IsGameOver() { return s_gameOver.load(); }

bool PauseMenu::WantsPointer() { return s_open.load() || s_gameOver.load(); }

void PauseMenu::SetLoadingMap(const std::string& scenario) {
    std::lock_guard<std::mutex> lock(s_mapMutex);
    s_loadingMap = scenario;
    s_loadingMapName = Upper(DisplayName(scenario));
}

void PauseMenu::DrawMenu(HWND hwnd) {
    ImGuiIO& io = ImGui::GetIO();

    // Dim the game behind the menu.
    ImGui::GetBackgroundDrawList()->AddRectFilled(ImVec2(0, 0), io.DisplaySize, IM_COL32(0, 0, 0, 140));

    const ImVec2 button = CenteredWindow(340.0f);
    if (ImGui::Begin("##NoblePauseMenu", nullptr, kPanelFlags)) {
        NobleStyle::Header("PAUSED");
        ImGui::Spacing();

        if (NobleStyle::PrimaryButton("RESUME", button)) {
            Close();
        }
        // Add new entries here (settings, ...).
        ImGui::Spacing();
        if (ImGui::Button("QUIT TO MENU", button)) {
            // The engine can't switch maps once started (RR also runs menu and match as separate
            // processes), so start a fresh Noble with no -map, which opens the front-end menu.
            Close();
            RelaunchAndClose(hwnd, L"", "quit to menu");
        }
        ImGui::Spacing();
        if (NobleStyle::DangerButton("QUIT", button)) {
            NOBLE_INFO("PauseMenu: quit requested");
            Close();
            // The main thread owns shutdown: WM_CLOSE ends its message loop and runs the normal exit path.
            PostMessageW(hwnd, WM_CLOSE, 0, 0);
        }
        ImGui::Spacing();
        ImGui::TextDisabled("Esc to resume");
    }
    ImGui::End();
}

// Credits paid for the match that just ended (career/Career.cpp), with rank progress.
static void DrawCredits() {
    const Career::Award a = Career::LastAward();
    if (!a.valid) return;
    const auto& ranks = Career::Ranks();
    for (const Career::AwardLine& l : a.lines) {
        ImGui::TextColored(NobleStyle::TextMuted(), "%s", l.label.c_str());
        ImGui::SameLine(260.0f);
        ImGui::Text("+%lld cR", static_cast<long long>(l.credits));
    }
    ImGui::Separator();
    ImGui::TextColored(NobleStyle::Accent(), "CREDITS EARNED");
    ImGui::SameLine(260.0f);
    ImGui::TextColored(NobleStyle::Accent(), "+%lld cR", static_cast<long long>(a.total));
    ImGui::Spacing();
    if (a.rankAfter > a.rankBefore) {
        ImGui::TextColored(NobleStyle::Accent(), "PROMOTED: %s", ranks[a.rankAfter].name);
    } else {
        ImGui::Text("%s", ranks[a.rankAfter].name);
    }
    if (a.rankAfter + 1 < static_cast<int>(ranks.size())) {
        const int64_t lo = ranks[a.rankAfter].credits;
        const int64_t hi = ranks[a.rankAfter + 1].credits;
        const float f = static_cast<float>(a.creditsAfter - lo) / static_cast<float>(hi - lo);
        char label[64];
        std::snprintf(label, sizeof(label), "%lld / %lld cR", static_cast<long long>(a.creditsAfter), static_cast<long long>(hi));
        ImGui::ProgressBar(f, ImVec2(-1.0f, 0.0f), label);
    }
}

void PauseMenu::DrawGameOver(HWND hwnd) {
    ImGuiIO& io = ImGui::GetIO();
    ImGui::GetBackgroundDrawList()->AddRectFilled(ImVec2(0, 0), io.DisplaySize, IM_COL32(4, 8, 14, 200));

    std::string mapName;
    {
        std::lock_guard<std::mutex> lock(s_mapMutex);
        mapName = s_loadingMapName;
    }

    const ImVec2 button = CenteredWindow(380.0f);
    if (ImGui::Begin("##NobleGameOver", nullptr, kPanelFlags)) {
        NobleStyle::Header("GAME OVER");
        if (!mapName.empty()) {
            ImGui::TextColored(NobleStyle::TextMuted(), "%s", mapName.c_str());
        }
        ImGui::Spacing();
        DrawCredits();
        ImGui::Spacing();

        const std::wstring again = Relaunch::CurrentGameArgs();
        if (!again.empty()) {
            if (NobleStyle::PrimaryButton("PLAY AGAIN", button)) {
                RelaunchAndClose(hwnd, again, "play again");
            }
            ImGui::Spacing();
        }
        if (ImGui::Button("RETURN TO MENU", button)) {
            RelaunchAndClose(hwnd, L"", "return to menu");
        }
        ImGui::Spacing();
        if (NobleStyle::DangerButton("QUIT", button)) {
            NOBLE_INFO("PauseMenu: quit requested from post-game panel");
            PostMessageW(hwnd, WM_CLOSE, 0, 0);
        }
    }
    ImGui::End();
}

void PauseMenu::DrawLoading() {
    ImGuiIO& io = ImGui::GetIO();
    ImDrawList* dl = ImGui::GetBackgroundDrawList();
    const float w = io.DisplaySize.x;
    const float h = io.DisplaySize.y;

    // Reach-style loading screen: a full-screen scene (the player's own art, Noble's art or the
    // mission's scene from the MCC install), map name bottom-left, a moving bar (the engine's
    // progress callback doesn't report real progress, so the bar only shows that it's working).
    std::string mapName, scenario;
    {
        std::lock_guard<std::mutex> lock(s_mapMutex);
        mapName = s_loadingMapName;
        scenario = s_loadingMap;
    }
    dl->AddRectFilledMultiColor(ImVec2(0, 0), ImVec2(w, h),
                                IM_COL32(10, 16, 24, 255), IM_COL32(10, 16, 24, 255),
                                IM_COL32(2, 4, 8, 255), IM_COL32(2, 4, 8, 255));
    const UiTexture scene = UiArt::LoadingBackground(scenario);
    if (scene) {
        UiArt::DrawCover(dl, scene, ImVec2(0, 0), ImVec2(w, h));
        dl->AddRectFilledMultiColor(ImVec2(0, h * 0.55f), ImVec2(w, h), IM_COL32(0, 0, 0, 0), IM_COL32(0, 0, 0, 0),
                                    IM_COL32(0, 0, 0, 215), IM_COL32(0, 0, 0, 215));
    }
    // Multiplayer and Firefight have no per-map scene: show the map's preview as well.
    const bool campaign = scenario.size() >= 3 && scenario[0] == 'm' && scenario[1] >= '0' && scenario[1] <= '9';
    if (!campaign) {
        const UiTexture preview = UiArt::MapPreview(scenario, true);
        if (preview) {
            const float pw = w * 0.30f, ph = pw * 292.0f / 668.0f;
            const ImVec2 p0(w * 0.93f - pw, h * 0.80f - ph), p1(w * 0.93f, h * 0.80f);
            dl->AddRectFilled(ImVec2(p0.x - 4, p0.y - 4), ImVec2(p1.x + 4, p1.y + 4), IM_COL32(0, 0, 0, 160));
            UiArt::DrawCover(dl, preview, p0, p1);
            dl->AddRect(p0, p1, IM_COL32(170, 200, 220, 140));
        }
    }

    const ImVec4 accent = NobleStyle::Accent();
    const ImU32 accentCol = ImGui::ColorConvertFloat4ToU32(accent);
    const ImU32 mutedCol = ImGui::ColorConvertFloat4ToU32(NobleStyle::TextMuted());

    const float left = w * 0.07f;
    const float base = h * 0.80f;

    ImFont* title = NobleStyle::TitleFont();
    ImFont* font = ImGui::GetFont();
    const float titleSize = title ? title->FontSize * 1.4f : ImGui::GetFontSize() * 2.0f;
    dl->AddText(font, ImGui::GetFontSize(), ImVec2(left, base - titleSize - 26.0f), mutedCol, "LOADING");
    if (!mapName.empty()) {
        dl->AddText(title ? title : font, titleSize, ImVec2(left, base - titleSize - 4.0f), IM_COL32(235, 240, 245, 255), mapName.c_str());
    }

    const float barW = w * 0.30f;
    const float barH = 4.0f;
    const ImVec2 b0(left, base + 8.0f);
    const ImVec2 b1(left + barW, base + 8.0f + barH);
    dl->AddRectFilled(b0, b1, IM_COL32(255, 255, 255, 30));
    const float t = static_cast<float>(ImGui::GetTime());
    const float seg = barW * 0.25f;
    const float pos = std::fmod(t * 0.6f, 1.0f) * (barW + seg) - seg;
    const float x0 = b0.x + (pos < 0.0f ? 0.0f : pos);
    const float x1 = b0.x + ((pos + seg) > barW ? barW : (pos + seg));
    if (x1 > x0) dl->AddRectFilled(ImVec2(x0, b0.y), ImVec2(x1, b1.y), accentCol);
}

void PauseMenu::Render(IDXGISwapChain* swapChain, bool loading) {
    const bool gameOver = s_gameOver.load();
    const bool open = s_open.load();
    if (!(open || gameOver || loading) || !swapChain || !ImGui::GetCurrentContext()) return;

    DXGI_SWAP_CHAIN_DESC desc{};
    if (FAILED(swapChain->GetDesc(&desc)) || !desc.OutputWindow) return;
    const HWND hwnd = desc.OutputWindow;

    ID3D11Device* device = nullptr;
    if (FAILED(swapChain->GetDevice(__uuidof(ID3D11Device), reinterpret_cast<void**>(&device))) || !device) return;
    ID3D11DeviceContext* context = nullptr;
    device->GetImmediateContext(&context);

    ID3D11Texture2D* backBuffer = nullptr;
    ID3D11RenderTargetView* rtv = nullptr;
    if (SUCCEEDED(swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&backBuffer))) && backBuffer) {
        device->CreateRenderTargetView(backBuffer, nullptr, &rtv);
    }

    if (context && rtv) {
        D3D11_TEXTURE2D_DESC bbDesc{};
        backBuffer->GetDesc(&bbDesc);

        // Feed ImGui ourselves on this thread (the Win32 backend's message handler runs on the
        // window thread and its event queue is not thread safe).
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange; // the window proc shows the arrow
        io.DisplaySize = ImVec2(static_cast<float>(bbDesc.Width), static_cast<float>(bbDesc.Height));

        static LARGE_INTEGER s_freq{}, s_last{};
        LARGE_INTEGER now{};
        QueryPerformanceCounter(&now);
        if (s_freq.QuadPart == 0) QueryPerformanceFrequency(&s_freq);
        float dt = s_last.QuadPart ? static_cast<float>(now.QuadPart - s_last.QuadPart) / static_cast<float>(s_freq.QuadPart) : 1.0f / 60.0f;
        if (dt <= 0.0f || dt > 0.25f) dt = 1.0f / 60.0f;
        s_last = now;
        io.DeltaTime = dt;

        RECT client{};
        GetClientRect(hwnd, &client);
        POINT cursor{};
        if ((open || gameOver) && GetForegroundWindow() == hwnd && GetCursorPos(&cursor) &&
            ScreenToClient(hwnd, &cursor) && client.right > 0 && client.bottom > 0) {
            // Window may be resized while the back buffer stays the same size: scale to back-buffer pixels.
            const float sx = io.DisplaySize.x / static_cast<float>(client.right);
            const float sy = io.DisplaySize.y / static_cast<float>(client.bottom);
            io.AddMousePosEvent(static_cast<float>(cursor.x) * sx, static_cast<float>(cursor.y) * sy);
            io.AddMouseButtonEvent(0, (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0);
        } else {
            io.AddMousePosEvent(-FLT_MAX, -FLT_MAX);
            io.AddMouseButtonEvent(0, false);
        }

        ImGui_ImplDX11_NewFrame();
        ImGui::NewFrame();
        if (gameOver) {
            DrawGameOver(hwnd);
        } else {
            if (loading) DrawLoading();
            if (open) DrawMenu(hwnd);
        }
        ImGui::Render();

        // The DX11 backend saves/restores most pipeline state but not the output-merger targets.
        ID3D11RenderTargetView* oldRtv = nullptr;
        ID3D11DepthStencilView* oldDsv = nullptr;
        context->OMGetRenderTargets(1, &oldRtv, &oldDsv);
        context->OMSetRenderTargets(1, &rtv, nullptr);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
        context->OMSetRenderTargets(1, &oldRtv, oldDsv);
        if (oldRtv) oldRtv->Release();
        if (oldDsv) oldDsv->Release();
    }

    if (rtv) rtv->Release();
    if (backBuffer) backBuffer->Release();
    if (context) context->Release();
    device->Release();
}

} // namespace noble
