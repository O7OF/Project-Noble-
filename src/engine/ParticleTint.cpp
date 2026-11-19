#include "ParticleTint.hpp"

#include "common/Logger.hpp"
#include "common/ArmorEffects.hpp"
#include "common/EngineModule.hpp"
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
#include <cstring>
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
std::unordered_map<ID3D11Device*, std::array<ID3D11ShaderResourceView*, 30>> s_deviceVariantSrvs;
std::set<void*> s_deferredHookTargets;

// Per-biped multiplayer flame tracking
std::mutex s_bipedMutex;
std::unordered_map<uint32_t, int> s_bipedVariants;
std::atomic<uint32_t> s_lastRenderedBiped{0xFFFFFFFFu};
std::atomic<int> s_localVariantIndex{-1};
std::atomic<int> s_bipedVariantCount{0};   // size of s_bipedVariants, readable without the mutex

// Performance (2026-11-18): PSSetShaderResources is called thousands of times per frame. Each new frame (one
// Present) starts a new epoch; within an epoch each shader resource view is classified once (palette or not)
// and the answer is kept in a small per-thread table, instead of three COM calls on every bind.
std::atomic<uint32_t> s_frameEpoch{1};
std::atomic<IDXGISwapChain*> s_lastHookedSwapChain{nullptr};
std::atomic<ID3D11DeviceContext*> s_lastHookedContext{nullptr};

constexpr UINT kPaletteWidth = 256;
constexpr UINT kPaletteHeight = 16;

void ParseTintColour() {
    int argc = 0;
    LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);
    if (!argv) return;
    std::string spec;
    int variant = -1;
    int primary = -1;
    for (int i = 1; i + 1 < argc; ++i) {
        if (_wcsicmp(argv[i], L"-armor-fx-runtime-variant") == 0) {
            variant = _wtoi(argv[i + 1]);
        } else if (_wcsicmp(argv[i], L"-armor-fx-runtime-primary") == 0) {
            primary = _wtoi(argv[i + 1]);
        } else if (_wcsicmp(argv[i], L"-armor-fx-runtime-tint") == 0) {
            const int len = WideCharToMultiByte(CP_UTF8, 0, argv[i + 1], -1, nullptr, 0, nullptr, nullptr);
            if (len > 1) {
                spec.resize(static_cast<size_t>(len - 1));
                WideCharToMultiByte(CP_UTF8, 0, argv[i + 1], -1, spec.data(), len, nullptr, nullptr);
            }
        }
    }
    LocalFree(argv);
    if (variant >= 0 || primary >= 0) {
        const int index = variant >= 0 ? variant : primary;
        if (index < 0 || index >= static_cast<int>(kPrimaryFlames.size())) return;
        s_localVariantIndex.store(index);
        s_colour[0] = kPrimaryFlames[static_cast<size_t>(index)].r;
        s_colour[1] = kPrimaryFlames[static_cast<size_t>(index)].g;
        s_colour[2] = kPrimaryFlames[static_cast<size_t>(index)].b;
        s_active = true;
        NOBLE_INFO("ParticleTint: armed for {} {} (RGB {:.2f},{:.2f},{:.2f})", variant >= 0 ? "variant" : "primary",
                   kPrimaryFlames[static_cast<size_t>(index)].id, s_colour[0], s_colour[1], s_colour[2]);
        return;
    }
    if (spec.empty()) return;
    unsigned rr = 0, gg = 0, bb = 0;
    if (sscanf_s(spec.c_str(), "%u,%u,%u", &rr, &gg, &bb) == 3) {
        s_colour[0] = static_cast<float>(rr & 0xFF) / 255.0f;
        s_colour[1] = static_cast<float>(gg & 0xFF) / 255.0f;
        s_colour[2] = static_cast<float>(bb & 0xFF) / 255.0f;
    } else if (spec.size() == 6 && sscanf_s(spec.c_str(), "%2x%2x%2x", &rr, &gg, &bb) == 3) {
        s_colour[0] = static_cast<float>(rr) / 255.0f;
        s_colour[1] = static_cast<float>(gg) / 255.0f;
        s_colour[2] = static_cast<float>(bb) / 255.0f;
    } else {
        return;
    }
    s_active = true;
    NOBLE_INFO("ParticleTint: armed for RGB ({:.2f},{:.2f},{:.2f})", s_colour[0], s_colour[1], s_colour[2]);
}

