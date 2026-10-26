#include "D3D11Renderer.hpp"
#include "common/Logger.hpp"

namespace noble {

D3D11Renderer::D3D11Renderer() = default;

D3D11Renderer::~D3D11Renderer() {
    Shutdown();
}

static HRESULT SafeCreateD3D(
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
            D3D_DRIVER_TYPE_HARDWARE,
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
    scd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
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
    CleanupRenderTarget();
    m_context.Reset();
    m_swapChain.Reset();
    m_device.Reset();
}

} // namespace noble
