#include "D3D11Renderer.hpp"
#include "common/Logger.hpp"
#include "Window.hpp"
#include <d3d11_4.h>

namespace noble {

void SyncExclusiveFullscreen(IDXGISwapChain* swapChain) {
    if (!swapChain) return;
    Window* window = Window::Active();
    if (!window) return;
    DXGI_SWAP_CHAIN_DESC desc{};
    if (FAILED(swapChain->GetDesc(&desc)) || desc.OutputWindow != window->GetHwnd()) return;

    BOOL fullscreen = FALSE;
    if (FAILED(swapChain->GetFullscreenState(&fullscreen, nullptr))) return;

    // DXGI leaves exclusive full screen by itself when the window loses focus (Alt-Tab). Note it and put
    // the window back to its normal geometry.
    if (!fullscreen && Window::IsExclusiveFullscreenActive()) {
        Window::SetExclusiveFullscreenActive(false);
        window->RequestGeometryRefresh();
        NOBLE_INFO("Display: left exclusive full screen (focus lost)");
    }

    const bool want = Window::WantsExclusiveFullscreen();
    static bool s_lastWant = false;
    static ULONGLONG s_nextAttempt = 0;
    static bool s_failureLogged = false;
    if (want != s_lastWant) {
        s_lastWant = want;
        s_nextAttempt = 0;
        s_failureLogged = false;
    }
    if (want == (fullscreen != FALSE)) return;
    const ULONGLONG now = GetTickCount64();
    if (now < s_nextAttempt) return;

    if (!want) {
        const HRESULT hr = swapChain->SetFullscreenState(FALSE, nullptr);
        Window::SetExclusiveFullscreenActive(false);
        window->RequestGeometryRefresh();
        NOBLE_INFO("Display: left exclusive full screen (0x{:08X})", static_cast<uint32_t>(hr));
        return;
    }

    const HRESULT hr = swapChain->SetFullscreenState(TRUE, nullptr);
    if (FAILED(hr)) {
        s_nextAttempt = now + 2000;
        if (!s_failureLogged) {
            s_failureLogged = true;
            NOBLE_WARN("Display: exclusive full screen not available yet (0x{:08X}); retrying", static_cast<uint32_t>(hr));
        }
        return;
    }
    Window::SetExclusiveFullscreenActive(true);
    // No ALLOW_MODE_SWITCH on the swap chain, so DXGI keeps the desktop mode (resolution and refresh rate).
    // The process is not DPI aware, so window/monitor sizes it sees can be scaled (a 4K display at 200 %
    // reads as 1920x1080); the output's own description is in real pixels.
    DXGI_OUTPUT_DESC outputDesc{};
    ComPtr<IDXGIOutput> output;
    if (SUCCEEDED(swapChain->GetContainingOutput(output.GetAddressOf())) && output && SUCCEEDED(output->GetDesc(&outputDesc))) {
        const RECT& r = outputDesc.DesktopCoordinates;
        NOBLE_INFO("Display: exclusive full screen on at the desktop mode ({}x{}, buffers {}x{})", r.right - r.left,
                   r.bottom - r.top, desc.BufferDesc.Width, desc.BufferDesc.Height);
    } else {
        NOBLE_INFO("Display: exclusive full screen on at the desktop mode");
    }
}

D3D11Renderer::D3D11Renderer() = default;

D3D11Renderer::~D3D11Renderer() {
    Shutdown();
}

static HRESULT SafeCreateD3D(
    D3D_DRIVER_TYPE driverType,
    UINT Flags,
    const D3D_FEATURE_LEVEL* pFeatureLevels,
    UINT FeatureLevels,
    const DXGI_SWAP_CHAIN_DESC* pSwapChainDesc,
    IDXGISwapChain** ppSwapChain,
    ID3D11Device** ppDevice,
    D3D_FEATURE_LEVEL* pFeatureLevel,
    ID3D11DeviceContext** ppImmediateContext,
    DWORD* pExceptionCode
) {
    __try {
        return D3D11CreateDeviceAndSwapChain(
            nullptr,
            driverType,
            nullptr,
            Flags,
            pFeatureLevels,
            FeatureLevels,
            D3D11_SDK_VERSION,
            pSwapChainDesc,
            ppSwapChain,
            ppDevice,
            pFeatureLevel,
            ppImmediateContext
        );
    }
    __except (*pExceptionCode = GetExceptionCode(), EXCEPTION_EXECUTE_HANDLER) {
        return E_FAIL;
    }
}

bool D3D11Renderer::Initialize(HWND hwnd, uint32_t width, uint32_t height) {
    m_hwnd = hwnd;
    m_width = width;
    m_height = height;

    DXGI_SWAP_CHAIN_DESC scd = { 0 };
    scd.BufferCount = 2;
    scd.BufferDesc.Width = width;
    scd.BufferDesc.Height = height;
    scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    scd.BufferDesc.RefreshRate.Numerator = 60;
    scd.BufferDesc.RefreshRate.Denominator = 1;
    // No DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH: DISPLAY MODE = FULLSCREEN must keep the desktop's resolution
    // and refresh rate. With the flag, DXGI switched a 4K display to the back buffer's size (2560x1600).
    scd.Flags = 0;
    // haloreach.dll builds a shader resource view on the backbuffer (rasterizer surface pool 1).
    // Its own swap-chain path uses BufferUsage 0x30 (SHADER_INPUT | RENDER_TARGET_OUTPUT); without
    // SHADER_INPUT, CreateShaderResourceView fails, pool 1 is never initialised, and the engine
    // crashes at haloreach.dll+0x2744F2 during game globals init. 
    scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT | DXGI_USAGE_SHADER_INPUT;
    scd.OutputWindow = hwnd;
    scd.SampleDesc.Count = 1;
    scd.SampleDesc.Quality = 0;
    scd.Windowed = TRUE;
    scd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    UINT createDeviceFlags = 0;
#ifdef _DEBUG
    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    D3D_FEATURE_LEVEL featureLevels[] = {
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0
    };
    D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;

    NOBLE_INFO("Creating D3D11 device and swap chain for HWND 0x{:X}...", reinterpret_cast<uintptr_t>(hwnd));
    spdlog::default_logger()->flush();

    DWORD exceptionCode = 0;
    HRESULT hr = SafeCreateD3D(
        D3D_DRIVER_TYPE_HARDWARE,
        createDeviceFlags,
        featureLevels,
        static_cast<UINT>(std::size(featureLevels)),
        &scd,
        m_swapChain.GetAddressOf(),
        m_device.GetAddressOf(),
        &featureLevel,
        m_context.GetAddressOf(),
        &exceptionCode
    );

    if (FAILED(hr) && exceptionCode == 0) {
        NOBLE_WARN("Hardware D3D11 creation returned 0x{:08X}. Trying WARP driver fallback...", static_cast<uint32_t>(hr));
        spdlog::default_logger()->flush();
        hr = SafeCreateD3D(
            D3D_DRIVER_TYPE_WARP,
            createDeviceFlags,
            featureLevels,
            static_cast<UINT>(std::size(featureLevels)),
            &scd,
            m_swapChain.GetAddressOf(),
            m_device.GetAddressOf(),
            &featureLevel,
            m_context.GetAddressOf(),
            &exceptionCode
        );
    }

    if (exceptionCode != 0) {
        NOBLE_ERROR("SEH Exception 0x{:08X} caught inside D3D11CreateDeviceAndSwapChain!", exceptionCode);
        spdlog::default_logger()->flush();
        return false;
    }

    NOBLE_INFO("D3D11CreateDeviceAndSwapChain returned HRESULT: 0x{:08X}", static_cast<uint32_t>(hr));
    spdlog::default_logger()->flush();

    if (FAILED(hr)) {
        NOBLE_ERROR("D3D11CreateDeviceAndSwapChain failed with HRESULT: 0x{:08X}", static_cast<uint32_t>(hr));
        spdlog::default_logger()->flush();
        return false;
    }

    NOBLE_INFO("Direct3D 11 device and swap chain created. Feature Level: 0x{:X}", static_cast<uint32_t>(featureLevel));
    spdlog::default_logger()->flush();

    // ID3D11Multithread protection wraps every context call in a critical section, which causes
    // massive CPU overhead and draw call stalls under AMD Radeon graphics drivers (e.g. RX 5700 XT).
    // haloreach.dll and Noble's UI/loading overlays execute strictly sequentially on the dedicated render thread.
    ComPtr<ID3D11Multithread> multithread;
    if (SUCCEEDED(m_context->QueryInterface(IID_PPV_ARGS(multithread.GetAddressOf()))) && multithread) {
        multithread->SetMultithreadProtected(FALSE);
        NOBLE_INFO("Direct3D 11 multithread protection disabled for peak draw call throughput.");
    }

    // Query adapter to detect GPU vendor (AMD Radeon: 0x1002, NVIDIA: 0x10DE, Intel: 0x8086)
    bool isAmdGpu = false;
    ComPtr<IDXGIDevice> dxgiDevice;
    if (SUCCEEDED(m_device.As(&dxgiDevice))) {
        ComPtr<IDXGIAdapter> adapter;
        if (SUCCEEDED(dxgiDevice->GetAdapter(&adapter))) {
            DXGI_ADAPTER_DESC desc{};
            if (SUCCEEDED(adapter->GetDesc(&desc))) {
                if (desc.VendorId == 0x1002) {
                    isAmdGpu = true;
                    char descStr[128] = { 0 };
                    WideCharToMultiByte(CP_UTF8, 0, desc.Description, -1, descStr, sizeof(descStr), nullptr, nullptr);
                    NOBLE_INFO("Direct3D 11: Detected AMD Radeon GPU '{}' (VendorId: 0x{:04X})", descStr, desc.VendorId);
                }
            }
        }
    }

    // Set maximum frame latency on the DXGI device.
    // On NVIDIA and Intel GPUs, 1 frame latency gives minimal input lag without pipeline stalling.
    // On AMD Radeon GPUs (e.g. RX 5700 XT, RDNA/GCN), setting latency to 1 with DXGI_SWAP_EFFECT_DISCARD
    // forces the AMD driver to hard-synchronize and wait for GPU idle on every Present, resulting in
    // severe micro-stutter and halving the frame rate. Latency 2 allows the AMD Command Processor to
    // overlap commands smoothly without pipeline stalls.
    ComPtr<IDXGIDevice1> dxgiDevice1;
    if (SUCCEEDED(m_device.As(&dxgiDevice1)) && dxgiDevice1) {
        const UINT latency = isAmdGpu ? 2u : 1u;
        dxgiDevice1->SetMaximumFrameLatency(latency);
        NOBLE_INFO("Direct3D 11 DXGI maximum frame latency set to {} ({}).",
                   latency, isAmdGpu ? "AMD Radeon optimized latency" : "low-latency input mode");
    }

    // Alt+Enter would toggle exclusive full screen behind the DISPLAY MODE setting's back; the setting owns it.
    ComPtr<IDXGIFactory> factory;
    if (SUCCEEDED(m_swapChain->GetParent(IID_PPV_ARGS(factory.GetAddressOf()))) && factory) {
        factory->MakeWindowAssociation(hwnd, DXGI_MWA_NO_ALT_ENTER);
    }

    if (!CreateRenderTarget()) {
        return false;
    }

    return true;
}

bool D3D11Renderer::CreateRenderTarget() {
    ComPtr<ID3D11Texture2D> backBuffer;
    HRESULT hr = m_swapChain->GetBuffer(0, IID_PPV_ARGS(backBuffer.GetAddressOf()));
    if (FAILED(hr)) {
        NOBLE_ERROR("Failed to get swap chain back buffer.");
        return false;
    }

    hr = m_device->CreateRenderTargetView(backBuffer.Get(), nullptr, m_renderTargetView.GetAddressOf());
    if (FAILED(hr)) {
        NOBLE_ERROR("Failed to create D3D11 render target view.");
        return false;
    }

    return true;
}

void D3D11Renderer::CleanupRenderTarget() {
    m_renderTargetView.Reset();
}

void D3D11Renderer::Resize(uint32_t width, uint32_t height) {
    if (width == 0 || height == 0 || !m_swapChain) return;

    m_width = width;
    m_height = height;

    CleanupRenderTarget();
    m_swapChain->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, 0);
    CreateRenderTarget();
}