ID3D11ShaderResourceView* BuildPaletteSrv(ID3D11Device* device, float r, float g, float b) {
    if (!device) return nullptr;
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
            pixels[i + 0] = mixTo(b);
            pixels[i + 1] = mixTo(g);
            pixels[i + 2] = mixTo(r);
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
    return srv;
}

ID3D11ShaderResourceView* GetVariantSrv(ID3D11Device* device, int variantIndex) {
    if (!device || variantIndex < 0 || variantIndex >= static_cast<int>(kPrimaryFlames.size())) return nullptr;
    const size_t vIdx = static_cast<size_t>(variantIndex);
    {
        std::lock_guard<std::mutex> lock(s_deviceMutex);
        auto it = s_deviceVariantSrvs.find(device);
        if (it != s_deviceVariantSrvs.end() && it->second[vIdx]) return it->second[vIdx];
    }
    const auto& entry = kPrimaryFlames[vIdx];
    ID3D11ShaderResourceView* srv = BuildPaletteSrv(device, entry.r, entry.g, entry.b);
    if (!srv) return nullptr;
    {
        std::lock_guard<std::mutex> lock(s_deviceMutex);
        s_deviceVariantSrvs[device][vIdx] = srv;
    }
    NOBLE_INFO("ParticleTint: created palette SRV for variant {} ({}) on device 0x{:X}",
               variantIndex, entry.id, reinterpret_cast<uintptr_t>(device));
    return srv;
}

ID3D11ShaderResourceView* GetTintSrv(ID3D11Device* device) {
    if (!device) return nullptr;
    {
        std::lock_guard<std::mutex> lock(s_deviceMutex);
        auto it = s_tintSrvs.find(device);
        if (it != s_tintSrvs.end()) return it->second;
    }
    ID3D11ShaderResourceView* srv = BuildPaletteSrv(device, s_colour[0], s_colour[1], s_colour[2]);
    if (!srv) return nullptr;
    {
        std::lock_guard<std::mutex> lock(s_deviceMutex);
        s_tintSrvs[device] = srv;
    }
    NOBLE_INFO("ParticleTint: created custom tint SRV on device 0x{:X}", reinterpret_cast<uintptr_t>(device));
    return srv;
}

bool IsFlamePalette(ID3D11ShaderResourceView* view, D3D11_TEXTURE2D_DESC* outDesc = nullptr) {
    if (!view) return false;
    Microsoft::WRL::ComPtr<ID3D11Resource> resource;
    view->GetResource(&resource);
    if (!resource) return false;
    Microsoft::WRL::ComPtr<ID3D11Texture2D> texture;
    if (FAILED(resource->QueryInterface(__uuidof(ID3D11Texture2D), reinterpret_cast<void**>(texture.GetAddressOf()))) || !texture) return false;
    D3D11_TEXTURE2D_DESC desc{};
    texture->GetDesc(&desc);
    if (outDesc) *outDesc = desc;
    return desc.Width == kPaletteWidth && desc.Height == kPaletteHeight &&
           (desc.Format == DXGI_FORMAT_B8G8R8A8_UNORM_SRGB || desc.Format == DXGI_FORMAT_B8G8R8A8_UNORM ||
            desc.Format == DXGI_FORMAT_R8G8B8A8_UNORM_SRGB || desc.Format == DXGI_FORMAT_R8G8B8A8_UNORM ||
            desc.Format == DXGI_FORMAT_B8G8R8X8_UNORM_SRGB || desc.Format == DXGI_FORMAT_B8G8R8X8_UNORM);
}

struct PaletteCacheEntry {
    ID3D11ShaderResourceView* view;
    uint32_t epoch;
    bool isPalette;
};
constexpr size_t kPaletteCacheSize = 1024;   // power of two
thread_local PaletteCacheEntry t_paletteCache[kPaletteCacheSize] = {};

bool IsFlamePaletteCached(ID3D11ShaderResourceView* view) {
    const uint32_t epoch = s_frameEpoch.load(std::memory_order_relaxed);
    const size_t slot = (reinterpret_cast<uintptr_t>(view) >> 4) & (kPaletteCacheSize - 1);
    PaletteCacheEntry& e = t_paletteCache[slot];
    if (e.view == view && e.epoch == epoch) return e.isPalette;
    const bool result = IsFlamePalette(view);
    e.view = view;
    e.epoch = epoch;
    e.isPalette = result;
    return result;
}

