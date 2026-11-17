#include "ParticleTint.hpp"

#include "common/Logger.hpp"
#include "hooks/HookManager.hpp"

#include <windows.h>
#include <shellapi.h>
#include <d3d11.h>
#include <d3d11_1.h>
#include <d3d11_2.h>
#include <d3d11_3.h>
#include <d3d11_4.h>
#include <dxgi.h>
#include <wrl/client.h>
#include <array>
#include <atomic>
#include <cstdio>
#include <mutex>
#include <set>
#include <string>
#include <unordered_map>

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

constexpr int kMaxHooks = 8;
std::atomic<bool> s_active{false};
std::atomic<bool> s_armed{false};
float s_colour[3] = {1.0f, 0.0f, 1.0f};

PSSetShaderResourcesFn s_originals[kMaxHooks] = {};
void* s_targets[kMaxHooks] = {};
int s_hookCount = 0;
std::set<void*> s_targetSet;

CreateDeferredContextFn s_originalDeferredContext = nullptr;
CreateDeviceFn s_originalCreateDevice = nullptr;
CreateDeviceAndSwapChainFn s_originalCreateDeviceSwapChain = nullptr;
bool s_exportsHooked = false;

std::mutex s_deviceMutex;
std::unordered_map<ID3D11Device*, ID3D11ShaderResourceView*> s_tintSrvs;
std::set<void*> s_deferredHookTargets;

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

bool IsFlamePalette(ID3D11ShaderResourceView* view, D3D11_TEXTURE2D_DESC* outDesc = nullptr) {
    if (!view) return false;
    Microsoft::WRL::ComPtr<ID3D11Resource> resource;
    view->GetResource(&resource);
    if (!resource) return false;
    Microsoft::WRL::ComPtr<ID3D11Texture2D> texture;
    if (FAILED(resource.As(&texture)) || !texture) return false;
    D3D11_TEXTURE2D_DESC desc{};
    texture->GetDesc(&desc);
    if (outDesc) *outDesc = desc;
    return desc.Width == kPaletteWidth && desc.Height == kPaletteHeight &&
           (desc.Format == DXGI_FORMAT_B8G8R8A8_UNORM_SRGB || desc.Format == DXGI_FORMAT_B8G8R8A8_UNORM);
}

void HandlePSSetShaderResources(PSSetShaderResourcesFn original, ID3D11DeviceContext* context, UINT startSlot,
                                UINT numViews, ID3D11ShaderResourceView* const* views) {
    static std::atomic<int> s_calls{0};
    const int callNo = ++s_calls;
    if (callNo == 1 || callNo == 100 || callNo == 1000 || callNo == 5000)
        NOBLE_INFO("ParticleTint: PSSet hook call #{} (startSlot={} numViews={})", callNo, startSlot, numViews);
    if (!s_active || numViews == 0 || !views) {
        original(context, startSlot, numViews, views);
        return;
    }
    {
        static std::mutex s_descMutex;
        static std::set<std::string> s_seen;
        std::lock_guard<std::mutex> lock(s_descMutex);
        for (UINT d = 0; d < numViews; ++d) {
            if (!views[d]) continue;
            Microsoft::WRL::ComPtr<ID3D11Resource> res;
            views[d]->GetResource(&res);
            Microsoft::WRL::ComPtr<ID3D11Texture2D> tex;
            if (!res || FAILED(res.As(&tex)) || !tex) continue;
            D3D11_TEXTURE2D_DESC td{};
            tex->GetDesc(&td);
            char key[96];
            std::snprintf(key, sizeof(key), "%ux%u fmt=%d slot=%u", td.Width, td.Height, static_cast<int>(td.Format), startSlot + d);
            if (td.Height <= 64 || td.Width <= 64) {
                char key2[96];
                std::snprintf(key2, sizeof(key2), "SMALL %ux%u fmt=%d slot=%u", td.Width, td.Height,
                              static_cast<int>(td.Format), startSlot + d);
                if (s_seen.insert(key2).second && s_seen.size() < 200) NOBLE_INFO("ParticleTint: bound texture {}", key2);
            } else if (s_seen.insert(key).second && s_seen.size() < 40) NOBLE_INFO("ParticleTint: bound texture {}", key);
        }
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
        D3D11_TEXTURE2D_DESC matchedDesc{};
        if (IsFlamePalette(views[i], &matchedDesc)) {
            static std::atomic<bool> s_loggedMatch{true};
            if (s_loggedMatch.exchange(false)) {
                NOBLE_INFO("ParticleTint: matched palette {}x{} fmt={}", matchedDesc.Width, matchedDesc.Height,
                           static_cast<int>(matchedDesc.Format));
            }
            if (!tint) {
                ID3D11Device* device = nullptr;
                context->GetDevice(&device);
                tint = GetTintSrv(device);
                if (device) device->Release();
            }
            if (tint) {
                replaced[i] = tint;
                changed = true;
                static std::atomic<bool> s_firstMatch{true};
                if (s_firstMatch.exchange(false)) NOBLE_INFO("ParticleTint: palette substituted");
            }
        }
    }
    original(context, startSlot, numViews, changed ? replaced.data() : views);
}