void D3D11Renderer::BeginFrame() {
    // Halo Reach dark slate blue clear color
    const float clearColor[4] = { 0.05f, 0.07f, 0.11f, 1.0f };
    ID3D11RenderTargetView* rtv = m_renderTargetView.Get();
    m_context->OMSetRenderTargets(1, &rtv, nullptr);
    m_context->ClearRenderTargetView(rtv, clearColor);

    D3D11_VIEWPORT vp = { 0 };
    vp.Width = static_cast<float>(m_width);
    vp.Height = static_cast<float>(m_height);
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    m_context->RSSetViewports(1, &vp);
}

void D3D11Renderer::EndFrame(bool vsync) {
    if (m_swapChain) {
        m_swapChain->Present(vsync ? 1 : 0, 0);
    }
}

void D3D11Renderer::Shutdown() {
    // DXGI requires leaving exclusive full screen before the swap chain is released.
    if (m_swapChain) {
        BOOL fullscreen = FALSE;
        if (SUCCEEDED(m_swapChain->GetFullscreenState(&fullscreen, nullptr)) && fullscreen) {
            m_swapChain->SetFullscreenState(FALSE, nullptr);
        }
        Window::SetExclusiveFullscreenActive(false);
    }
    CleanupRenderTarget();
    m_context.Reset();
    m_swapChain.Reset();
    m_device.Reset();
}

} // namespace noble
