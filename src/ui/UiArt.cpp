#include "UiArt.hpp"
#include "common/Logger.hpp"
#include "common/Settings.hpp"

#include <windows.h>
#include <d3d11.h>
#include <wincodec.h>

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <functional>
#include <map>
#include <mutex>
#include <random>
#include <unordered_map>

namespace noble {
namespace UiArt {

namespace fs = std::filesystem;

namespace {

std::mutex s_mutex;
ID3D11Device* s_device = nullptr;
fs::path s_mccRoot;
fs::path s_exeDir;
std::map<std::string, UiTexture> s_cache;         // key -> texture (empty entries remember misses)
std::vector<ID3D11ShaderResourceView*> s_views;   // released in Shutdown
std::mt19937 s_rng{std::random_device{}()};

// ---------------------------------------------------------------------------------------------
// MCC UI texture packs.
// <name>.perm.bin: records of 0x1E0 bytes, magic 0xCDBFA090; name (ASCII) at +0x44, u16 width and
// height at +0x74/+0x76, u64 data size at +0x88 and u64 offset into <name>.temp.bin at +0x90.
// Data is BC1 or BC3; which one follows from the size (8 or 16 bytes per 4x4 block).
// ---------------------------------------------------------------------------------------------
struct PackEntry {
    uint16_t width = 0, height = 0;
    uint64_t size = 0, offset = 0;
};
std::map<std::string, std::unordered_map<std::string, PackEntry>> s_packs;

fs::path TexturePackDir() { return s_mccRoot / "Data" / "UI" / "texturepacks"; }

const std::unordered_map<std::string, PackEntry>& PackIndex(const std::string& pack) {
    auto it = s_packs.find(pack);
    if (it != s_packs.end()) return it->second;
    auto& index = s_packs[pack];
    std::ifstream in(TexturePackDir() / (pack + ".perm.bin"), std::ios::binary);
    if (!in) {
        NOBLE_WARN("UiArt: MCC texture pack '{}' not found", pack);
        return index;
    }
    std::vector<uint8_t> data((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    for (size_t off = 0; off + 0x1E0 <= data.size(); off += 0x1E0) {
        const uint8_t* r = data.data() + off;
        uint32_t magic = 0;
        memcpy(&magic, r, 4);
        if (magic != 0xCDBFA090u) continue;
        char name[0x25] = {};
        memcpy(name, r + 0x44, 0x24);
        PackEntry e;
        memcpy(&e.width, r + 0x74, 2);
        memcpy(&e.height, r + 0x76, 2);
        memcpy(&e.size, r + 0x88, 8);
        memcpy(&e.offset, r + 0x90, 8);
        index.emplace(name, e);  // first entry wins (some names repeat)
    }
    NOBLE_INFO("UiArt: indexed {} images in MCC texture pack '{}'", index.size(), pack);
    return index;
}

DXGI_FORMAT BlockFormat(int w, int h, uint64_t size) {
    const uint64_t blocks = static_cast<uint64_t>((w + 3) / 4) * static_cast<uint64_t>((h + 3) / 4);
    if (size == blocks * 8) return DXGI_FORMAT_BC1_UNORM;
    if (size == blocks * 16) return DXGI_FORMAT_BC3_UNORM;
    if (size == static_cast<uint64_t>(w) * h * 4) return DXGI_FORMAT_R8G8B8A8_UNORM;
    return DXGI_FORMAT_UNKNOWN;
}

UiTexture CreateTexture(int w, int h, DXGI_FORMAT format, const void* pixels) {
    UiTexture out;
    if (!s_device || w <= 0 || h <= 0 || format == DXGI_FORMAT_UNKNOWN) return out;
    const bool bc = format != DXGI_FORMAT_R8G8B8A8_UNORM && format != DXGI_FORMAT_B8G8R8A8_UNORM;
    if (bc && ((w % 4) != 0 || (h % 4) != 0)) return out;  // D3D11 needs whole blocks at mip 0
    D3D11_TEXTURE2D_DESC desc{};
    desc.Width = static_cast<UINT>(w);
    desc.Height = static_cast<UINT>(h);
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = format;
    desc.SampleDesc.Count = 1;
    desc.Usage = D3D11_USAGE_IMMUTABLE;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    D3D11_SUBRESOURCE_DATA init{};
    init.pSysMem = pixels;
    init.SysMemPitch = bc ? static_cast<UINT>((w / 4) * (format == DXGI_FORMAT_BC1_UNORM ? 8 : 16))
                          : static_cast<UINT>(w * 4);
    ID3D11Texture2D* tex = nullptr;
    if (FAILED(s_device->CreateTexture2D(&desc, &init, &tex)) || !tex) return out;
    ID3D11ShaderResourceView* srv = nullptr;
    const HRESULT hr = s_device->CreateShaderResourceView(tex, nullptr, &srv);
    tex->Release();
    if (FAILED(hr) || !srv) return out;
    s_views.push_back(srv);
    out.id = reinterpret_cast<ImTextureID>(srv);
    out.width = w;
    out.height = h;
    return out;
}

UiTexture LoadPackImage(const std::string& pack, const std::string& name) {
    const auto& index = PackIndex(pack);
    auto it = index.find(name);
    if (it == index.end()) return {};
    const PackEntry& e = it->second;
    if (e.size == 0 || e.size > 64ull * 1024 * 1024) return {};
    std::ifstream in(TexturePackDir() / (pack + ".temp.bin"), std::ios::binary);
    if (!in) return {};
    std::vector<uint8_t> data(static_cast<size_t>(e.size));
    in.seekg(static_cast<std::streamoff>(e.offset));
    in.read(reinterpret_cast<char*>(data.data()), static_cast<std::streamsize>(data.size()));
    if (in.gcount() != static_cast<std::streamsize>(data.size())) return {};
    return CreateTexture(e.width, e.height, BlockFormat(e.width, e.height, e.size), data.data());
}

// ---------------------------------------------------------------------------------------------
// MCC's loading-screen pack (LoadingTexturePack.perm.bin) has no names: it is a run of entries
// {u32 0x5E73CDD7, u32 size, u32 size, u32 0, data}, each padded, and 1920x1080 BC1 for the
// full-screen pictures. Reach's campaign scenes are found by a CRC-32 of their first 4 KB, so a
// reordered pack still works (and a changed picture is simply not used).
// ---------------------------------------------------------------------------------------------
struct LoadingEntry {
    uint64_t offset = 0;   // of the pixel data
    uint32_t size = 0;
    uint32_t crc = 0;
};
std::vector<LoadingEntry> s_loadingEntries;
bool s_loadingScanned = false;

struct SceneFingerprint {
    const char* scenario;
    uint32_t crc;
};
// Campaign mission scenes in MCC's loading pack (Reach, MCC build of 2026).
const SceneFingerprint kCampaignScenes[] = {
    { "m05", 0x4BE90F58u }, { "m10", 0xB7F2B364u }, { "m20", 0x3BB563E1u }, { "m30", 0x057B9E68u },
    { "m35", 0x1CA892C6u }, { "m45", 0x700A3B1Bu }, { "m50", 0x35A69E1Eu }, { "m52", 0x5CF7A8FFu },
    { "m60", 0xB52EE1B3u }, { "m70", 0x82D5F573u }, { "m70_a", 0x83F6FEBCu }, { "m70_bonus", 0x49776DBEu },
};

uint32_t Crc32(const uint8_t* p, size_t n) {
    uint32_t c = 0xFFFFFFFFu;
    for (size_t i = 0; i < n; ++i) {
        c ^= p[i];
        for (int k = 0; k < 8; ++k) c = (c >> 1) ^ (0xEDB88320u & (0u - (c & 1u)));
    }
    return ~c;
}

void ScanLoadingPack() {
    if (s_loadingScanned) return;
    s_loadingScanned = true;
    const fs::path path = TexturePackDir() / "loadingtexturepack.perm.bin";
    std::ifstream in(path, std::ios::binary);
    if (!in) return;
    std::error_code ec;
    const uint64_t fileSize = fs::file_size(path, ec);
    if (ec) return;
    constexpr uint32_t kFullHd = 1920u * 1080u / 2u;  // BC1
    uint64_t pos = 0;
    std::vector<uint8_t> window(64 * 1024);
    std::vector<uint8_t> head(4096);
    while (pos + 16 <= fileSize && s_loadingEntries.size() < 4096) {
        uint32_t hdr[4] = {};
        in.seekg(static_cast<std::streamoff>(pos));
        in.read(reinterpret_cast<char*>(hdr), sizeof(hdr));
        if (!in) break;
        if (hdr[0] == 0x5E73CDD7u && hdr[1] == hdr[2] && hdr[1] > 0 && pos + 16 + hdr[1] <= fileSize) {
            LoadingEntry e;
            e.offset = pos + 16;
            e.size = hdr[1];
            if (e.size == kFullHd) {
                in.read(reinterpret_cast<char*>(head.data()), static_cast<std::streamsize>(head.size()));
                e.crc = Crc32(head.data(), head.size());
            }
            s_loadingEntries.push_back(e);
            pos += 16 + hdr[1];
        }
        // Find the next entry header (entries are padded; look ahead at most 64 KB at a time).
        in.clear();
        in.seekg(static_cast<std::streamoff>(pos));
        in.read(reinterpret_cast<char*>(window.data()), static_cast<std::streamsize>(window.size()));
        const std::streamsize got = in.gcount();
        if (got < 16) break;
        bool found = false;
        for (std::streamsize i = 0; i + 4 <= got; i += 4) {
            uint32_t m = 0;
            memcpy(&m, window.data() + i, 4);
            if (m == 0x5E73CDD7u) {
                pos += static_cast<uint64_t>(i);
                found = true;
                break;
            }
        }
        if (!found) pos += static_cast<uint64_t>(got) - 4;  // keep scanning (rare)
        in.clear();
    }
    NOBLE_INFO("UiArt: {} images in MCC's loading pack", s_loadingEntries.size());
}

UiTexture LoadLoadingScene(uint32_t crc) {
    ScanLoadingPack();
    for (const LoadingEntry& e : s_loadingEntries) {
        if (e.crc != crc) continue;
        std::ifstream in(TexturePackDir() / "loadingtexturepack.perm.bin", std::ios::binary);
        std::vector<uint8_t> data(e.size);
        in.seekg(static_cast<std::streamoff>(e.offset));
        in.read(reinterpret_cast<char*>(data.data()), static_cast<std::streamsize>(data.size()));
        if (in.gcount() != static_cast<std::streamsize>(data.size())) return {};
        return CreateTexture(1920, 1080, DXGI_FORMAT_BC1_UNORM, data.data());
    }
    return {};
}

// ---------------------------------------------------------------------------------------------
// Image files (player / Noble art) through WIC.
// ---------------------------------------------------------------------------------------------
UiTexture LoadImageFile(const fs::path& path) {
    const HRESULT coInit = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    UiTexture out;
    IWICImagingFactory* factory = nullptr;
    IWICBitmapDecoder* decoder = nullptr;
    IWICBitmapFrameDecode* frame = nullptr;
    IWICFormatConverter* converter = nullptr;
    if (SUCCEEDED(CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&factory))) &&
        SUCCEEDED(factory->CreateDecoderFromFilename(path.c_str(), nullptr, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &decoder)) &&
        SUCCEEDED(decoder->GetFrame(0, &frame)) &&
        SUCCEEDED(factory->CreateFormatConverter(&converter)) &&
        SUCCEEDED(converter->Initialize(frame, GUID_WICPixelFormat32bppRGBA, WICBitmapDitherTypeNone, nullptr, 0.0, WICBitmapPaletteTypeCustom))) {
        UINT w = 0, h = 0;
        converter->GetSize(&w, &h);
        if (w > 0 && h > 0 && w <= 8192 && h <= 8192) {
            std::vector<uint8_t> pixels(static_cast<size_t>(w) * h * 4);
            if (SUCCEEDED(converter->CopyPixels(nullptr, w * 4, static_cast<UINT>(pixels.size()), pixels.data()))) {
                out = CreateTexture(static_cast<int>(w), static_cast<int>(h), DXGI_FORMAT_R8G8B8A8_UNORM, pixels.data());
            }
        }
    }
    if (converter) converter->Release();
    if (frame) frame->Release();
    if (decoder) decoder->Release();
    if (factory) factory->Release();
    if (SUCCEEDED(coInit)) CoUninitialize();
    if (!out) NOBLE_WARN("UiArt: could not load image '{}'", path.string());
    return out;
}

bool IsImageFile(const fs::path& p) {
    std::wstring ext = p.extension().wstring();
    std::transform(ext.begin(), ext.end(), ext.begin(), ::towlower);
    return ext == L".png" || ext == L".jpg" || ext == L".jpeg" || ext == L".bmp";
}

std::vector<fs::path> ImagesIn(const fs::path& dir) {
    std::vector<fs::path> out;
    std::error_code ec;
    for (const auto& e : fs::directory_iterator(dir, ec)) {
        if (e.is_regular_file(ec) && IsImageFile(e.path())) out.push_back(e.path());
    }
    std::sort(out.begin(), out.end());
    return out;
}

// First existing <dir>/<stem>.(png|jpg|jpeg|bmp).
fs::path FindImage(const fs::path& dir, const std::string& stem) {
    std::error_code ec;
    for (const char* ext : { ".png", ".jpg", ".jpeg", ".bmp" }) {
        const fs::path p = dir / (stem + ext);
        if (fs::exists(p, ec)) return p;
    }
    return {};
}

fs::path CustomDir() { return s_exeDir / "data" / "ui" / "custom"; }
fs::path NobleDir() { return s_exeDir / "data" / "ui" / "noble"; }

UiTexture Cached(const std::string& key, const std::function<UiTexture()>& load) {
    auto it = s_cache.find(key);
    if (it != s_cache.end()) return it->second;
    UiTexture t = load();
    s_cache[key] = t;
    return t;
}

UiTexture FileCached(const fs::path& p) {
    return Cached("file:" + p.string(), [&] { return LoadImageFile(p); });
}

UiTexture SceneCached(const std::string& scenario) {
    for (const SceneFingerprint& s : kCampaignScenes) {
        if (scenario == s.scenario) {
            return Cached("scene:" + scenario, [&] { return LoadLoadingScene(s.crc); });
        }
    }
    return {};
}

struct PreviewName {
    const char* scenario;
    const char* smallName;
    const char* largeName;
};
const PreviewName kPreviews[] = {
    { "50_panopticon", "HREACHMP_BOARDWALK", "HREACHMP_BOARDWALKLARGE" },
    { "70_boneyard", "HREACHMP_BONEYARD", "HREACHMP_BONEYARDLARGE" },
    { "45_launch_station", "HREACHMP_COUNTDOWN", "HREACHMP_COUNTDOWNLARGE" },
    { "30_settlement", "HREACHMP_POWERHOUSE", "HREACHMP_POWERHOUSELARGE" },
    { "52_ivory_tower", "HREACHMP_REFLECTION", "HREACHMP_REFLECTIONLARGE" },
    { "35_island", "HREACHMP_SPIRE", "HREACHMP_SPIRELARGE" },
    { "20_sword_slayer", "HREACHMP_SWORDBASE", "HREACHMP_SWORDBASELARGE" },
    { "45_aftship", "HREACHMP_ZEALOT", "HREACHMP_ZEALOTLARGE" },
    { "dlc_slayer", "HREACHMP_ANCHOR9", "HREACHMP_ANCHOR9LARGE" },
    { "dlc_invasion", "HREACHMP_BREAKPOINT", "HREACHMP_BREAKPOINTLARGE" },
    { "dlc_medium", "HREACHMP_TEMPEST", "HREACHMP_TEMPESTLARGE" },
    { "condemned", "HREACHMP_CONDEMNED", "HREACHMP_CONDEMNEDLARGE" },
    { "trainingpreserve", "HREACHMP_HIGHLANDS", "HREACHMP_HIGHLANDSLARGE" },
    { "cex_beaver_creek", "HREACHMP_BATTLECANYON", "HREACHMP_BATTLECANYONLARGE" },
    { "cex_damnation", "HREACHMP_PENANCE", "HREACHMP_PENANCELARGE" },
    { "cex_timberland", "HREACHMP_RIDGELINE", "HREACHMP_RIDGELINELARGE" },
    { "cex_prisoner", "HREACHMP_SOLITARY", "HREACHMP_SOLITARYLARGE" },
    { "cex_hangemhigh", "HREACHMP_HIGHNOON", "HREACHMP_HIGHNOONLARGE" },
    { "cex_headlong", "HREACHMP_BREAKNECK", "HREACHMP_BREAKNECKLARGE" },
    { "forge_halo", "HREACHMP_FORGEWORLD", "HREACHMP_FORGEWORLDLARGE" },
    { "m05", "HREACH_C01_NOBLEACTUAL", "HREACH_C01_NOBLEACTUAL_LARGE" },
    { "m10", "HREACH_C02_WINTERCONTINGENCY", "HREACH_C02_WINTERCONTINGENCY_LARGE" },
    { "m20", "HREACH_C03_ONISWORDBASE", "HREACH_C03_ONISWORDBASE_LARGE" },
    { "m30", "HREACH_C04_NIGHTFALL", "HREACH_C04_NIGHTFALL_LARGE" },
    { "m35", "HREACH_C05_TIPOFTHESPEAR", "HREACH_C05_TIPOFTHESPEAR_LARGE" },
    { "m45", "HREACH_C06_LONGNIGHTOFSOLACE", "HREACH_C06_LONGNIGHTOFSOLACE_LARGE" },
    { "m50", "HREACH_C07_EXODUS", "HREACH_C07_EXODUS_LARGE" },
    { "m52", "HREACH_C08_NEWALEXANDRIA", "HREACH_C08_NEWALEXANDRIA_LARGE" },
    { "m60", "HREACH_C09_THEPACKAGE", "HREACH_C09_THEPACKAGE_LARGE" },
    { "m70", "HREACH_C10_THEPILLAROFAUTUMN", "HREACH_C10_THEPILLAROFAUTUMN_LARGE" },
    { "m70_a", "HREACH_C10A_EPILOGUE", "HREACH_C10A_EPILOGUE_LARGE" },
    { "m70_bonus", "HREACH_C11_LONEWOLF", "HREACH_C11_LONEWOLF_LARGE" },
    { "ff50_park", "REACHFIREFIGHT_BEACHHEAD", "REACHFIREFIGHT_BEACHHEADLARGE" },
    { "ff45_corvette", "REACHFIREFIGHT_CORVETTE", "REACHFIREFIGHT_CORVETTELARGE" },
    { "ff20_courtyard", "REACHFIREFIGHT_COURTYARD", "REACHFIREFIGHT_COURTYARDLARGE" },
    { "ff60_icecave", "REACHFIREFIGHT_GLACIER", "REACHFIREFIGHT_GLACIERLARGE" },
    { "ff70_holdout", "REACHFIREFIGHT_HOLDOUT", "REACHFIREFIGHT_HOLDOUTLARGE" },
    { "ff60_airview", "REACHFIREFIGHT_OUTPOST", "REACHFIREFIGHT_OUTPOSTLARGE" },
    { "ff10_prototype", "REACHFIREFIGHT_OVERLOOK", "REACHFIREFIGHT_OVERLOOKLARGE" },
    { "ff30_waterfront", "REACHFIREFIGHT_WATERFRONT", "REACHFIREFIGHT_WATERFRONTLARGE" },
    { "ff_unearthed", "REACHFIREFIGHT_UNEARTHED", "REACHFIREFIGHT_UNEARTHEDLARGE" },
    { "cex_ff_halo", "REACHFIREFIGHT_INSTALLATION04", "REACHFIREFIGHT_INSTALLATION04LARGE" },
};

UiTexture s_menuBackground;
bool s_menuBackgroundChosen = false;

} // namespace

void Init(ID3D11Device* device, const fs::path& mccRoot, const fs::path& exeDir) {
    std::lock_guard<std::mutex> lock(s_mutex);
    s_device = device;
    if (s_device) s_device->AddRef();
    s_mccRoot = mccRoot;
    s_exeDir = exeDir;
    std::error_code ec;
    fs::create_directories(CustomDir() / "backgrounds", ec);
    fs::create_directories(CustomDir() / "loading", ec);
    NOBLE_INFO("UiArt: MCC '{}', custom art in '{}'", mccRoot.string(), CustomDir().string());
}

void Shutdown() {
    std::lock_guard<std::mutex> lock(s_mutex);
    for (ID3D11ShaderResourceView* v : s_views) v->Release();
    s_views.clear();
    s_cache.clear();
    s_packs.clear();
    s_menuBackground = {};
    s_menuBackgroundChosen = false;
    if (s_device) s_device->Release();
    s_device = nullptr;
}

UiTexture PackImage(const std::string& pack, const std::string& name) {
    std::lock_guard<std::mutex> lock(s_mutex);
    return Cached("pack:" + pack + ":" + name, [&] { return LoadPackImage(pack, name); });
}

UiTexture MenuBackground() {
    std::lock_guard<std::mutex> lock(s_mutex);
    if (s_menuBackgroundChosen) return s_menuBackground;
    s_menuBackgroundChosen = true;

    // 1. The player's own backgrounds always win.
    std::vector<fs::path> custom = ImagesIn(CustomDir() / "backgrounds");
    if (!custom.empty()) {
        s_menuBackground = FileCached(custom[std::uniform_int_distribution<size_t>(0, custom.size() - 1)(s_rng)]);
        if (s_menuBackground) return s_menuBackground;
    }
    // 2./3. Noble's own art and/or Reach's campaign scenes, as the setting asks.
    const std::string mode = Settings::Get().Snapshot().menuBackground;
    std::vector<fs::path> noble = ImagesIn(NobleDir() / "backgrounds");
    std::vector<std::string> pool;  // "file:<path>" or "scene:<scenario>"
    if (mode != "reach") for (const auto& p : noble) pool.push_back("file:" + p.string());
    if (mode != "noble" || pool.empty()) {
        for (const SceneFingerprint& s : kCampaignScenes) {
            if (std::string(s.scenario) == "m70_a") continue;  // the epilogue is mostly black space
            pool.push_back(std::string("scene:") + s.scenario);
        }
    }
    std::shuffle(pool.begin(), pool.end(), s_rng);
    for (const std::string& key : pool) {
        UiTexture t = key.rfind("file:", 0) == 0 ? FileCached(fs::path(key.substr(5))) : SceneCached(key.substr(6));
        if (t) {
            s_menuBackground = t;
            NOBLE_INFO("UiArt: menu background {}", key);
            break;
        }
    }
    return s_menuBackground;
}

UiTexture LoadingBackground(const std::string& scenario) {
    std::lock_guard<std::mutex> lock(s_mutex);
    return Cached("loading:" + scenario, [&]() -> UiTexture {
    for (const fs::path& dir : { CustomDir() / "loading", NobleDir() / "loading" }) {
        fs::path p = FindImage(dir, scenario);
        if (p.empty()) p = FindImage(dir, "default");
        if (!p.empty()) {
            UiTexture t = FileCached(p);
            if (t) return t;
        }
    }
    if (UiTexture t = SceneCached(scenario)) return t;
    // Multiplayer / Firefight: no per-map scene is known, so use a Reach campaign scene; the loading
    // screen adds the map's preview on top.
    const SceneFingerprint& pick = kCampaignScenes[std::hash<std::string>{}(scenario) % 10 + 1];
    return SceneCached(pick.scenario);
    });
}

UiTexture MapPreview(const std::string& scenario, bool large) {
    for (const PreviewName& p : kPreviews) {
        if (scenario == p.scenario) return PackImage("levelstexturepack", large ? p.largeName : p.smallName);
    }
    return {};
}

UiTexture MapPhoto(const std::string& scenario) {
    if (scenario.empty()) return {};
    if (UiTexture preview = MapPreview(scenario, true)) return preview;
    // A mod (or the player) can supply a picture for a map MCC does not know: the same file name the
    // loading screen uses. No scene fallback here - that would show a different map.
    std::lock_guard<std::mutex> lock(s_mutex);
    return Cached("mapphoto:" + scenario, [&]() -> UiTexture {
        for (const fs::path& dir : { CustomDir() / "loading", NobleDir() / "loading" }) {
            const fs::path p = FindImage(dir, scenario);
            if (!p.empty()) {
                UiTexture t = FileCached(p);
                if (t) return t;
            }
        }
        return {};
    });
}

namespace {
fs::path ReachArtFile(const std::string& category, const std::string& stem, bool prefix) {
    std::error_code ec;
    for (const fs::path& base : { CustomDir() / "reach", s_exeDir / "data" / "ui" / "reach" }) {
        const fs::path dir = base / category;
        if (!prefix) {
            const fs::path p = FindImage(dir, stem);
            if (!p.empty()) return p;
            continue;
        }
        if (!fs::is_directory(dir, ec)) continue;
        for (const fs::path& p : ImagesIn(dir)) {
            if (p.stem().string().rfind(stem, 0) == 0) return p;
        }
    }
    return {};
}
UiTexture ReachArtLocked(const std::string& category, const std::string& stem) {
    return Cached("reach:" + category + "/" + stem, [&] {
        const fs::path p = ReachArtFile(category, stem, false);
        return p.empty() ? UiTexture{} : LoadImageFile(p);
    });
}
} // namespace

UiTexture ReachArt(const std::string& category, const std::string& stem) {
    std::lock_guard<std::mutex> lock(s_mutex);
    return ReachArtLocked(category, stem);
}

UiTexture RankIcon(int rankIndex) {
    if (rankIndex < 0 || rankIndex > 99) return {};
    char stem[8];
    std::snprintf(stem, sizeof(stem), "%02d", rankIndex);
    const std::string key = stem;
    std::lock_guard<std::mutex> lock(s_mutex);
    return Cached("rank:" + key, [&] {
        const fs::path p = ReachArtFile("ranks", key + "_", true);
        return p.empty() ? UiTexture{} : LoadImageFile(p);
    });
}

UiTexture DifficultyIcon(int difficulty) {
    static const char* const kNames[4] = { "easy", "normal", "heroic", "legendary" };
    if (difficulty < 0 || difficulty > 3) return {};
    std::lock_guard<std::mutex> lock(s_mutex);
    return ReachArtLocked("difficulty", kNames[difficulty]);
}

void DrawContain(ImDrawList* dl, const UiTexture& tex, ImVec2 min, ImVec2 max, ImU32 tint) {
    if (!tex || !dl) return;
    const float w = max.x - min.x, h = max.y - min.y;
    if (w <= 0 || h <= 0) return;
    const float scale = std::min(w / static_cast<float>(tex.width), h / static_cast<float>(tex.height));
    const float dw = tex.width * scale, dh = tex.height * scale;
    const ImVec2 p0(min.x + (w - dw) * 0.5f, min.y + (h - dh) * 0.5f);
    dl->AddImage(tex.id, p0, ImVec2(p0.x + dw, p0.y + dh), ImVec2(0, 0), ImVec2(1, 1), tint);
}

UiTexture SkullIcon(const std::string& iconName) {
    return PackImage("skullstexturepack", iconName);
}

UiTexture NobleControllerImage() {
    std::lock_guard<std::mutex> lock(s_mutex);
    return Cached("noble:controller", [&]() -> UiTexture {
        for (const fs::path& dir : { CustomDir(), NobleDir() }) {
            const fs::path p = FindImage(dir, "controller");
            if (!p.empty()) {
                if (UiTexture t = FileCached(p)) return t;
            }
        }
        return {};
    });
}

UiTexture ControllerImage() {
    UiTexture t = PackImage("controllertexturepack", "LARGECONTROLLERFRONT");
    if (!t) t = PackImage("globaluitexturepack", "LARGECONTROLLERTOP");
    return t;
}

void DrawCover(ImDrawList* dl, const UiTexture& tex, ImVec2 min, ImVec2 max, ImU32 tint) {
    if (!tex || !dl) return;
    const float w = max.x - min.x, h = max.y - min.y;
    if (w <= 0 || h <= 0) return;
    const float texAspect = static_cast<float>(tex.width) / static_cast<float>(tex.height);
    const float boxAspect = w / h;
    ImVec2 uv0(0, 0), uv1(1, 1);
    if (texAspect > boxAspect) {  // texture wider: crop left/right
        const float keep = boxAspect / texAspect;
        uv0.x = (1.0f - keep) * 0.5f;
        uv1.x = 1.0f - uv0.x;
    } else {                      // texture taller: crop top/bottom
        const float keep = texAspect / boxAspect;
        uv0.y = (1.0f - keep) * 0.5f;
        uv1.y = 1.0f - uv0.y;
    }
    dl->AddImage(tex.id, min, max, uv0, uv1, tint);
}


static std::vector<NameplateInfo> s_nameplateCatalog;
static std::vector<EmblemInfo> s_emblemCatalog;
static bool s_customizationCatalogLoaded = false;

static std::string CleanXmlName(const std::string& raw, const std::string& prefix) {
    std::string s = raw;
    if (s.rfind(prefix, 0) == 0) s = s.substr(prefix.size());
    for (char& c : s) if (c == '_') c = ' ';
    bool capNext = true;
    for (char& c : s) {
        if (std::isspace(static_cast<unsigned char>(c))) {
            capNext = true;
        } else if (capNext) {
            c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
            capNext = false;
        } else {
            c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        }
    }
    return s;
}

static void EnsureCustomizationCatalogs() {
    if (s_customizationCatalogLoaded) return;
    s_customizationCatalogLoaded = true;

    // Try reading <mccRoot>/Data/UI/playercustomization.xml
    const fs::path xmlPath = s_mccRoot / "Data" / "UI" / "playercustomization.xml";
    std::ifstream file(xmlPath);
    if (file) {
        std::string line;
        auto getAttr = [](const std::string& l, const std::string& key) -> std::string {
            const std::string pat = key + "=\"";
            size_t p = l.find(pat);
            if (p == std::string::npos) return "";
            p += pat.size();
            size_t endP = l.find("\"", p);
            return (endP == std::string::npos) ? "" : l.substr(p, endP - p);
        };

        while (std::getline(file, line)) {
            if (line.find("<Nameplate ") != std::string::npos) {
                std::string idStr = getAttr(line, "id");
                std::string imgStr = getAttr(line, "image");
                std::string nameStr = getAttr(line, "name");
                if (!idStr.empty() && !imgStr.empty()) {
                    NameplateInfo info;
                    info.id = std::atoi(idStr.c_str());
                    info.image = imgStr;
                    info.name = nameStr.empty() ? ("Nameplate " + idStr) : CleanXmlName(nameStr, "$NAMEPLATE_");
                    s_nameplateCatalog.push_back(info);
                }
            } else if (line.find("<Emblem ") != std::string::npos) {
                std::string idStr = getAttr(line, "id");
                std::string imgStr = getAttr(line, "image1");
                std::string nameStr = getAttr(line, "name");
                if (!idStr.empty() && !imgStr.empty()) {
                    EmblemInfo info;
                    info.id = std::atoi(idStr.c_str());
                    info.image1 = imgStr;
                    info.name = nameStr.empty() ? ("Emblem " + idStr) : CleanXmlName(nameStr, "$EMBLEM_");
                    s_emblemCatalog.push_back(info);
                }
            }
        }
        NOBLE_INFO("UiArt: Loaded {} nameplates and {} emblems from playercustomization.xml",
                   s_nameplateCatalog.size(), s_emblemCatalog.size());
    }

    if (s_nameplateCatalog.empty()) {
        s_nameplateCatalog.push_back({ 1, "Default", "PlayerIDNamePlateBase" });
        for (int i = 2; i <= 50; ++i) {
            char buf[64];
            std::snprintf(buf, sizeof(buf), "PlayerIDNamePlate_%03d", i);
            s_nameplateCatalog.push_back({ i, "Nameplate " + std::to_string(i), buf });
        }
    }

    if (s_emblemCatalog.empty()) {
        static const char* kFallbackEmblems[] = {
            "e_002_p", "Radioactive", "e_003_p", "Jolly Roger", "e_004_p", "Valkyrie",
            "e_005_p", "Skull King",  "e_006_p", "Yin Yang",    "e_007_p", "Triad",
            "e_008_p", "Flaming Ninja", "e_009_p", "Spartan Swords", "e_010_p", "Helmet",
            "e_019_p", "Campfire",    "e_021_p", "Atomic",      "e_023_p", "Bulltrue",
            "e_049_p", "Spartan"
        };
        int id = 1;
        for (size_t i = 0; i < std::size(kFallbackEmblems); i += 2) {
            s_emblemCatalog.push_back({ id++, kFallbackEmblems[i + 1], kFallbackEmblems[i] });
        }
    }
}

const std::vector<NameplateInfo>& GetNameplateCatalog() {
    EnsureCustomizationCatalogs();
    return s_nameplateCatalog;
}

const std::vector<EmblemInfo>& GetEmblemCatalog() {
    EnsureCustomizationCatalogs();
    return s_emblemCatalog;
}

UiTexture Nameplate(int id) {
    EnsureCustomizationCatalogs();
    const auto& catalog = s_nameplateCatalog;
    std::string imageName = "PlayerIDNamePlateBase";
    for (const auto& item : catalog) {
        if (item.id == id) {
            imageName = item.image;
            break;
        }
    }

    const fs::path imgDir = s_mccRoot / "Data" / "UI" / "NamePlates" / "Images";
    fs::path target = imgDir / (imageName + ".png");
    std::error_code ec;
    if (!fs::exists(target, ec)) {
        char buf[64];
        std::snprintf(buf, sizeof(buf), "PlayerIDNamePlate_%03d.png", id);
        target = imgDir / buf;
    }
    if (!fs::exists(target, ec)) {
        target = imgDir / "PlayerIDNamePlateBase.png";
    }
    if (!fs::exists(target, ec)) return {};

    std::lock_guard<std::mutex> lock(s_mutex);
    return FileCached(target);
}

UiTexture Emblem(int id) {
    EnsureCustomizationCatalogs();
    for (const auto& item : s_emblemCatalog) {
        if (item.id == id) return EmblemByName(item.image1);
    }
    return EmblemByName("e_002_p");
}

UiTexture EmblemByName(const std::string& image1) {
    if (image1.empty()) return {};
    return PackImage("emblemstexturepack", image1);
}

} // namespace UiArt
} // namespace noble
