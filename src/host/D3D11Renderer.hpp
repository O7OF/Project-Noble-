#pragma once

#include <d3d11.h>
#include <dxgi.h>
#include <wrl/client.h>
#include <cstdint>

namespace noble {

using Microsoft::WRL::ComPtr;

// DISPLAY MODE = FULLSCREEN: moves the swap chain in or out of DXGI exclusive full screen so it matches
// Window::WantsExclusiveFullscreen() (setting on and Noble focused). Call on the render thread right before
// Present (ReachHost's Present hook does). Entering keeps the desktop mode (resolution and refresh rate). Buffers are not resized: they are already at native resolution and the engine
// holds references to them. Failures (e.g. DXGI_ERROR_NOT_CURRENTLY_AVAILABLE) are retried every 2 s.
void SyncExclusiveFullscreen(IDXGISwapChain* swapChain);

class D3D11Renderer {
public:
    D3D11Renderer();
    ~D3D11Renderer();

    bool Initialize(HWND hwnd, uint32_t width, uint32_t height);
    void Shutdown();

    void BeginFrame();
    void EndFrame(bool vsync = true);
    void Resize(uint32_t width, uint32_t height);

    ID3D11Device* GetDevice() const { return m_device.Get(); }
    ID3D11DeviceContext* GetContext() const { return m_context.Get(); }
    IDXGISwapChain* GetSwapChain() const { return m_swapChain.Get(); }
    ID3D11RenderTargetView* GetRenderTargetView() const { return m_renderTargetView.Get(); }

private:
    bool CreateRenderTarget();
    void CleanupRenderTarget();

    ComPtr<ID3D11Device> m_device;
    ComPtr<ID3D11DeviceContext> m_context;
    ComPtr<IDXGISwapChain> m_swapChain;
    ComPtr<ID3D11RenderTargetView> m_renderTargetView;

    HWND m_hwnd = nullptr;
    uint32_t m_width = 0;
    uint32_t m_height = 0;
};

} // namespace noble
