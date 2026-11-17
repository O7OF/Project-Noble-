#include "ParticleTint.hpp"

#include "common/Logger.hpp"
#include "hooks/HookManager.hpp"

#include <windows.h>
#include <shellapi.h>
#include <d3d11.h>
#include <dxgi.h>
#include <wrl/client.h>
#include <array>
#include <atomic>
#include <cstdio>
#include <cstring>
#include <mutex>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

namespace noble::ParticleTint {

namespace {

using PSSetShaderResourcesFn = void(STDMETHODCALLTYPE*)(ID3D11DeviceContext*, UINT, UINT,
                                                        ID3D11ShaderResourceView* const*);
using CreateDeferredContextFn = HRESULT(STDMETHODCALLTYPE*)(ID3D11Device*, UINT, ID3D11DeviceContext**);
using CreateDeviceFn = HRESULT(WINAPI*)(IDXGIAdapter*, D3D_DRIVER_TYPE, HMODULE, UINT, const D3D_FEATURE_LEVEL*, UINT,
                                        UINT, ID3D11Device**, D3D_FEATURE_LEVEL*, ID3D11DeviceContext**);
using CreateDeviceAndSwapChainFn = HRESULT(WINAPI*)(IDXGIAdapter*, D3D_DRIVER_TYPE, HMODULE, UINT,
                                                    const D3D_FEATURE_LEVEL*, UINT, UINT, const DXGI_SWAP_CHAIN_DESC*,
                                                    IDXGISwapChain**, ID3D11Device**, D3D_FEATURE_LEVEL*,
                                                    ID3D11DeviceContext**);

std::atomic<bool> s_active{false};
std::atomic<bool> s_hookInstalled{false};
float s_colour[3] = {1.0f, 0.0f, 1.0f};

PSSetShaderResourcesFn s_originalImmediate = nullptr;
PSSetShaderResourcesFn s_originalDeferred = nullptr;
void* s_targetImmediate = nullptr;
void* s_targetDeferred = nullptr;

CreateDeviceFn s_originalCreateDevice = nullptr;
CreateDeviceAndSwapChainFn s_originalCreateDeviceSwapChain = nullptr;
CreateDeferredContextFn s_originalDeferredContext = nullptr;
bool s_exportsHooked = false;

std::mutex s_deviceMutex;
std::unordered_map<ID3D11Device*, ID3D11ShaderResourceView*> s_tintSrvs;
std::set<void*> s_hookedContextTargets;

constexpr UINT kPaletteWidth = 256;
constexpr UINT kPaletteHeight = 16;

void ParseTintColour() {
    int argc = 0;
    LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);
    if (!argv) return;
    std::string spec;
    for (int i = 1; i + 1 < argc; ++i) {
        if (_wcsicmp(argv[i], L"-armor-fx-runtime-tint") == 0) {
            const int len = WideCharToMultiByte(CP_UTF8, 0, argv[i + 1], -1, nullptr, 0, nullptr, nullptr);
            if (len > 1) {
                spec.resize(static_cast<size_t>(len - 1));
                WideCharToMultiByte(CP_UTF8, 0, argv[i + 1], -1, spec.data(), len, nullptr, nullptr);
            }
            break;
        }
    }
    LocalFree(argv);
    if (spec.empty()) return;