// True when no flame colour is in use anywhere: then every bind passes straight through with no extra work.
bool NothingToTint() {
    return !s_active.load(std::memory_order_relaxed) &&
           s_localVariantIndex.load(std::memory_order_relaxed) < 0 &&
           s_bipedVariantCount.load(std::memory_order_relaxed) == 0;
}

// ---- Which character owns the flame being drawn (2026-11-19) ----
// The engine queues each particle emitter's draw and runs it later from its sorted transparent pass. The queued draw
// (one call per emitter) carries its particle system index; particle system -> parent effect -> the object the effect
// is attached to. A hook on that draw records the owner for the duration of the call, so the palette bind inside it
// can use the owner's flame colour instead of guessing. Data-array layout as the engine's own datum lookup reads it:
// count +0x44, element size +0x20, elements +0x50, first u16 of an element = salt.
constexpr uint32_t kNoOwner = 0xFFFFFFFFu;
constexpr uintptr_t kEmitterDrawRva = 0x27B6C4;
constexpr uintptr_t kTlsIndexRva = 0xC17B18;
constexpr size_t kTlsParticleSystems = 0x270;
constexpr size_t kTlsEffects = 0x68;
constexpr size_t kParticleSystemStride = 0x54;
constexpr size_t kParticleSystemEffect = 0x14;
constexpr size_t kEffectObject = 0x3C;
constexpr uint8_t kEmitterDrawBytes[] = {0x48, 0x8B, 0xC4, 0x48, 0x89, 0x48, 0x08, 0x55, 0x53, 0x56, 0x57, 0x41, 0x54, 0x41, 0x55};

using EmitterDrawFn = void(__fastcall*)(uint8_t*);
EmitterDrawFn s_originalEmitterDraw = nullptr;
std::atomic<bool> s_ownerHookTried{false};
std::atomic<bool> s_ownerHookActive{false};
thread_local uint32_t t_flameOwner = kNoOwner;
std::atomic<uint32_t> s_statBinds{0}, s_statOwned{0}, s_statMatched{0};
std::atomic<ULONGLONG> s_statLastLog{0};
std::atomic<uint32_t> s_statLastOwner{0xFFFFFFFFu};
std::atomic<ULONGLONG> s_lastRegisterMs{0};   // a match is registering players' flame colours
std::atomic<uint32_t> s_statRebinds{0};

// The engine skips re-binding the flame palette when the next flame uses the same one, so a player's flames drawn
// right after a level fire would keep the fire's (stock) palette. Remember, per render thread, which flame palette
// is bound right now so each flame draw can switch it to its owner's colour itself.
constexpr int kNoPalette = INT_MIN;
constexpr int kBoundCustom = -3;
thread_local ID3D11DeviceContext* t_palCtx = nullptr;
thread_local PSSetShaderResourcesFn t_palOriginal = nullptr;
thread_local ID3D11ShaderResourceView* t_palStock = nullptr;
thread_local int t_palBound = kNoPalette;

// What an owner's flames should use: a variant (>= 0), kStockFlame, or -1 = no decision (old fallback).
int OwnerDecision(uint32_t owner) {
    if (owner == kNoOwner) return -1;
    const int v = GetBipedFlameVariant(owner);
    if (v == kStockFlame || v >= 0) return v;
    if (s_ownerHookActive.load(std::memory_order_relaxed) &&
        GetTickCount64() - s_lastRegisterMs.load(std::memory_order_relaxed) < 2000) {
        return kStockFlame;   // a level fire, an AI, a vehicle
    }
    return -1;
}

void RebindPaletteForOwner(uint32_t owner) {
    if (owner == kNoOwner || !t_palCtx || !t_palOriginal || !t_palStock || t_palBound == kNoPalette) return;
    if (NothingToTint()) return;
    const int want = OwnerDecision(owner);
    if (want == -1 || want == t_palBound) return;
    ID3D11ShaderResourceView* view = nullptr;
    if (want == kStockFlame) {
        view = t_palStock;
    } else if (want < static_cast<int>(kPrimaryFlames.size())) {
        ID3D11Device* device = nullptr;
        t_palCtx->GetDevice(&device);
        if (device) {
            view = GetVariantSrv(device, want);
            device->Release();
        }
    }
    if (!view) return;
    t_palOriginal(t_palCtx, 2, 1, &view);
    t_palBound = want;
    s_statRebinds.fetch_add(1, std::memory_order_relaxed);
}
#ifdef NOBLE_DEV_BUILD
std::atomic<bool> s_diagBusy{false};
std::atomic<uint32_t> s_diagOwner{0xFFFFFFFFu};
uint8_t s_diagEntry[0x18] = {};
uint8_t s_diagObj[0x200] = {};
#endif