#define DEFINE_PS_HOOK(N)                                                                                 \
    void STDMETHODCALLTYPE HookPS##N(ID3D11DeviceContext* c, UINT s, UINT n, ID3D11ShaderResourceView* const* v) { \
        HandlePSSetShaderResources(s_originals[N], c, s, n, v);                                           \
    }
DEFINE_PS_HOOK(0)
DEFINE_PS_HOOK(1)
DEFINE_PS_HOOK(2)
DEFINE_PS_HOOK(3)
DEFINE_PS_HOOK(4)
DEFINE_PS_HOOK(5)
DEFINE_PS_HOOK(6)
DEFINE_PS_HOOK(7)

void* const kDetours[kMaxHooks] = {
    reinterpret_cast<void*>(&HookPS0), reinterpret_cast<void*>(&HookPS1), reinterpret_cast<void*>(&HookPS2),
    reinterpret_cast<void*>(&HookPS3), reinterpret_cast<void*>(&HookPS4), reinterpret_cast<void*>(&HookPS5),
    reinterpret_cast<void*>(&HookPS6), reinterpret_cast<void*>(&HookPS7),
};

void HookContextTarget(void* target, const char* label) {
    if (!target) return;
    std::lock_guard<std::mutex> lock(s_deviceMutex);
    if (s_targetSet.count(target)) return;
    if (s_hookCount >= kMaxHooks) {
        NOBLE_WARN("ParticleTint: no hook slot left for {} at 0x{:X}", label, reinterpret_cast<uintptr_t>(target));
        return;
    }
    const int slot = s_hookCount;
    char name[64];
    std::snprintf(name, sizeof(name), "d3d11_ps_srv_%d", slot);
    if (!HookManager::InstallHook(target, kDetours[slot], reinterpret_cast<void**>(&s_originals[slot]), name)) return;
    s_targets[slot] = target;
    s_targetSet.insert(target);
    ++s_hookCount;
    NOBLE_INFO("ParticleTint: hooked {} PSSetShaderResources at 0x{:X} (slot {})", label,
               reinterpret_cast<uintptr_t>(target), slot);
}

void InstallDeferredContextHook(ID3D11Device* device);
void HookContext(ID3D11Device*, ID3D11DeviceContext* context);

HRESULT STDMETHODCALLTYPE HookCreateDeferredContext(ID3D11Device* device, UINT flags,
                                                     ID3D11DeviceContext** context) {
    HRESULT hr = s_originalDeferredContext(device, flags, context);
    if (SUCCEEDED(hr) && context && *context) HookContext(device, *context);
    return hr;
}

template <typename T>
void HookContextInterface(ID3D11DeviceContext* context, const char* label) {
    Microsoft::WRL::ComPtr<T> iface;
    if (FAILED(context->QueryInterface(__uuidof(T), reinterpret_cast<void**>(iface.GetAddressOf()))) || !iface) return;
    void** vtable = *reinterpret_cast<void***>(iface.Get());
    if (vtable && vtable[8]) HookContextTarget(vtable[8], label);
}

void HookContext(ID3D11Device* device, ID3D11DeviceContext* context) {
    if (device) InstallDeferredContextHook(device);
    if (!context) return;
    HookContextInterface<ID3D11DeviceContext>(context, "base");
    HookContextInterface<ID3D11DeviceContext1>(context, "context1");
    HookContextInterface<ID3D11DeviceContext2>(context, "context2");
    HookContextInterface<ID3D11DeviceContext3>(context, "context3");
    HookContextInterface<ID3D11DeviceContext4>(context, "context4");
}

void InstallDeferredContextHook(ID3D11Device* device) {
    if (!device) return;
    void** vtable = *reinterpret_cast<void***>(device);
    if (!vtable || !vtable[31]) return;
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
    if (!d3d11) d3d11 = LoadLibraryW(L"d3d11.dll");
    if (!d3d11) {
        NOBLE_WARN("ParticleTint: could not load d3d11.dll for export hooks");
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

void HookFromSwapChain(IDXGISwapChain* swapChain) {
    if (!swapChain) return;
    Microsoft::WRL::ComPtr<ID3D11Device> device;
    if (FAILED(swapChain->GetDevice(__uuidof(ID3D11Device), reinterpret_cast<void**>(device.GetAddressOf()))) || !device) return;
    Microsoft::WRL::ComPtr<ID3D11DeviceContext> context;
    device->GetImmediateContext(context.GetAddressOf());
    if (context) {
        std::lock_guard<std::mutex> lock(s_deviceMutex);
        static std::set<ID3D11DeviceContext*> s_seenContexts;
        if (s_seenContexts.insert(context.Get()).second) {
            NOBLE_INFO("ParticleTint: hooking context 0x{:X} from swapchain 0x{:X}",
                       reinterpret_cast<uintptr_t>(context.Get()), reinterpret_cast<uintptr_t>(swapChain));
        }
    }
    HookContext(device.Get(), context.Get());
}

void Install(ID3D11Device* device, ID3D11DeviceContext* context) {
    if (!s_armed.exchange(true)) {
        ParseTintColour();
        if (s_active) InstallExportHooks();
    }
    if (s_active) HookContext(device, context);
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