    unsigned rr = 0, gg = 0, bb = 0;
    if (std::sscanf(spec.c_str(), "%u,%u,%u", &rr, &gg, &bb) == 3) {
        s_colour[0] = static_cast<float>(rr & 0xFF) / 255.0f;
        s_colour[1] = static_cast<float>(gg & 0xFF) / 255.0f;
        s_colour[2] = static_cast<float>(bb & 0xFF) / 255.0f;
    } else if (spec.size() == 6 && std::sscanf(spec.c_str(), "%2x%2x%2x", &rr, &gg, &bb) == 3) {
        s_colour[0] = static_cast<float>(rr) / 255.0f;
        s_colour[1] = static_cast<float>(gg) / 255.0f;
        s_colour[2] = static_cast<float>(bb) / 255.0f;
    } else {
        return;
    }
    s_active = true;
    NOBLE_INFO("ParticleTint: armed for RGB ({:.2f},{:.2f},{:.2f})", s_colour[0], s_colour[1], s_colour[2]);
}

ID3D11ShaderResourceView* GetTintSrv(ID3D11Device* device) {
    if (!device) return nullptr;
    {
        std::lock_guard<std::mutex> lock(s_deviceMutex);
        auto it = s_tintSrvs.find(device);
        if (it != s_tintSrvs.end()) return it->second;
    }
    D3D11_TEXTURE2D_DESC desc{};
    desc.Width = kPaletteWidth;
    desc.Height = kPaletteHeight;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
    desc.SampleDesc.Count = 1;
    desc.Usage = D3D11_USAGE_IMMUTABLE;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

    std::array<unsigned char, kPaletteWidth * kPaletteHeight * 4> pixels{};
    for (UINT y = 0; y < kPaletteHeight; ++y) {
        for (UINT x = 0; x < kPaletteWidth; ++x) {
            const float t = static_cast<float>(x) / static_cast<float>(kPaletteWidth - 1);
            const float core = t < 0.92f ? 0.0f : (t - 0.92f) / 0.08f;
            const auto mixTo = [&](float target) {
                const float value = target * t + core * 0.85f;
                return static_cast<unsigned char>(value > 1.0f ? 255 : value * 255.0f);
            };
            const size_t i = (static_cast<size_t>(y) * kPaletteWidth + x) * 4;
            pixels[i + 0] = mixTo(s_colour[2]);
            pixels[i + 1] = mixTo(s_colour[1]);
            pixels[i + 2] = mixTo(s_colour[0]);
            pixels[i + 3] = 255;
        }
    }
    D3D11_SUBRESOURCE_DATA data{};
    data.pSysMem = pixels.data();
    data.SysMemPitch = kPaletteWidth * 4;
    Microsoft::WRL::ComPtr<ID3D11Texture2D> texture;
    if (FAILED(device->CreateTexture2D(&desc, &data, &texture))) return nullptr;
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
    srvDesc.Format = desc.Format;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = 1;
    ID3D11ShaderResourceView* srv = nullptr;
    if (FAILED(device->CreateShaderResourceView(texture.Get(), &srvDesc, &srv))) return nullptr;
    {
        std::lock_guard<std::mutex> lock(s_deviceMutex);
        s_tintSrvs[device] = srv;
    }
    NOBLE_INFO("ParticleTint: created tint palette SRV on device 0x{:X}", reinterpret_cast<uintptr_t>(device));
    return srv;
}

bool IsFlamePalette(ID3D11ShaderResourceView* view) {
    if (!view) return false;
    Microsoft::WRL::ComPtr<ID3D11Resource> resource;
    view->GetResource(&resource);
    if (!resource) return false;
    Microsoft::WRL::ComPtr<ID3D11Texture2D> texture;
    if (FAILED(resource.As(&texture)) || !texture) return false;
    D3D11_TEXTURE2D_DESC desc{};
    texture->GetDesc(&desc);
    return desc.Width == kPaletteWidth && desc.Height == kPaletteHeight &&
           (desc.Format == DXGI_FORMAT_B8G8R8A8_UNORM_SRGB || desc.Format == DXGI_FORMAT_B8G8R8A8_UNORM);
}

void HandlePSSetShaderResources(PSSetShaderResourcesFn original, ID3D11DeviceContext* context, UINT startSlot,
                                UINT numViews, ID3D11ShaderResourceView* const* views) {
    if (!s_active || numViews == 0 || !views) {
        original(context, startSlot, numViews, views);
        return;
    }
    std::array<ID3D11ShaderResourceView*, 16> replaced{};
    if (numViews > replaced.size()) {
        original(context, startSlot, numViews, views);
        return;
    }
    ID3D11ShaderResourceView* tint = nullptr;
    bool changed = false;
    for (UINT i = 0; i < numViews; ++i) {
        replaced[i] = views[i];
        if (IsFlamePalette(views[i])) {
            if (!tint) {
                ID3D11Device* device = nullptr;
                context->GetDevice(&device);
                tint = GetTintSrv(device);
                if (device) device->Release();
            }
            if (tint) {
                replaced[i] = tint;
                changed = true;
            }
        }
    }
    original(context, startSlot, numViews, changed ? replaced.data() : views);
}

void STDMETHODCALLTYPE HookPSImmediate(ID3D11DeviceContext* context, UINT startSlot, UINT numViews,
                                        ID3D11ShaderResourceView* const* views) {
    HandlePSSetShaderResources(s_originalImmediate, context, startSlot, numViews, views);
}

void STDMETHODCALLTYPE HookPSDeferred(ID3D11DeviceContext* context, UINT startSlot, UINT numViews,
                                       ID3D11ShaderResourceView* const* views) {
    HandlePSSetShaderResources(s_originalDeferred, context, startSlot, numViews, views);
}

void HookContext(ID3D11Device* device, ID3D11DeviceContext* context);

HRESULT STDMETHODCALLTYPE HookCreateDeferredContext(ID3D11Device* device, UINT flags,
                                                     ID3D11DeviceContext** context) {
    HRESULT hr = s_originalDeferredContext(device, flags, context);
    if (SUCCEEDED(hr) && context && *context) HookContext(device, *context);
    return hr;
}

std::set<void*> s_deferredHookTargets;

void InstallDeferredContextHook(ID3D11Device* device) {
    if (!device) return;
    void** vtable = *reinterpret_cast<void***>(device);
    if (!vtable || !vtable[31]) return;  // ID3D11Device::CreateDeferredContext
    void* target = vtable[31];
    {
        std::lock_guard<std::mutex> lock(s_deviceMutex);
        if (s_deferredHookTargets.count(target)) return;
        s_deferredHookTargets.insert(target);
    }
    if (HookManager::InstallHook(target, reinterpret_cast<void*>(&HookCreateDeferredContext),
                                  reinterpret_cast<void**>(&s_originalDeferredContext),
                                  "d3d11_create_deferred_context")) {
        NOBLE_INFO("ParticleTint: hooked ID3D11Device::CreateDeferredContext at 0x{:X}", reinterpret_cast<uintptr_t>(target));
    }
}

void HookContext(ID3D11Device* device, ID3D11DeviceContext* context) {
    if (device) InstallDeferredContextHook(device);
    if (!context) return;
    void** vtable = *reinterpret_cast<void***>(context);
    if (!vtable || !vtable[8]) return;
    void* target = vtable[8];
    {
        std::lock_guard<std::mutex> lock(s_deviceMutex);
        if (s_hookedContextTargets.count(target)) return;
    }
    const char* name = nullptr;
    if (!s_targetImmediate) {
        if (HookManager::InstallHook(target, reinterpret_cast<void*>(&HookPSImmediate),
                                      reinterpret_cast<void**>(&s_originalImmediate), "d3d11_ps_srv")) {
            s_targetImmediate = target;
            name = "immediate";
        }
    } else if (target != s_targetImmediate && !s_targetDeferred) {
        if (HookManager::InstallHook(target, reinterpret_cast<void*>(&HookPSDeferred),
                                      reinterpret_cast<void**>(&s_originalDeferred), "d3d11_ps_srv_deferred")) {
            s_targetDeferred = target;
            name = "deferred";
        }
    }
    if (name) {
        std::lock_guard<std::mutex> lock(s_deviceMutex);
        s_hookedContextTargets.insert(target);
        NOBLE_INFO("ParticleTint: hooked {} PSSetShaderResources at 0x{:X}", name, reinterpret_cast<uintptr_t>(target));
    }
}

HRESULT WINAPI HookCreateDevice(IDXGIAdapter* adapter, D3D_DRIVER_TYPE driverType, HMODULE software, UINT flags,
                                const D3D_FEATURE_LEVEL* levels, UINT numLevels, UINT sdkVersion,
                                ID3D11Device** device, D3D_FEATURE_LEVEL* featureLevel,
                                ID3D11DeviceContext** context) {
    HRESULT hr = s_originalCreateDevice(adapter, driverType, software, flags, levels, numLevels, sdkVersion, device,
                                        featureLevel, context);
    if (SUCCEEDED(hr) && device && context && *device && *context) HookContext(*device, *context);
    return hr;
}

HRESULT WINAPI HookCreateDeviceAndSwapChain(IDXGIAdapter* adapter, D3D_DRIVER_TYPE driverType, HMODULE software,
                                            UINT flags, const D3D_FEATURE_LEVEL* levels, UINT numLevels, UINT sdkVersion,
                                            const DXGI_SWAP_CHAIN_DESC* swapChainDesc, IDXGISwapChain** swapChain,
                                            ID3D11Device** device, D3D_FEATURE_LEVEL* featureLevel,
                                            ID3D11DeviceContext** context) {
    HRESULT hr = s_originalCreateDeviceSwapChain(adapter, driverType, software, flags, levels, numLevels, sdkVersion,
                                                 swapChainDesc, swapChain, device, featureLevel, context);
    if (SUCCEEDED(hr) && device && context && *device && *context) HookContext(*device, *context);
    return hr;
}

void InstallExportHooks() {
    if (s_exportsHooked) return;
    HMODULE d3d11 = GetModuleHandleW(L"d3d11.dll");
    if (!d3d11) {
        NOBLE_WARN("ParticleTint: d3d11.dll not loaded yet; engine device creation will not be captured");
        return;
    }
    void* createDevice = reinterpret_cast<void*>(GetProcAddress(d3d11, "D3D11CreateDevice"));
    void* createDeviceSwap = reinterpret_cast<void*>(GetProcAddress(d3d11, "D3D11CreateDeviceAndSwapChain"));
    if (createDevice) {
        HookManager::InstallHook(createDevice, reinterpret_cast<void*>(&HookCreateDevice),
                                 reinterpret_cast<void**>(&s_originalCreateDevice), "d3d11_create_device");
    }
    if (createDeviceSwap) {
        HookManager::InstallHook(createDeviceSwap, reinterpret_cast<void*>(&HookCreateDeviceAndSwapChain),
                                 reinterpret_cast<void**>(&s_originalCreateDeviceSwapChain),
                                 "d3d11_create_device_swapchain");
    }
    s_exportsHooked = true;
}

} // namespace

void Install(ID3D11Device* device, ID3D11DeviceContext* context) {
    if (s_hookInstalled) return;
    s_hookInstalled = true;
    ParseTintColour();
    if (!s_active) return;
    InstallExportHooks();
    HookContext(device, context);
}

void Shutdown() {
    s_active = false;
    std::lock_guard<std::mutex> lock(s_deviceMutex);
    for (auto& pair : s_tintSrvs) {
        if (pair.second) pair.second->Release();
    }
    s_tintSrvs.clear();
}

} // namespace noble::ParticleTint