const uint8_t* DatumGet(const uint8_t* array, uint32_t datum) {
    if (!array || datum == kNoOwner) return nullptr;
    const int32_t count = *reinterpret_cast<const int32_t*>(array + 0x44);
    const uint32_t index = datum & 0xFFFFu;
    if (static_cast<int32_t>(index) >= count) return nullptr;
    const uint64_t size = *reinterpret_cast<const uint64_t*>(array + 0x20);
    const uint8_t* data = *reinterpret_cast<const uint8_t* const*>(array + 0x50);
    if (!data || size == 0 || size > 0x10000) return nullptr;
    const uint8_t* e = data + size * index;
    const uint16_t salt = *reinterpret_cast<const uint16_t*>(e);
    return (salt != 0 && salt == static_cast<uint16_t>(datum >> 16)) ? e : nullptr;
}

uint32_t ResolveFlameOwner(const uint8_t* emitterDraw) {
    __try {
        const auto base = reinterpret_cast<uintptr_t>(noble::ReachModule());
        if (!base || !emitterDraw) return kNoOwner;
        const uint32_t tlsIndex = *reinterpret_cast<const uint32_t*>(base + kTlsIndexRva);
        const auto tlsVec = static_cast<uintptr_t>(__readgsqword(0x58));
        const uintptr_t tls = *reinterpret_cast<const uintptr_t*>(tlsVec + static_cast<uintptr_t>(tlsIndex) * 8);
        if (!tls) return kNoOwner;
        const uint8_t* systems = *reinterpret_cast<const uint8_t* const*>(tls + kTlsParticleSystems);
        if (!systems) return kNoOwner;
        const uint16_t psIndex = *reinterpret_cast<const uint16_t*>(emitterDraw + 4);
        if (static_cast<int32_t>(psIndex) >= *reinterpret_cast<const int32_t*>(systems + 0x44)) return kNoOwner;
        const uint8_t* psData = *reinterpret_cast<const uint8_t* const*>(systems + 0x50);
        if (!psData) return kNoOwner;
        const uint8_t* ps = psData + static_cast<size_t>(psIndex) * kParticleSystemStride;
        const uint32_t effectDatum = *reinterpret_cast<const uint32_t*>(ps + kParticleSystemEffect);
        const uint8_t* effects = *reinterpret_cast<const uint8_t* const*>(tls + kTlsEffects);
        const uint8_t* effect = DatumGet(effects, effectDatum);
        if (!effect) return kNoOwner;
        uint32_t obj = *reinterpret_cast<const uint32_t*>(effect + kEffectObject);
        // The armour effect can sit on an attachment object; walk up to the root object (the character), the way the
        // engine itself finds an effect's location. Object table: TLS +0x10, entries +0x50 (0x18 each, object
        // pointer at +0x10); an object's parent is at +0x14.
        const uint8_t* objects = *reinterpret_cast<const uint8_t* const*>(tls + 0x10);
        if (!objects) return obj;
        const int32_t maxObjects = *reinterpret_cast<const int32_t*>(objects + 0x44);
        const uint8_t* objEntries = *reinterpret_cast<const uint8_t* const*>(objects + 0x50);
        for (int depth = 0; depth < 8 && obj != kNoOwner && objEntries; ++depth) {
            const uint32_t idx = obj & 0xFFFFu;
            if (static_cast<int32_t>(idx) >= maxObjects) break;
            const uint8_t* entry = objEntries + static_cast<size_t>(idx) * 0x18;
            if (*reinterpret_cast<const uint16_t*>(entry) != static_cast<uint16_t>(obj >> 16)) break;
            const uint8_t* objPtr = *reinterpret_cast<const uint8_t* const*>(entry + 0x10);
            if (!objPtr) break;
            const uint32_t parent = *reinterpret_cast<const uint32_t*>(objPtr + 0x14);
            if (parent == kNoOwner || parent == obj) break;
            obj = parent;
        }
#ifdef NOBLE_DEV_BUILD
        // Dev: keep a copy of the final owner object so the stats log can show how it links to a player.
        if (!s_diagBusy.exchange(true)) {
            const uint32_t idx = obj & 0xFFFFu;
            if (objEntries && obj != kNoOwner && static_cast<int32_t>(idx) < maxObjects) {
                const uint8_t* entry = objEntries + static_cast<size_t>(idx) * 0x18;
                const uint8_t* objPtr = *reinterpret_cast<const uint8_t* const*>(entry + 0x10);
                if (objPtr) {
                    std::memcpy(s_diagEntry, entry, sizeof(s_diagEntry));
                    std::memcpy(s_diagObj, objPtr, sizeof(s_diagObj));
                    s_diagOwner.store(obj);
                }
            }
            s_diagBusy.store(false);
        }
#endif
        return obj;
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        return kNoOwner;
    }
}

