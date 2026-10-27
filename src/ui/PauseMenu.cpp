#include "PauseMenu.hpp"
#include "common/Logger.hpp"

#include <d3d11.h>
#include <dxgi.h>
#include <imgui.h>
#include <imgui_impl_dx11.h>
#include <atomic>
#include <string>

namespace noble {

static std::atomic<bool> s_open{false};

bool PauseMenu::IsOpen() { return s_open.load(); }

void PauseMenu::Toggle() {
    const bool nowOpen = !s_open.load();
    s_open.store(nowOpen);
    NOBLE_INFO("PauseMenu: {}", nowOpen ? "opened" : "closed");
}

void PauseMenu::Close() {
    if (s_open.exchange(false)) {
        NOBLE_INFO("PauseMenu: closed");
    }
}

void PauseMenu::DrawMenu(HWND hwnd) {
    ImGuiIO& io = ImGui::GetIO();

    // Dim the game behind the menu.
    ImGui::GetBackgroundDrawList()->AddRectFilled(ImVec2(0, 0), io.DisplaySize, IM_COL32(0, 0, 0, 140));

    const ImVec2 size(320.0f, 0.0f);
    ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(size, ImGuiCond_Always);
    const ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
                                   ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize;
    if (ImGui::Begin("##NoblePauseMenu", nullptr, flags)) {
        ImGui::TextColored(ImVec4(0.0f, 0.88f, 1.0f, 1.0f), "PAUSED");
        ImGui::Separator();
        ImGui::Spacing();

        const ImVec2 button(-1.0f, 36.0f);
        if (ImGui::Button("RESUME", button)) {
            Close();
        }
        // Add new entries here (settings, ...).
        ImGui::Spacing();
        if (ImGui::Button("QUIT TO MENU", button)) {
            // The engine can't switch maps once started (RR also runs menu and match as separate
            // processes), so start a fresh Noble with no -map, which opens the front-end menu, and
            // close this one.
            NOBLE_INFO("PauseMenu: quit to menu requested");
            wchar_t exe[MAX_PATH] = {};
            if (GetModuleFileNameW(nullptr, exe, MAX_PATH)) {
                std::wstring cmd = L"\"" + std::wstring(exe) + L"\"";
                std::wstring dir = exe;
                const size_t slash = dir.find_last_of(L"\\/");
                if (slash != std::wstring::npos) dir.resize(slash);
                STARTUPINFOW si{};
                si.cb = sizeof(si);
                PROCESS_INFORMATION pi{};
                if (CreateProcessW(exe, cmd.data(), nullptr, nullptr, FALSE, 0, nullptr, dir.c_str(), &si, &pi)) {
                    CloseHandle(pi.hThread);
                    CloseHandle(pi.hProcess);
                } else {
                    NOBLE_ERROR("PauseMenu: could not restart Noble (error {})", GetLastError());
                }
            }
            Close();
            PostMessageW(hwnd, WM_CLOSE, 0, 0);
        }
        ImGui::Spacing();
        if (ImGui::Button("QUIT", button)) {
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

void PauseMenu::Render(IDXGISwapChain* swapChain) {
    if (!s_open.load() || !swapChain || !ImGui::GetCurrentContext()) return;

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
        if (GetForegroundWindow() == hwnd && GetCursorPos(&cursor) && ScreenToClient(hwnd, &cursor) &&
            client.right > 0 && client.bottom > 0) {
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
        DrawMenu(hwnd);
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