void __fastcall EmitterDrawDetour(uint8_t* emitterDraw) {
    const uint32_t previous = t_flameOwner;
    t_flameOwner = ResolveFlameOwner(emitterDraw);
    RebindPaletteForOwner(t_flameOwner);
    s_originalEmitterDraw(emitterDraw);
    t_flameOwner = previous;
}

bool EntryBytesMatch(const uint8_t* target) {
    __try {
        return std::memcmp(target, kEmitterDrawBytes, sizeof(kEmitterDrawBytes)) == 0;
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        return false;
    }
}

// Installed once the game module is loaded. All-or-nothing: if the entry bytes don't match this game build the hook
// is skipped and flame colours fall back to the old behaviour.
void TryInstallOwnerHook() {
    if (s_ownerHookActive.load() || !noble::ReachModule()) return;
    if (s_ownerHookTried.exchange(true)) return;
    const auto base = reinterpret_cast<uint8_t*>(noble::ReachModule());
    uint8_t* target = base + kEmitterDrawRva;
    if (!EntryBytesMatch(target)) {
        NOBLE_WARN("ParticleTint: flame owner tracking not installed (unexpected game build); using the old flame colour matching");
        return;
    }
    if (noble::HookManager::InstallHook(target, reinterpret_cast<void*>(&EmitterDrawDetour),
                                        reinterpret_cast<void**>(&s_originalEmitterDraw), "particle_emitter_draw")) {
        s_ownerHookActive.store(true);
        NOBLE_INFO("ParticleTint: flame owner tracking installed (each player's flames use that player's colour)");
    }
}

void LogOwnerStats() {
#ifdef NOBLE_DEV_BUILD
    const ULONGLONG now = GetTickCount64();
    ULONGLONG last = s_statLastLog.load(std::memory_order_relaxed);
    if (now - last < 10000) return;
    if (!s_statLastLog.compare_exchange_strong(last, now)) return;
    const uint32_t binds = s_statBinds.exchange(0), owned = s_statOwned.exchange(0), matched = s_statMatched.exchange(0);
    if (binds) {
        std::string reg;
        {
            std::lock_guard<std::mutex> lock(s_bipedMutex);
            for (const auto& kv : s_bipedVariants) reg += fmt::format(" {:08X}={}", kv.first, kv.second);
        }
        NOBLE_INFO("ParticleTint: last 10 s: {} flame palette binds, {} with a known owner, {} owner had a registered colour, {} switched per flame (last owner {:08X}; registered:{})",
                   binds, owned, matched, s_statRebinds.exchange(0), s_statLastOwner.load(), reg.empty() ? std::string(" none") : reg);
        if (!s_diagBusy.exchange(true)) {
            uint8_t entry[0x18], obj[0x200];
            std::memcpy(entry, s_diagEntry, sizeof(entry));
            std::memcpy(obj, s_diagObj, sizeof(obj));
            const uint32_t diagOwner = s_diagOwner.load();
            s_diagBusy.store(false);
            std::string links;
            {
                std::lock_guard<std::mutex> lock(s_bipedMutex);
                for (size_t off = 0; off + 4 <= sizeof(obj); off += 4) {
                    uint32_t v;
                    std::memcpy(&v, obj + off, 4);
                    if (s_bipedVariants.count(v)) links += fmt::format(" +{:X}={:08X}", off, v);
                }
            }
            std::string head, ent;
            for (size_t off = 0; off < 0x40; off += 4) {
                uint32_t v;
                std::memcpy(&v, obj + off, 4);
                head += fmt::format(" {:08X}", v);
            }
            for (size_t i = 0; i < 8; ++i) ent += fmt::format("{:02X}", entry[i]);
            NOBLE_INFO("ParticleTint: owner {:08X} entry {} head{} player links:{}", diagOwner, ent, head,
                       links.empty() ? std::string(" none") : links);
        }
    }
#endif
}

void HandlePSSetShaderResources(PSSetShaderResourcesFn original, ID3D11DeviceContext* context, UINT startSlot,
                                UINT numViews, ID3D11ShaderResourceView* const* views) {
    if (numViews == 0 || !views || NothingToTint()) {
        original(context, startSlot, numViews, views);
        return;
    }
    // Only slot 2 can hold the Reach armor flame palette
    if (startSlot > 2 || (startSlot + numViews) <= 2) {
        original(context, startSlot, numViews, views);
        return;
    }
    const UINT slot2Idx = 2 - startSlot;
    if (!views[slot2Idx] || !IsFlamePaletteCached(views[slot2Idx])) {
        if (t_palCtx == context) t_palBound = kNoPalette;   // slot 2 no longer holds a flame palette
        original(context, startSlot, numViews, views);
        return;
    }
    t_palCtx = context;
    t_palOriginal = original;
    t_palStock = views[slot2Idx];
    t_palBound = kStockFlame;

    // A flame palette at slot 2 is being bound! Resolve which flame variant to use:
    int variantToUse = -1;
    bool ownerResolved = false;
    s_statBinds.fetch_add(1, std::memory_order_relaxed);
    LogOwnerStats();

    // 0. Best: the engine draw that is running right now told us whose flame this is.
    const uint32_t owner = t_flameOwner;
    if (owner != kNoOwner) {
        s_statOwned.fetch_add(1, std::memory_order_relaxed);
        s_statLastOwner.store(owner, std::memory_order_relaxed);
        const int ownerVariant = GetBipedFlameVariant(owner);
        if (ownerVariant == kStockFlame) {
            s_statMatched.fetch_add(1, std::memory_order_relaxed);
            original(context, startSlot, numViews, views);   // this player's flames stay stock
            return;
        }
        if (ownerVariant >= 0) {
            s_statMatched.fetch_add(1, std::memory_order_relaxed);
            variantToUse = ownerVariant;
            ownerResolved = true;
        } else if (s_ownerHookActive.load(std::memory_order_relaxed) &&
                   GetTickCount64() - s_lastRegisterMs.load(std::memory_order_relaxed) < 2000) {
            // The owner is known but is not a player with a Noble colour (a level fire, an AI, a vehicle): stock.
            original(context, startSlot, numViews, views);
            return;
        }
    }

    // 1. Fallback: try reading the currently rendered object from haloreach.dll+0xB43B38
    HMODULE hReach = noble::ReachModule();
    if (!ownerResolved && hReach) {
        uintptr_t base = reinterpret_cast<uintptr_t>(hReach);
        uint32_t currObj = 0xFFFFFFFFu;
        __try {
            currObj = *reinterpret_cast<const uint32_t*>(base + 0xB43B38);
        } __except (EXCEPTION_EXECUTE_HANDLER) {}
        if (currObj != 0xFFFFFFFFu) {
            variantToUse = GetBipedFlameVariant(currObj);
        }
        if (variantToUse < 0) {
            const uint32_t lastBiped = s_lastRenderedBiped.load(std::memory_order_relaxed);
            if (lastBiped != 0xFFFFFFFFu) {
                variantToUse = GetBipedFlameVariant(lastBiped);
            }
        }
    }

    // 2. If not matched to a remote biped, use the local player's equipped/previewed variant
    if (variantToUse < 0) {
        variantToUse = s_localVariantIndex.load(std::memory_order_relaxed);
    }

    ID3D11ShaderResourceView* tint = nullptr;
    ID3D11Device* device = nullptr;
    context->GetDevice(&device);
    if (device) {
        if (variantToUse >= 0 && variantToUse < static_cast<int>(kPrimaryFlames.size())) {
            tint = GetVariantSrv(device, variantToUse);
        } else if (s_active) {
            tint = GetTintSrv(device);
        }
        device->Release();
    }

    if (tint) {
        std::array<ID3D11ShaderResourceView*, 16> replaced{};
        if (numViews <= replaced.size()) {
            for (UINT k = 0; k < numViews; ++k) replaced[k] = views[k];
            replaced[slot2Idx] = tint;
            t_palBound = (variantToUse >= 0 && variantToUse < static_cast<int>(kPrimaryFlames.size())) ? variantToUse
                                                                                                         : kBoundCustom;
            original(context, startSlot, numViews, replaced.data());
            return;
        }
    }
    original(context, startSlot, numViews, views);
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
    // Called on every Present: start a new classification epoch, and only do the (costly) hook installation the
    // first time a swap chain is seen. Deferred contexts created later are caught by the CreateDeferredContext hook.
    s_frameEpoch.fetch_add(1, std::memory_order_relaxed);
    if (s_lastHookedSwapChain.load(std::memory_order_relaxed) == swapChain) return;
    s_lastHookedSwapChain.store(swapChain, std::memory_order_relaxed);
    TryInstallOwnerHook();
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

void HookFromContext(ID3D11DeviceContext* context) {
    if (!context) return;
    if (s_lastHookedContext.load(std::memory_order_relaxed) == context) return;
    s_lastHookedContext.store(context, std::memory_order_relaxed);
    TryInstallOwnerHook();
    Microsoft::WRL::ComPtr<ID3D11Device> device;
    context->GetDevice(device.GetAddressOf());
    HookContext(device.Get(), context);
}

void Install(ID3D11Device* device, ID3D11DeviceContext* context) {
    TryInstallOwnerHook();
    if (!s_armed.exchange(true)) {
        ParseTintColour();
        InstallExportHooks();
    }
    HookContext(device, context);
}

void SetVariantIndex(int variantIndex) {
    s_localVariantIndex.store(variantIndex);
    if (variantIndex < 0 || variantIndex >= static_cast<int>(kPrimaryFlames.size())) {
        return;
    }
    const auto& entry = kPrimaryFlames[static_cast<size_t>(variantIndex)];
    s_colour[0] = entry.r;
    s_colour[1] = entry.g;
    s_colour[2] = entry.b;
    s_active = true;
    NOBLE_INFO("ParticleTint: runtime variant {} ({}) selected", variantIndex, entry.id);
}

int GetVariantIndex() {
    return s_localVariantIndex.load();
}

void RegisterBipedFlame(uint32_t bipedDatum, int variantIndex) {
    if (bipedDatum == 0xFFFFFFFFu) return;
    s_lastRegisterMs.store(GetTickCount64(), std::memory_order_relaxed);
    std::lock_guard<std::mutex> lock(s_bipedMutex);
    if ((variantIndex >= 0 && variantIndex < static_cast<int>(kPrimaryFlames.size())) || variantIndex == kStockFlame) {
        s_bipedVariants[bipedDatum] = variantIndex;
    } else {
        s_bipedVariants.erase(bipedDatum);
    }
    s_bipedVariantCount.store(static_cast<int>(s_bipedVariants.size()), std::memory_order_relaxed);
}

void UnregisterBiped(uint32_t bipedDatum) {
    std::lock_guard<std::mutex> lock(s_bipedMutex);
    s_bipedVariants.erase(bipedDatum);
    s_bipedVariantCount.store(static_cast<int>(s_bipedVariants.size()), std::memory_order_relaxed);
}

void NoteRenderedBiped(uint32_t bipedDatum) {
    if (bipedDatum != 0xFFFFFFFFu) {
        s_lastRenderedBiped.store(bipedDatum, std::memory_order_relaxed);
    }
}

int GetBipedFlameVariant(uint32_t bipedDatum) {
    if (bipedDatum == 0xFFFFFFFFu) return -1;
    if (s_bipedVariantCount.load(std::memory_order_relaxed) == 0) return -1;
    std::lock_guard<std::mutex> lock(s_bipedMutex);
    auto it = s_bipedVariants.find(bipedDatum);
    return (it != s_bipedVariants.end()) ? it->second : -1;
}

void Shutdown() {
    s_active = false;
    std::lock_guard<std::mutex> lock(s_deviceMutex);
    for (auto& pair : s_tintSrvs) {
        if (pair.second) pair.second->Release();
    }
    s_tintSrvs.clear();
    for (auto& pair : s_deviceVariantSrvs) {
        for (auto* srv : pair.second) {
            if (srv) srv->Release();
        }
    }
    s_deviceVariantSrvs.clear();
    std::lock_guard<std::mutex> bipedLock(s_bipedMutex);
    s_bipedVariants.clear();
    s_bipedVariantCount.store(0, std::memory_order_relaxed);
    s_lastHookedSwapChain.store(nullptr);
    s_lastHookedContext.store(nullptr);
}

} // namespace noble::ParticleTint
