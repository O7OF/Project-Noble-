#include "BlamEngine.hpp"
#include "engine/ForgePlacement.hpp"
#include "EmbeddedBlocks.hpp"
#include "engine/VariantRules.hpp"
#include "ReachHost.hpp"
#include "network/NetworkTransport.hpp"
#include "network/MasterServerClient.hpp"
#include "ui/LoadScreen.hpp"
#include "ui/PauseMenu.hpp"
#include "career/Career.hpp"
#include "vfs/VFS.hpp"
#include "hooks/HookManager.hpp"
#include "hooks/LobbyPatches.hpp"
#include "hooks/EnginePatches.hpp"
#include "common/Logger.hpp"
#include <optional>
#include <algorithm>
#include <thread>
#include <chrono>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <cwchar>
#include <mutex>
#include <fstream>
#include <filesystem>
#include <objbase.h>
#include <shellapi.h>
#pragma comment(lib, "ole32.lib")

namespace noble {
static void SafeCallVariantInit(HMODULE hModule, void* targetBuffer) {
    if (!hModule || !targetBuffer) return;
    __try {
        typedef void (*InitDefaultVariantFn)(void* buffer);
        auto pInitVariant = reinterpret_cast<InitDefaultVariantFn>(
            reinterpret_cast<uintptr_t>(hModule) + 0x6BFF8);
        pInitVariant(targetBuffer);
    } __except(EXCEPTION_EXECUTE_HANDLER) {
    }
}

// Game variant decoder inside haloreach.dll (RVA 0x373A0, valid for the supported DLL build).
// Same call the reference host's set_game_variant makes: ok = decode(fileBytes, out[0xFC04], &status).
static bool SafeCallGameVariantDecode(HMODULE hModule, const void* fileData, void* outVariant, void* outStatus) {
    if (!hModule || !fileData || !outVariant || !outStatus) return false;
    __try {
        typedef char (*DecodeGameVariantFn)(const void* fileData, void* outVariant, void* outStatus);
        auto pDecode = reinterpret_cast<DecodeGameVariantFn>(
            reinterpret_cast<uintptr_t>(hModule) + 0x373A0);
        return pDecode(fileData, outVariant, outStatus) != 0;
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        return false;
    }
}

static bool SafeCallMapVariantParse(HMODULE hModule, const void* fileData, size_t fileSize, void* outVariant, char* outStatus) {
    if (!hModule || !fileData || !outVariant) return false;
    __try {
        typedef char (*ParseMapVariantFn)(const void* fileData, size_t fileSize, void* outVariant, char* outStatus);
        auto pParse = reinterpret_cast<ParseMapVariantFn>(
            reinterpret_cast<uintptr_t>(hModule) + 0x17AF00);
        return pParse(fileData, fileSize, outVariant, outStatus) != 0;
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        return false;
    }
}

BlamEngine::BlamEngine() = default;

BlamEngine::~BlamEngine() {
    Shutdown();
}

namespace {
using SerializeMapVariantFn = void (*)(void* outChunk, void* variant);
SerializeMapVariantFn s_origSerializeMapVariant = nullptr;
std::mutex s_capturedMutex;
std::vector<uint8_t> s_capturedChunk(0x7028, 0);
bool s_haveCapturedChunk = false;

// Candidates for the live variant struct, collected while a save call is in flight. The save routine reaches the
// variant either through a copy (mode 1 for the local one, mode 0 for the signed-in one) or through a getter.
constexpr int kMaxVariantCandidates = 6;
std::mutex s_candidateMutex;
void* s_variantCandidates[kMaxVariantCandidates] = {};
int s_variantCandidateCount = 0;
std::atomic<bool> s_armVariantCapture{ false };
std::atomic<void*> s_lastLiveVariantSource{ nullptr };
std::atomic<void*> s_lastCopyVariantSource{ nullptr };

void RecordVariantCandidate(void* pointer) {
    if (!pointer || !s_armVariantCapture.load(std::memory_order_relaxed)) return;
    std::lock_guard<std::mutex> lock(s_candidateMutex);
    for (int i = 0; i < s_variantCandidateCount; ++i) {
        if (s_variantCandidates[i] == pointer) return;
    }
    if (s_variantCandidateCount < kMaxVariantCandidates) s_variantCandidates[s_variantCandidateCount++] = pointer;
}

using CopyMapVariantFn = void (*)(void* dst, void* src, long long mode, void* extra);
CopyMapVariantFn s_origCopyMapVariant = nullptr;

void HookCopyMapVariant(void* dst, void* src, long long mode, void* extra) {
    if (s_origCopyMapVariant) s_origCopyMapVariant(dst, src, mode, extra);
    if (src && mode == 0) {
        s_lastCopyVariantSource.store(src, std::memory_order_release);
    }
    RecordVariantCandidate(src);
}

using GetVariantSourceFn = void* (*)();
GetVariantSourceFn s_origGetVariantSource = nullptr;

struct GetVariantSourceStubBlock {
    uint8_t code[192];
    void* originalFn;
};
GetVariantSourceStubBlock* s_getVariantSourceStub = nullptr;

void __fastcall RecordLiveVariantSource(void* source) {
    if (source) {
        s_lastLiveVariantSource.store(source, std::memory_order_release);
    }
    RecordVariantCandidate(source);
}

// 0x6BCEC is a tiny LTCG leaf called from ~103 sites; preserve r9/r10/r11 and XMM0..5
// across C++ atomics/mutex inside RecordLiveVariantSource.
void* EnsureGetVariantSourceStub() {
    if (s_getVariantSourceStub) return s_getVariantSourceStub->code;
    auto* blk = static_cast<GetVariantSourceStubBlock*>(
        VirtualAlloc(nullptr, sizeof(GetVariantSourceStubBlock), MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE));
    if (!blk) return nullptr;
    std::memset(blk, 0, sizeof(GetVariantSourceStubBlock));

    size_t p = 0;
    auto emit = [&](std::initializer_list<uint8_t> bytes) {
        for (uint8_t b : bytes) blk->code[p++] = b;
    };
    auto emitU32 = [&](uint32_t v) {
        std::memcpy(blk->code + p, &v, 4);
        p += 4;
    };
    auto emitU64 = [&](uint64_t v) {
        std::memcpy(blk->code + p, &v, 8);
        p += 8;
    };

    // Align rsp%16==0 for Win64 call into original getter
    emit({ 0x48, 0x83, 0xEC, 0x28 });       // sub rsp, 0x28
    emit({ 0xFF, 0x15 });                   // call qword ptr [rip + disp32]
    size_t dispPos = p;
    emitU32(0);
    int32_t disp = static_cast<int32_t>(offsetof(GetVariantSourceStubBlock, originalFn) - (dispPos + 4));
    std::memcpy(blk->code + dispPos, &disp, 4);

    // Save rax (`source`) + volatile r9/r10/r11 (4 qwords -> rsp%16==0) and XMM0..5
    emit({ 0x50 });                         // push rax
    emit({ 0x41, 0x51, 0x41, 0x52, 0x41, 0x53 }); // push r9, r10, r11
    emit({ 0x48, 0x81, 0xEC, 0x80, 0x00, 0x00, 0x00 }); // sub rsp, 0x80
    emit({ 0xF3, 0x0F, 0x7F, 0x44, 0x24, 0x20 });
    emit({ 0xF3, 0x0F, 0x7F, 0x4C, 0x24, 0x30 });
    emit({ 0xF3, 0x0F, 0x7F, 0x54, 0x24, 0x40 });
    emit({ 0xF3, 0x0F, 0x7F, 0x5C, 0x24, 0x50 });
    emit({ 0xF3, 0x0F, 0x7F, 0x64, 0x24, 0x60 });
    emit({ 0xF3, 0x0F, 0x7F, 0x6C, 0x24, 0x70 });
    emit({ 0x48, 0x89, 0xC1 });             // mov rcx, rax
    emit({ 0x48, 0xB8 });
    emitU64(reinterpret_cast<uint64_t>(&RecordLiveVariantSource));
    emit({ 0xFF, 0xD0 });                   // call rax
    emit({ 0xF3, 0x0F, 0x6F, 0x44, 0x24, 0x20 });
    emit({ 0xF3, 0x0F, 0x6F, 0x4C, 0x24, 0x30 });
    emit({ 0xF3, 0x0F, 0x6F, 0x54, 0x24, 0x40 });
    emit({ 0xF3, 0x0F, 0x6F, 0x5C, 0x24, 0x50 });
    emit({ 0xF3, 0x0F, 0x6F, 0x64, 0x24, 0x60 });
    emit({ 0xF3, 0x0F, 0x6F, 0x6C, 0x24, 0x70 });
    emit({ 0x48, 0x81, 0xC4, 0x80, 0x00, 0x00, 0x00 }); // add rsp, 0x80
    emit({ 0x41, 0x5B, 0x41, 0x5A, 0x41, 0x59 }); // pop r11, r10, r9
    emit({ 0x58 });                         // pop rax
    emit({ 0x48, 0x83, 0xC4, 0x28 });       // add rsp, 0x28
    emit({ 0xC3 });                         // ret

    FlushInstructionCache(GetCurrentProcess(), blk, sizeof(GetVariantSourceStubBlock));
    s_getVariantSourceStub = blk;
    return blk->code;
}

void HookSerializeMapVariant(void* outChunk, void* variant) {
    if (s_origSerializeMapVariant) s_origSerializeMapVariant(outChunk, variant);
    if (!outChunk) return;
    std::lock_guard<std::mutex> lock(s_capturedMutex);
    std::memcpy(s_capturedChunk.data(), outChunk, s_capturedChunk.size());
    s_haveCapturedChunk = true;
}
} // namespace

bool BlamEngine::InstallMapVariantCapture() {
    if (s_origSerializeMapVariant || !m_hModule) return s_origSerializeMapVariant != nullptr;
    void* target = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(m_hModule) + 0x17AC38);
    if (!HookManager::InstallHook(target, reinterpret_cast<void*>(&HookSerializeMapVariant),
                                  reinterpret_cast<void**>(&s_origSerializeMapVariant), "capture_mvar_serialize")) {
        NOBLE_WARN("BlamEngine: could not hook the map-variant serializer");
        return false;
    }
    NOBLE_INFO("BlamEngine: map-variant serializer hooked (RVA 0x17AC38)");

    void* copyTarget = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(m_hModule) + 0x6C290);
    if (!HookManager::InstallHook(copyTarget, reinterpret_cast<void*>(&HookCopyMapVariant),
                                  reinterpret_cast<void**>(&s_origCopyMapVariant), "capture_mvar_copy")) {
        NOBLE_WARN("BlamEngine: could not hook the map-variant copy");
    } else {
        NOBLE_INFO("BlamEngine: map-variant copy hooked (RVA 0x6C290)");
    }

    void* getterTarget = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(m_hModule) + 0x6BCEC);
    void* getterDetour = EnsureGetVariantSourceStub();
    if (!getterDetour || !HookManager::InstallHook(getterTarget, getterDetour,
                                  &s_getVariantSourceStub->originalFn, "capture_mvar_source")) {
        NOBLE_WARN("BlamEngine: could not hook the map-variant source getter");
    } else {
        s_origGetVariantSource = reinterpret_cast<GetVariantSourceFn>(s_getVariantSourceStub->originalFn);
        NOBLE_INFO("BlamEngine: map-variant source getter hooked (RVA 0x6BCEC)");
    }
    return true;
}

namespace {
// The module's TLS index, from its TLS directory. The engine's per-thread state pointer lives at [tls block + 0x48];
// the live map variant is 0x110F0 into that state.
bool ReadModuleTlsIndex(HMODULE module, DWORD& index) {
    if (!module) return false;
    const auto* dos = reinterpret_cast<const IMAGE_DOS_HEADER*>(module);
    if (dos->e_magic != IMAGE_DOS_SIGNATURE) return false;
    const auto* nt = reinterpret_cast<const IMAGE_NT_HEADERS*>(reinterpret_cast<const uint8_t*>(module) + dos->e_lfanew);
    if (nt->Signature != IMAGE_NT_SIGNATURE) return false;
    const IMAGE_DATA_DIRECTORY& dir = nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_TLS];
    if (!dir.VirtualAddress) return false;
    const auto* tls = reinterpret_cast<const IMAGE_TLS_DIRECTORY*>(reinterpret_cast<const uint8_t*>(module) + dir.VirtualAddress);
    const uintptr_t imageBase = reinterpret_cast<uintptr_t>(module);
    if (tls->AddressOfIndex < imageBase) return false;
    index = *reinterpret_cast<const DWORD*>(tls->AddressOfIndex);
    return true;
}

// SEH reads live in their own object-free helpers: MSVC cannot mix __try with C++ objects that need unwinding.
bool ReadTlsBlock(DWORD index, void*& tlsBlock) {
    __try {
        const uintptr_t teb = __readgsqword(0x30);
        auto** slots = *reinterpret_cast<void***>(teb + 0x58);
        tlsBlock = slots ? slots[index] : nullptr;
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        return false;
    }
    return true;
}

bool ReadEngineState(void* tlsBlock, void*& state) {
    __try {
        state = *reinterpret_cast<void**>(static_cast<uint8_t*>(tlsBlock) + 0x48);
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        return false;
    }
    return true;
}

bool CallMapVariantSaveTask(HMODULE module) {
    __try {
        typedef void (*SaveTaskFn)(void);
        auto pTask = reinterpret_cast<SaveTaskFn>(reinterpret_cast<uintptr_t>(module) + 0x156788);
        pTask();
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        return false;
    }
    return true;
}

bool CallMapVariantSave(HMODULE module) {
    __try {
        typedef void (*SaveMapVariantFn)(void);
        auto pSave = reinterpret_cast<SaveMapVariantFn>(reinterpret_cast<uintptr_t>(module) + 0x156974);
        pSave();
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        return false;
    }
    return true;
}

void* CallGetVariantSource(HMODULE module) {
    void* source = nullptr;
    __try {
        typedef void* (*GetSourceFn)();
        auto pGet = reinterpret_cast<GetSourceFn>(reinterpret_cast<uintptr_t>(module) + 0x6BCEC);
        source = pGet();
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        source = nullptr;
    }
    return source;
}

bool CallVariantInit(HMODULE module, void* variant) {
    __try {
        typedef void (*InitFn)(void*);
        auto pInit = reinterpret_cast<InitFn>(reinterpret_cast<uintptr_t>(module) + 0x6BFF8);
        pInit(variant);
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        return false;
    }
    return true;
}

bool CallVariantCopy(HMODULE module, void* dst, void* src, long long mode) {
    __try {
        typedef void (*CopyFn)(void*, void*, long long, void*);
        auto pCopy = reinterpret_cast<CopyFn>(reinterpret_cast<uintptr_t>(module) + 0x6C290);
        pCopy(dst, src, mode, nullptr);
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        return false;
    }
    return true;
}

bool CopyMemorySafe(void* dst, const void* src, size_t size) {
    __try {
        std::memcpy(dst, src, size);
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        return false;
    }
    return true;
}

bool CallMapVariantSerialize(HMODULE module, void* out, void* variant) {
    __try {
        typedef void (*SerializeFn)(void*, void*);
        auto pSerialize = reinterpret_cast<SerializeFn>(reinterpret_cast<uintptr_t>(module) + 0x17AC38);
        pSerialize(out, variant);
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        return false;
    }
    return true;
}
} // namespace

bool BlamEngine::TrySerializeLiveMapVariant(std::vector<uint8_t>& outChunk) {
    if (!m_hModule) return false;

    const auto be32 = [](const uint8_t* p) {
        return (static_cast<uint32_t>(p[0]) << 24) | (static_cast<uint32_t>(p[1]) << 16) |
               (static_cast<uint32_t>(p[2]) << 8) | p[3];
    };

    const auto trySerializeSource = [&](void* src, int copyFlags, const char* label) -> bool {
        if (!src) return false;
        std::vector<uint8_t> chunk(0x7028, 0);
        std::vector<uint8_t> committed(0x20000, 0);
        const bool initOk = CallVariantInit(m_hModule, committed.data());
        const bool copyOk = initOk && CallVariantCopy(m_hModule, committed.data(), src, copyFlags);
        uint32_t packed = 0;
        if (copyOk && CallMapVariantSerialize(m_hModule, chunk.data(), committed.data()) &&
            std::memcmp(chunk.data(), "mvar", 4) == 0 && be32(chunk.data() + 4) == 0x7028) {
            packed = be32(chunk.data() + 0x20);
        }
        if (packed == 0 || packed > 0x7000) {
            std::fill(chunk.begin(), chunk.end(), uint8_t{0});
            if (!CallMapVariantSerialize(m_hModule, chunk.data(), src) ||
                std::memcmp(chunk.data(), "mvar", 4) != 0 || be32(chunk.data() + 4) != 0x7028) {
                return false;
            }
            packed = be32(chunk.data() + 0x20);
        }
        if (packed == 0 || packed > 0x7000) return false;
        NOBLE_INFO("BlamEngine: serialized live map variant from {} at {:p} ({} packed bytes, copyFlags={}, init={}, copy={})",
                   label, src, packed, copyFlags, initOk, copyOk);
        outChunk = std::move(chunk);
        return true;
    };

    // Match haloreach.dll's MapVariantSave (0x156974): when a map is loaded in Forge mode,
    // CallGetVariantSource (0x6BCEC) returns tls[0x290]+0x308 (the live sandbox variant), which
    // must be copied into a fresh variant struct with copyFlags=0 before serializing.
    // TrySerializeLiveMapVariant usually runs on the DX11 Present thread where tls[0x290] is null,
    // so check the live source pointer captured from the simulation thread's HookGetVariantSource first.
    if (void* source = CallGetVariantSource(m_hModule)) {
        if (trySerializeSource(source, 0, "CallGetVariantSource")) return true;
    }
    if (void* source = s_lastLiveVariantSource.load(std::memory_order_acquire)) {
        if (trySerializeSource(source, 0, "HookGetVariantSource")) return true;
    }
    if (void* source = s_lastCopyVariantSource.load(std::memory_order_acquire)) {
        if (trySerializeSource(source, 0, "HookCopyMapVariant")) return true;
    }

    DWORD tlsIndex = 0;
    void* tlsBlock = nullptr;
    void* engineState = nullptr;
    if (ReadModuleTlsIndex(m_hModule, tlsIndex) && ReadTlsBlock(tlsIndex, tlsBlock) && tlsBlock &&
        ReadEngineState(tlsBlock, engineState) && engineState) {
        void* variant = static_cast<uint8_t*>(engineState) + 0x110F0;
        if (trySerializeSource(variant, 1, "engineState+0x110F0")) return true;
    }

    NOBLE_WARN("BlamEngine: failed to serialize live map variant");
    return false;
}

namespace {
std::mutex s_liveDumpMutex;
std::filesystem::path s_liveDumpDir;
int s_liveDumpSeconds = 0;
int s_liveDumpIndex = 0;
uint64_t s_liveDumpNextMs = 0;
} // namespace

void BlamEngine::ConfigureLiveDump(std::filesystem::path dir, int seconds) {
    std::lock_guard<std::mutex> lock(s_liveDumpMutex);
    s_liveDumpDir = std::move(dir);
    s_liveDumpSeconds = seconds;
    s_liveDumpIndex = 0;
    s_liveDumpNextMs = GetTickCount64() + static_cast<uint64_t>(seconds) * 1000;
    if (seconds > 0) NOBLE_INFO("BlamEngine: live variant dumps every {}s into '{}'", seconds, s_liveDumpDir.string());
}

void BlamEngine::TickLiveDump() {
    std::filesystem::path dir;
    int index = 0;
    {
        std::lock_guard<std::mutex> lock(s_liveDumpMutex);
        if (s_liveDumpSeconds <= 0) return;
        const uint64_t now = GetTickCount64();
        if (now < s_liveDumpNextMs) return;
        s_liveDumpNextMs = now + static_cast<uint64_t>(s_liveDumpSeconds) * 1000;
        dir = s_liveDumpDir;
        index = ++s_liveDumpIndex;
    }
    const bool ok = DumpLiveMapVariant(dir, index);
    NOBLE_WARN("BlamEngine: live dump {} -> {} ('{}')", index, ok ? 1 : 0, dir.string());
}

bool BlamEngine::DumpLiveMapVariant(const std::filesystem::path& dir, int index) {
    if (!m_hModule) return false;
    DWORD tlsIndex = 0;
    if (!ReadModuleTlsIndex(m_hModule, tlsIndex)) return false;
    void* tlsBlock = nullptr;
    if (!ReadTlsBlock(tlsIndex, tlsBlock) || !tlsBlock) return false;
    void* engineState = nullptr;
    if (!ReadEngineState(tlsBlock, engineState) || !engineState) return false;

    std::error_code ec;
    std::filesystem::create_directories(dir, ec);
    char name[64] = {};
    std::snprintf(name, sizeof(name), "live_%lu_%03d", static_cast<unsigned long>(GetCurrentProcessId()), index);
    const std::string stem = name;

    // A wide window around the state, so the editor's own structures are in the dump too.
    std::vector<uint8_t> state(0x60000, 0);
    if (!CopyMemorySafe(state.data(), engineState, state.size())) state.clear();
    if (!state.empty()) {
        std::ofstream(dir / (stem + "_state.bin"), std::ios::binary).write(
            reinterpret_cast<const char*>(state.data()), static_cast<std::streamsize>(state.size()));
    }

    void* variant = static_cast<uint8_t*>(engineState) + 0x110F0;
    std::vector<uint8_t> raw(0x20000, 0);
    if (!CopyMemorySafe(raw.data(), variant, raw.size())) raw.clear();
    if (!raw.empty()) {
        std::ofstream(dir / (stem + "_variant.bin"), std::ios::binary).write(
            reinterpret_cast<const char*>(raw.data()), static_cast<std::streamsize>(raw.size()));
    }

    std::vector<uint8_t> chunk(0x7028, 0);
    if (CallMapVariantSerialize(m_hModule, chunk.data(), variant)) {
        std::ofstream(dir / (stem + "_chunk.bin"), std::ios::binary).write(
            reinterpret_cast<const char*>(chunk.data()), static_cast<std::streamsize>(chunk.size()));
    }

    std::vector<uint8_t> committed(0x20000, 0);
    if (CallVariantInit(m_hModule, committed.data()) && CallVariantCopy(m_hModule, committed.data(), variant, 1)) {
        std::vector<uint8_t> copyChunk(0x7028, 0);
        if (CallMapVariantSerialize(m_hModule, copyChunk.data(), committed.data())) {
            std::ofstream(dir / (stem + "_copy_chunk.bin"), std::ios::binary).write(
                reinterpret_cast<const char*>(copyChunk.data()), static_cast<std::streamsize>(copyChunk.size()));
        }
        std::ofstream(dir / (stem + "_copy.bin"), std::ios::binary).write(
            reinterpret_cast<const char*>(committed.data()), static_cast<std::streamsize>(committed.size()));
    }
    return true;
}

bool BlamEngine::TakeCapturedMapVariantChunk(std::vector<uint8_t>& out) {
    std::lock_guard<std::mutex> lock(s_capturedMutex);
    if (!s_haveCapturedChunk) return false;
    out = s_capturedChunk;
    s_haveCapturedChunk = false;
    return true;
}

bool BlamEngine::TriggerMapVariantSave() {
    if (!m_hModule) return false;

    {
        std::lock_guard<std::mutex> lock(s_candidateMutex);
        s_variantCandidateCount = 0;
    }
    s_armVariantCapture.store(true, std::memory_order_relaxed);
    // Calling the task and then the routine made the routine fault (the task had already started it), so only the
    // routine is invoked here.
    const bool ok = CallMapVariantSave(m_hModule);
    s_armVariantCapture.store(false, std::memory_order_relaxed);

    void* candidates[kMaxVariantCandidates] = {};
    int count = 0;
    {
        std::lock_guard<std::mutex> lock(s_candidateMutex);
        count = s_variantCandidateCount;
        for (int i = 0; i < count; ++i) candidates[i] = s_variantCandidates[i];
    }
    NOBLE_INFO("BlamEngine: save routine {} and exposed {} variant candidate(s)", ok ? "returned" : "faulted", count);

    const auto be32 = [](const uint8_t* p) {
        return (static_cast<uint32_t>(p[0]) << 24) | (static_cast<uint32_t>(p[1]) << 16) |
               (static_cast<uint32_t>(p[2]) << 8) | p[3];
    };
    for (int i = 0; i < count; ++i) {
        void* candidate = candidates[i];
        if (!candidate) continue;
        std::vector<uint8_t> chunk(0x7028, 0);
        if (!CallMapVariantSerialize(m_hModule, chunk.data(), candidate)) continue;
        if (std::memcmp(chunk.data(), "mvar", 4) != 0 || be32(chunk.data() + 4) != 0x7028) continue;
        const uint32_t packed = be32(chunk.data() + 0x20);
        if (packed == 0 || packed > 0x7000) continue;
        NOBLE_INFO("BlamEngine: serialized variant candidate {:p} ({} packed bytes)", candidate, packed);
        std::lock_guard<std::mutex> lock(s_capturedMutex);
        s_capturedChunk = std::move(chunk);
        s_haveCapturedChunk = true;
        break;
    }
    return ok;
}

bool BlamEngine::Initialize(const ReachInstallInfo& installInfo, HMODULE hModule, const ReachExports& exports, const EngineHostContext& context) {
    m_hModule = hModule;
    InstallMapVariantCapture();
    InstallForgePlacementWatch(m_hModule);
    s_active = this;
    if (m_isInitialized) return true;

    m_hModule = hModule;
    m_exports = exports;
    m_context = context;

    NOBLE_INFO("Instantiating Blam! Reach Game Engine interface...");

    // 1. Pass Library Settings FIRST (as discovered in host-probe main.c lines 3439-3490)
    // haloreach.dll requires a 510-byte (0x1FE) structure with UTF-16 strings for paths
    if (m_exports.SetLibrarySettings) {
        NOBLE_INFO("Configuring haloreach.dll with SetLibrarySettings (3 x \"en-US\" locale strings)...");
        ReachLibrarySettings settings{};
        wcsncpy_s(settings.locale0, L"en-US", _TRUNCATE);
        wcsncpy_s(settings.locale1, L"en-US", _TRUNCATE);
        wcsncpy_s(settings.locale2, L"en-US", _TRUNCATE);

        m_exports.SetLibrarySettings(&settings);
        NOBLE_INFO("SetLibrarySettings initialized successfully.");
    }

    // 2. Create Game Engine Interface (CreateDataAccess removed per the engine-launch-flow spec)
    if (!m_exports.CreateGameEngine) {
        NOBLE_ERROR("CreateGameEngine export not available in haloreach.dll!");
        return false;
    }

    typedef int32_t (*CreateGameEngineCall)(void** outEngine);
    auto pCreateGameEngine = reinterpret_cast<CreateGameEngineCall>(m_exports.CreateGameEngine);

    int32_t status = pCreateGameEngine(&m_engineInstance);
    NOBLE_INFO("CreateGameEngine returned status: 0x{:X}, Instance: 0x{:X}", 
        status, reinterpret_cast<uintptr_t>(m_engineInstance));

    if (!m_engineInstance) {
        NOBLE_ERROR("Failed to obtain valid c_game_engine instance from haloreach.dll!");
        return false;
    }

    // Resolve virtual method table
    void** vtable = *reinterpret_cast<void***>(m_engineInstance);
    if (vtable) {
        m_fnShutdown       = reinterpret_cast<EngineShutdownFn>(vtable[0]);
        m_fnInitialize     = reinterpret_cast<EngineInitFn>(vtable[1]);
        m_fnStart          = reinterpret_cast<EngineStartFn>(vtable[2]);
        m_fnProcessEvent   = reinterpret_cast<EngineProcessEventFn>(vtable[3]); // vtbl+0x18
        m_fnExecuteCommand = reinterpret_cast<EngineExecuteCommandFn>(vtable[9]);

        NOBLE_INFO("c_game_engine VTable bound successfully:");
        NOBLE_INFO(" - vfunc[0] (Shutdown):       0x{:X}", reinterpret_cast<uintptr_t>(m_fnShutdown));
        NOBLE_INFO(" - vfunc[1] (Initialize):     0x{:X}", reinterpret_cast<uintptr_t>(m_fnInitialize));
        NOBLE_INFO(" - vfunc[2] (Start):          0x{:X}", reinterpret_cast<uintptr_t>(m_fnStart));
        NOBLE_INFO(" - vfunc[3] (ProcessEvent):   0x{:X} (vtbl+0x18)", reinterpret_cast<uintptr_t>(m_fnProcessEvent));
        NOBLE_INFO(" - vfunc[9] (ExecuteCommand): 0x{:X}", reinterpret_cast<uintptr_t>(m_fnExecuteCommand));
    }

    // 3. Init(engine, device, 0, swap_chain, 0)
    if (m_fnInitialize) {
        NOBLE_INFO("Calling c_game_engine Init(engine, device:0x{:X}, 0, swap_chain:0x{:X}, 0)...",
            reinterpret_cast<uintptr_t>(m_context.pDevice),
            reinterpret_cast<uintptr_t>(m_context.pSwapChain));
        int32_t initStatus = m_fnInitialize(m_engineInstance, m_context.pDevice, nullptr, m_context.pSwapChain, nullptr);
        NOBLE_INFO("c_game_engine Init returned: 0x{:X}", initStatus);
    }

    // Configure Host object paths and renderer devices
    ReachHost::Get().SetPaths(installInfo.reachDirectory, installInfo.mapsDirectory, ReachHost::Get().GetDataDir());
    ReachHost::Get().SetDeviceAndContext(m_context.pDevice, m_context.pContext);
    // Pool SafeGuards: removed. The null atom arrays they guarded against came from the swap chain
    // missing DXGI_USAGE_SHADER_INPUT (fixed in D3D11Renderer.cpp) and host slot 32 not filling
    // the display-settings buffer (fixed in ReachHost.cpp).
    NOBLE_INFO("BlamEngine: Backbuffer pool set up via swap chain SHADER_INPUT; SafeGuard hooks removed.");

    m_isInitialized = true;
    NOBLE_INFO("BlamEngine host interface emulation initialized and ready for single-pass map launch.");
    return true;
}

// Multiplayer map table: launch index (written at +0x10/+0x14/+0x1D5C0) and map id
// (written at +0x1D334/+0x1D5BC). Values match the engine build the reference host supports
// (its host-probe set_map table); the main menu uses launch index -2 with map id 1035.
struct ReachMultiplayerMap {
    const char* scenario;
    const char* displayName;
    uint32_t launchIndex;
    uint32_t mapId;
};

static const ReachMultiplayerMap kMultiplayerMaps[] = {
    { "50_panopticon",     "boardwalk",     190, 1035 },
    { "70_boneyard",       "boneyard",      191, 1080 },
    { "45_launch_station", "countdown",     192, 1020 },
    { "30_settlement",     "powerhouse",    193, 1055 },
    { "52_ivory_tower",    "reflection",    194, 1150 },
    { "35_island",         "spire",         195, 1200 },
    { "20_sword_slayer",   "sword base",    196, 1000 },
    { "45_aftship",        "zealot",        197, 1040 },
    { "dlc_slayer",        "anchor 9",      198, 2001 },
    { "dlc_invasion",      "breakpoint",    199, 2002 },
    { "dlc_medium",        "tempest",       200, 2004 },
    { "condemned",         "condemned",     201, 1500 },
    { "trainingpreserve",  "highlands",     202, 1510 },
    { "cex_beaver_creek",  "battle canyon", 203, 10020 },
    { "cex_damnation",     "penance",       204, 10010 },
    { "cex_timberland",    "ridgeline",     205, 10030 },
    { "cex_prisoner",      "solitary",      206, 10070 },
    { "cex_hangemhigh",    "high noon",     207, 10060 },
    { "cex_headlong",      "breakneck",     208, 10050 },
    { "forge_halo",        "forge world",   209, 3006 },
};

static constexpr uint32_t kMainMenuLaunchIndex = static_cast<uint32_t>(-2);
static constexpr uint32_t kMainMenuMapId = 1035; // menu backdrop map id in the engine's captured menu block

static uint32_t MapIdForLaunchIndex(uint32_t launchIndex) {
    if (launchIndex == kMainMenuLaunchIndex) return kMainMenuMapId;
    for (const auto& m : kMultiplayerMaps) {
        if (m.launchIndex == launchIndex) return m.mapId;
    }
    return 0;
}

// Resolve a requested game variant (absolute path, file name, or "game_variants\<file>")
// against the places Noble actually receives variants from.
static std::filesystem::path ResolveVariantFile(const std::string& requested) {
    if (requested.empty()) return {};
    std::error_code ec;
    const std::filesystem::path direct(requested);
    if (std::filesystem::is_regular_file(direct, ec)) return direct;

    const auto reachDir = ReachHost::Get().GetReachDir();
    const auto dataDir = ReachHost::Get().GetDataDir();
    const std::filesystem::path name = direct.filename();
    const std::filesystem::path candidates[] = {
        reachDir / "game_variants" / direct,
        reachDir / "game_variants" / name,
        dataDir / "game_variants" / direct,
        dataDir / "game_variants" / name,
        dataDir / "variants" / name,
        std::filesystem::current_path() / direct,
        std::filesystem::current_path() / name,
    };
    for (const auto& candidate : candidates) {
        if (std::filesystem::is_regular_file(candidate, ec)) return candidate;
    }
    return {};
}

// Some stock Reach variants are map-specific even though MCC also ships a generic file
// with the same game type. Invasion is the important case: 3nvasion_054.bin carries no
// compatibility with Boneyard/Spire/Breakpoint and the engine rejects it with
// "map and game incompatible" (EngineEvent reason 4). Substitute the matching stock
// map-specific Invasion variant instead of silently running Slayer.
static std::filesystem::path SelectMapSpecificVariant(const std::filesystem::path& requested, uint32_t mapId) {
    if (requested.empty()) return requested;
    const std::wstring stem = requested.stem().wstring();
    if (_wcsicmp(stem.c_str(), L"3nvasion_054") != 0) return requested;

    const wchar_t* replacement = nullptr;
    switch (mapId) {
        case 1080: replacement = L"3nvasion_boneyard_054.bin"; break; // Boneyard
        case 1200: replacement = L"3nvasion_spire_054.bin";    break; // Spire
        case 2002: replacement = L"3nvasion_dlc_054.bin";      break; // Breakpoint
        default: return requested;
    }

    const std::filesystem::path candidate = ReachHost::Get().GetReachDir() / "game_variants" / replacement;
    std::error_code ec;
    if (std::filesystem::is_regular_file(candidate, ec)) return candidate;
    return requested;
}

uint32_t BlamEngine::ResolveMapId(const std::string& mapName) {
    std::string lower = mapName;
    std::transform(lower.begin(), lower.end(), lower.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });

    // Main Menu (-2 / 0xFFFFFFFE in authentic Reach Blam! engine)
    if (lower.find("mainmenu") != std::string::npos || lower == "0" || lower == "menu" || lower == "-2") {
        return static_cast<uint32_t>(-2);
    }

    // Firefight Maps (210 - 219)
    if (lower.find("beachhead") != std::string::npos || lower.find("ff50_park") != std::string::npos || lower == "210") return 210;
    if (lower.find("corvette") != std::string::npos || lower.find("ff45_corvette") != std::string::npos || lower == "211") return 211;
    if (lower.find("courtyard") != std::string::npos || lower.find("ff20_courtyard") != std::string::npos || lower == "212") return 212;
    if (lower.find("glacier") != std::string::npos || lower.find("ff60_icecave") != std::string::npos || lower == "213") return 213;
    if (lower.find("holdout") != std::string::npos || lower.find("ff70_holdout") != std::string::npos || lower == "214") return 214;
    if (lower.find("outpost") != std::string::npos || lower.find("ff60_airview") != std::string::npos || lower.find("ff60_ruins") != std::string::npos || lower == "215") return 215;
    if (lower.find("overlook") != std::string::npos || lower.find("ff10_prototype") != std::string::npos || lower == "216") return 216;
    if (lower.find("waterfront") != std::string::npos || lower.find("ff30_waterfront") != std::string::npos || lower == "217") return 217;
    if (lower.find("unearthed") != std::string::npos || lower.find("ff_unearthed") != std::string::npos || lower == "218") return 218;
    if (lower.find("installation 04") != std::string::npos || lower.find("installation04") != std::string::npos || lower.find("cex_ff_halo") != std::string::npos || lower == "219") return 219;

    // Campaign Missions (178 - 189)
    if (lower.find("noble actual") != std::string::npos || lower.find("m05") != std::string::npos || lower == "178") return 178;
    if (lower.find("winter contingency") != std::string::npos || lower.find("m10") != std::string::npos || lower == "179") return 179;
    if (lower.find("oni: sword base") != std::string::npos || lower.find("oni sword base") != std::string::npos || lower.find("m20") != std::string::npos || lower == "180") return 180;
    if (lower.find("nightfall") != std::string::npos || lower.find("m30") != std::string::npos || lower == "181") return 181;
    if (lower.find("tip of the spear") != std::string::npos || lower.find("m35") != std::string::npos || lower == "182") return 182;
    if (lower.find("long night of solace") != std::string::npos || lower.find("m45") != std::string::npos || lower == "183") return 183;
    if (lower.find("exodus") != std::string::npos || lower.find("m50") != std::string::npos || lower == "184") return 184;
    if (lower.find("new alexandria") != std::string::npos || lower.find("m52") != std::string::npos || lower == "185") return 185;
    if (lower.find("the package") != std::string::npos || lower.find("m60") != std::string::npos || lower == "186") return 186;
    // The two extra m70 scenes must be tested BEFORE "m70": the substrings overlap ("m70_bonus" contains
    // "m70"), so Lone Wolf used to resolve to Pillar of Autumn's launch index.
    if (lower.find("lone wolf") != std::string::npos || lower.find("m70_bonus") != std::string::npos || lower == "189") return 189;
    if (lower.find("epilogue") != std::string::npos || lower.find("m70_a") != std::string::npos || lower == "188") return 188;
    if (lower.find("the pillar of autumn") != std::string::npos || lower.find("m70") != std::string::npos || lower == "187") return 187;

    // Multiplayer / Forge maps (launch index 190-209)
    for (const auto& m : kMultiplayerMaps) {
        if (lower.find(m.scenario) != std::string::npos || lower.find(m.displayName) != std::string::npos ||
            lower == std::to_string(m.launchIndex)) {
            return m.launchIndex;
        }
    }

    return 0;
}

std::string BlamEngine::ScenarioFromReachMapId(int32_t mapId, int mode) {
    switch (mapId) {
        case 5005: return "m05";
        case 5010: return "m10";
        case 5020: return "m20";
        case 5030: return "m30";
        case 5035: return "m35";
        case 5045: return "m45";
        case 5050: return "m50";
        case 5052: return "m52";
        case 5060: return "m60";
        case 5070: return "m70";
        case 5075: return "m70_a";
        case 5080: return "m70_bonus";
        case 1000: return "20_sword_slayer";
        case 1020: return "45_launch_station";
        case 1035: return "50_panopticon";
        case 1040: return "45_aftship";
        case 1055: return "30_settlement";
        case 1080: return "70_boneyard";
        case 1150: return "52_ivory_tower";
        case 1200: return "35_island";
        case 1500: return "condemned";
        case 1510: return "trainingpreserve";
        case 2001: return "dlc_slayer";
        case 2002: return "dlc_invasion";
        case 2004: return "dlc_medium";
        case 3006: return "forge_halo";
        case 10010: return "cex_damnation";
        case 10020: return "cex_beaver_creek";
        case 10030: return "cex_timberland";
        case 10050: return "cex_headlong";
        case 10060: return "cex_hangemhigh";
        case 10070: return "cex_prisoner";
        case 7000: return "ff10_prototype";
        case 7020: return "ff20_courtyard";
        case 7030: return "ff30_waterfront";
        case 7040: return "ff45_corvette";
        case 7060: return "ff50_park";
        case 7080: return "ff60_airview";
        case 7100: return "ff60_icecave";
        case 7110: return "ff70_holdout";
        case 7130: return "ff_unearthed";
        case 7500: return "cex_ff_halo";
        default:
            if (mode == 3 || mode == 1) return "m10";
            if (mode == 4 || mode == 1035) return "ff20_courtyard";
            return "20_sword_slayer";
    }
}

bool BlamEngine::InspectSavedFilm(const std::filesystem::path& filmPath, std::string& outScenario, std::string& outMode, uint32_t& outMapId) {
    std::ifstream in(filmPath, std::ios::binary);
    if (!in.is_open()) return false;

    uint8_t buf[0x0A40] = {};
    in.read(reinterpret_cast<char*>(buf), sizeof(buf));
    const std::streamsize got = in.gcount();
    if (got < 0x340) return false;
    if (std::memcmp(buf, "_blf", 4) != 0 || std::memcmp(buf + 0x30, "chdr", 4) != 0) return false;

    const uint8_t chdrMode = buf[0x006A];
    int32_t mapId = 0;
    std::memcpy(&mapId, buf + 0x006C, sizeof(int32_t));
    if (got >= 0x092C && std::memcmp(buf + 0x0340, "flmh", 4) == 0) {
        int32_t flmhMapId = 0;
        std::memcpy(&flmhMapId, buf + 0x0928, sizeof(int32_t));
        if (flmhMapId != 0) mapId = flmhMapId;
    }
    outMapId = static_cast<uint32_t>(mapId);

    std::string scenarioPath;
    if (got >= 0x0930 && std::memcmp(buf + 0x0340, "flmh", 4) == 0) {
        const char* pScen = reinterpret_cast<const char*>(buf + 0x092C);
        size_t len = 0;
        while (len < 255 && pScen[len] != '\0' && (static_cast<unsigned char>(pScen[len]) >= 32 && static_cast<unsigned char>(pScen[len]) < 127)) {
            ++len;
        }
        if (len > 0) {
            scenarioPath.assign(pScen, len);
        } else {
            for (size_t off = 0x0340; off + 16 < static_cast<size_t>(got); ++off) {
                if (std::memcmp(buf + off, "haloreach\\maps\\", 15) == 0 ||
                    std::memcmp(buf + off, "maps\\", 5) == 0 ||
                    std::memcmp(buf + off, "levels\\", 7) == 0) {
                    size_t slen = 0;
                    while (slen < 255 && buf[off + slen] != '\0' &&
                           (static_cast<unsigned char>(buf[off + slen]) >= 32 && static_cast<unsigned char>(buf[off + slen]) < 127)) {
                        ++slen;
                    }
                    if (slen > 0) {
                        scenarioPath.assign(reinterpret_cast<const char*>(buf + off), slen);
                        break;
                    }
                }
            }
        }
    }
    if (!scenarioPath.empty()) {
        const size_t slash = scenarioPath.find_last_of("\\/");
        outScenario = (slash != std::string::npos) ? scenarioPath.substr(slash + 1) : scenarioPath;
    }
    if (outScenario.size() > 4 && outScenario.substr(outScenario.size() - 4) == ".map") {
        outScenario.resize(outScenario.size() - 4);
    }
    bool validScen = !outScenario.empty();
    for (char c : outScenario) {
        if (!std::isalnum(static_cast<unsigned char>(c)) && c != '_' && c != '-') { validScen = false; break; }
    }
    if (!validScen) {
        outScenario = ScenarioFromReachMapId(mapId, chdrMode);
    }

    if (chdrMode == 3) {
        outMode = "campaign";
    } else if (chdrMode == 4) {
        outMode = "firefight";
    } else if (outScenario == "forge_halo") {
        outMode = "forge";
    } else {
        outMode = "multiplayer";
    }

    return true;
}

int BlamEngine::s_engineModeOverride = 0;
BlamEngine* BlamEngine::s_active = nullptr;
bool BlamEngine::s_guestPostStart = false;
std::string BlamEngine::s_savedFilmPath;
bool BlamEngine::s_isSavedFilmPlayback = false;
std::atomic<bool> BlamEngine::s_waitingForRemotePlayers{false};
std::atomic<int> BlamEngine::s_waitingRemoteJoined{0};
std::atomic<int> BlamEngine::s_waitingRemoteExpected{0};

bool BlamEngine::IsWaitingForRemotePlayers() {
  return s_waitingForRemotePlayers.load(std::memory_order_relaxed);
}
int BlamEngine::WaitingRemotePlayersJoined() {
  return s_waitingRemoteJoined.load(std::memory_order_relaxed);
}
int BlamEngine::WaitingRemotePlayersExpected() {
  return s_waitingRemoteExpected.load(std::memory_order_relaxed);
}
std::optional<VariantFields> BlamEngine::s_variantOverrides;
std::string BlamEngine::s_variantPatch;
std::atomic<bool> BlamEngine::s_gameTeamsEnabled{false};

static void ApplyVariantPatch(const std::string& spec, VariantFields& fields) {
    size_t start = 0;
    while (start < spec.size()) {
        const size_t comma = spec.find(',', start);
        const std::string item = spec.substr(start, comma == std::string::npos ? std::string::npos : comma - start);
        start = comma == std::string::npos ? spec.size() : comma + 1;
        const size_t eq = item.find('=');
        if (eq == std::string::npos) continue;
        const std::string key = item.substr(0, eq);
        const int value = std::atoi(item.c_str() + eq + 1);
        auto byte = [&](uint8_t& field, int lo, int hi) {
            field = static_cast<uint8_t>(std::clamp(value, lo, hi));
        };
        if (key == "time") byte(fields.timeLimit, 0, 255);
        else if (key == "rounds") byte(fields.roundsToWin, 0, 15);
        else if (key == "roundlimit") byte(fields.roundLimit, 0, 31);
        else if (key == "sudden") byte(fields.suddenDeath, 0, 127);
        else if (key == "grace") byte(fields.gracePeriod, 0, 31);
        else if (key == "respawn") byte(fields.respawnTime, 0, 255);
        else if (key == "lives") byte(fields.livesPerRound, 0, 63);
        else if (key == "teamLives") byte(fields.teamLivesPerRound, 0, 127);
        else if (key == "traits") byte(fields.traitsDuration, 0, 63);
        else if (key == "speed") byte(fields.speed, 0, 31);
        else if (key == "gravity") byte(fields.gravity, 0, 15);
        else if (key == "radar") byte(fields.radarState, 0, 7);
        else if (key == "primary") byte(fields.primaryWeapon, 0, 255);
        else if (key == "secondary") byte(fields.secondaryWeapon, 0, 255);
        else if (key == "ability") byte(fields.ability, 0, 255);
        else if (key == "damage") byte(fields.weaponDamage, 0, 15);
        else if (key == "health") byte(fields.healthMult, 0, 7);
        else if (key == "shield") byte(fields.shieldMult, 0, 7);
        else if (key == "infinite") byte(fields.infiniteAmmo, 0, 3);
        else if (key == "team") fields.miscFlags = static_cast<uint8_t>(value ? (fields.miscFlags | 1) : (fields.miscFlags & ~1));
        else if (key == "score") fields.scoreToWin = static_cast<uint16_t>(std::clamp(value, 0, 65535));
    }
}

std::vector<uint8_t> BlamEngine::BuildLaunchBlock(uint32_t mapId, uint32_t gameModeTag, int difficulty, uint64_t skulls,
                                                   const std::string& customMvarPath, const std::string& customVariantPath,
                                                   const std::string& connectAddress) {
    s_gameTeamsEnabled.store(false, std::memory_order_relaxed);
    constexpr size_t LAUNCH_BLOCK_SIZE = 0x2BF30; // 180,016 bytes
    std::vector<uint8_t> block(LAUNCH_BLOCK_SIZE, 0);

    // 1. Try loading pre-built launch block from data/blocks/ or data/
    bool loadedPrebuilt = false;
    std::filesystem::path blockPath;

    std::filesystem::path dataRoot = ReachHost::Get().GetDataDir();
    if (!std::filesystem::exists(dataRoot / "launch_block_1.bin")) {
        if (std::filesystem::exists(std::filesystem::path("data") / "launch_block_1.bin")) {
            dataRoot = "data";
        }
    }

    int clampedDiff = (difficulty >= 0 && difficulty <= 3) ? difficulty : 1;

    const bool isSavedFilm = IsSavedFilmPlayback() || !s_savedFilmPath.empty();
    if (isSavedFilm && NetworkTransport::Get().IsActive()) {
        NetworkTransport::Get().Shutdown();
    }
    bool isMainMenu = isSavedFilm ? false : (mapId == kMainMenuLaunchIndex || mapId == 0 || gameModeTag == 0);
    const bool isGuest = !connectAddress.empty();
    if (!isMainMenu && !isSavedFilm && !NetworkTransport::Get().IsActive()) {
        if (isGuest) {
            std::string tip = "127.0.0.1";
            uint16_t tport = 11774;
            size_t c = connectAddress.rfind(':');
            if (c != std::string::npos) {
                tip = connectAddress.substr(0, c);
                int p = std::atoi(connectAddress.substr(c + 1).c_str());
                if (p > 0 && p <= 65535) tport = static_cast<uint16_t>(p);
            } else {
                tip = connectAddress;
            }
            NetworkTransport::Get().Initialize(false, tip, tport);
        } else {
            NetworkTransport::Get().Initialize(true);
        }
    }

    std::string targetHostIp = "127.0.0.1";
    uint16_t targetHostPort = 11774;
    if (isGuest) {
        size_t colon = connectAddress.rfind(':');
        if (colon != std::string::npos) {
            targetHostIp = connectAddress.substr(0, colon);
            int p = std::atoi(connectAddress.substr(colon + 1).c_str());
            if (p > 0 && p <= 65535) targetHostPort = static_cast<uint16_t>(p);
        } else {
            targetHostIp = connectAddress;
        }
    }

    uint8_t targetOctets[4] = { 127, 0, 0, 1 };
    int to0 = 0, to1 = 0, to2 = 0, to3 = 0;
    if (sscanf_s(targetHostIp.c_str(), "%d.%d.%d.%d", &to0, &to1, &to2, &to3) == 4) {
        targetOctets[0] = static_cast<uint8_t>(to0);
        targetOctets[1] = static_cast<uint8_t>(to1);
        targetOctets[2] = static_cast<uint8_t>(to2);
        targetOctets[3] = static_cast<uint8_t>(to3);
    }

    // The mode tag decides, not the map's number: a Forge launch on a Firefight scenario is still Forge, and
    // keying these off the map range made it load the Firefight block and HUD.
    const bool isCampaign = (gameModeTag == 1);
    const bool isFirefight = (gameModeTag == 1035);
    const bool isForge = (gameModeTag == 3);
    uint32_t effectiveMapId = isMainMenu ? static_cast<uint32_t>(-2) : mapId;
    // Diagnostic: force which map's launch block is loaded, independent of the map being launched.
    uint32_t blockMapId = mapId;
    if (const char* forcedBlockMap = std::getenv("NOBLE_BLOCK_MAP")) {
        blockMapId = static_cast<uint32_t>(std::stoul(forcedBlockMap));
        NOBLE_WARN("BlamEngine: launch block forced to map id {} (launching {})", blockMapId, mapId);
    }

    std::string blockName;
    bool convertToCampaign = false;
    if (isMainMenu) {
        blockName = "lb_m_c_1.bin";
    } else if (gameModeTag == 1035) {
        std::string cand1 = "firefight_" + std::to_string(blockMapId) + "_d" + std::to_string(clampedDiff) + ".bin";
        std::string cand2 = "firefight_" + std::to_string(blockMapId) + ".bin";
        if (EmbeddedBlocks::HasBlock(cand1)) blockName = cand1;
        else if (EmbeddedBlocks::HasBlock(cand2)) blockName = cand2;
        else if (std::filesystem::exists(dataRoot / "blocks" / cand1)) blockPath = dataRoot / "blocks" / cand1;
        else if (std::filesystem::exists(dataRoot / "blocks" / cand2)) blockPath = dataRoot / "blocks" / cand2;
    } else if (gameModeTag == 1) {
        std::string cand1 = "campaign_" + std::to_string(blockMapId) + "_d" + std::to_string(clampedDiff) + ".bin";
        std::string cand2 = "campaign_" + std::to_string(blockMapId) + ".bin";
        if (EmbeddedBlocks::HasBlock(cand1)) blockName = cand1;
        else if (EmbeddedBlocks::HasBlock(cand2)) blockName = cand2;
        else if (std::filesystem::exists(dataRoot / "blocks" / cand1)) blockPath = dataRoot / "blocks" / cand1;
        else if (std::filesystem::exists(dataRoot / "blocks" / cand2)) blockPath = dataRoot / "blocks" / cand2;
        // A campaign mission on a map that only ships Firefight blocks (e.g. a mod map on a Firefight canvas):
        // the block differs only in four mode bytes, so take the Firefight block for this map and present it as
        // campaign (0x0C/0x28 = 1, 0x2A/0x2B = 0x11, as every campaign block has).
        if (blockName.empty() && blockPath.empty()) {
            std::string ff1 = "firefight_" + std::to_string(blockMapId) + "_d" + std::to_string(clampedDiff) + ".bin";
            std::string ff2 = "firefight_" + std::to_string(blockMapId) + ".bin";
            if (EmbeddedBlocks::HasBlock(ff1)) { blockName = ff1; convertToCampaign = true; }
            else if (EmbeddedBlocks::HasBlock(ff2)) { blockName = ff2; convertToCampaign = true; }
            else if (std::filesystem::exists(dataRoot / "blocks" / ff1)) { blockPath = dataRoot / "blocks" / ff1; convertToCampaign = true; }
            else if (std::filesystem::exists(dataRoot / "blocks" / ff2)) { blockPath = dataRoot / "blocks" / ff2; convertToCampaign = true; }
        }
    }

    if (!blockName.empty()) {
        size_t embSize = 0;
        const uint8_t* embData = EmbeddedBlocks::GetBlock(blockName, &embSize);
        if (embData && embSize > 0) {
            size_t toCopy = std::min(embSize, static_cast<size_t>(LAUNCH_BLOCK_SIZE));
            std::memcpy(block.data(), embData, toCopy);
            loadedPrebuilt = true;
            NOBLE_INFO("BlamEngine: Loaded authentic pre-built launch block from embedded resource '{}' ({} bytes)", blockName, embSize);
        }
    }

    // Fallback: Check disk if embedded resource wasn't used
    if (!loadedPrebuilt && !blockPath.empty() && std::filesystem::exists(blockPath)) {
        std::ifstream file(blockPath, std::ios::binary);
        if (file) {
            file.read(reinterpret_cast<char*>(block.data()), LAUNCH_BLOCK_SIZE);
            loadedPrebuilt = true;
            NOBLE_INFO("BlamEngine: Loaded authentic pre-built launch block from '{}'", blockPath.string());
        }
    }

    if (convertToCampaign) {
        block[0x0C] = 0x01;
        block[0x28] = 0x01;
        block[0x2A] = 0x11;
        block[0x2B] = 0x11;
        NOBLE_INFO("BlamEngine: Firefight block for map {} presented as campaign (mode bytes 0x0C/0x28/0x2A/0x2B)", blockMapId);
    }

    // Fallback: Synthesize from lb_m_c_1.bin template if needed
    if (!loadedPrebuilt && (isCampaign || isFirefight)) {
        size_t embTmplSize = 0;
        const uint8_t* embTmpl = EmbeddedBlocks::GetBlock("lb_m_c_1.bin", &embTmplSize);
        if (embTmpl && embTmplSize > 0) {
            size_t toCopy = std::min(embTmplSize, static_cast<size_t>(LAUNCH_BLOCK_SIZE));
            std::memcpy(block.data(), embTmpl, toCopy);
            loadedPrebuilt = true;
            NOBLE_INFO("BlamEngine: Synthesizing launch block from embedded 'lb_m_c_1.bin' template for mode {}", isCampaign ? "Campaign" : "Firefight");
        } else if (std::filesystem::exists(dataRoot / "lb_m_c_1.bin")) {
            std::ifstream file(dataRoot / "lb_m_c_1.bin", std::ios::binary);
            if (file) {
                file.read(reinterpret_cast<char*>(block.data()), LAUNCH_BLOCK_SIZE);
                loadedPrebuilt = true;
                NOBLE_INFO("BlamEngine: Synthesizing launch block from disk template for mode {}", isCampaign ? "Campaign" : "Firefight");
            }
        }
    }

    if (!loadedPrebuilt) {
        // Initial header markers (reversed from host-probe::new_launch_block)
        block[0] = isGuest ? 0x00 : 0x08;
        block[1] = 0x03;
        block[2] = 0x00;
        block[3] = 0x10;
        block[4] = 0x03;
        // +0x0C selects the engine class: 1 = campaign, 3 = multiplayer/Megalo (Forge is a
        // multiplayer mode), 5 = Firefight. 0x04 is the special map-editor path, NOT Forge:
        // the engine's mode dispatcher explicitly excludes 0x04 from the normal game-start
        // branch (see haloreach+0xDCCC), so launching Forge with it stalls at launch state 2.
        const int overrideMode = BlamEngine::EngineModeOverride();
        block[0x0C] = static_cast<uint8_t>(overrideMode > 0
                                               ? overrideMode
                                               : ((isMainMenu || isCampaign) ? 0x01 : (isFirefight ? 0x05 : 0x03)));
        if (overrideMode > 0) {
            NOBLE_WARN("BlamEngine: launch-block +0x0C forced to 0x{:X} by -engine-mode (research hook)", overrideMode);
        }
        block[0x10] = isMainMenu ? 0xFE : 0xBE;
        block[0x11] = isMainMenu ? 0xFF : 0x00;
        block[0x12] = isMainMenu ? 0xFF : 0x00;
        block[0x13] = isMainMenu ? 0xFF : 0x00;
        block[0x14] = isMainMenu ? 0xFE : 0xBE;
        block[0x15] = isMainMenu ? 0xFF : 0x00;
        block[0x16] = isMainMenu ? 0xFF : 0x00;
        block[0x17] = isMainMenu ? 0xFF : 0x00;
        block[0x1A] = 0x88;
        block[0x1B] = 0x88;
        block[0x34] = 0x01;
        block[0x44] = 0xFF;
        block[0x45] = 0xFF;

        // Set 16 player slot descriptors to unassigned (-1)
        for (size_t slot = 0; slot < 16; ++slot) {
            size_t offset = 0x100 + (slot * 0x20);
            block[offset + 0] = 0xFF;
            block[offset + 1] = 0xFF;
            block[offset + 2] = 0xFF;
            block[offset + 3] = 0xFF;
            for (size_t b = 8; b < 16; ++b) {
                block[offset + b] = 0xFF;
            }
        }
    }

    uint64_t localMachineId = NetworkTransport::Get().GetLocalMachineId();
    uint64_t hostMachineId = NetworkTransport::Get().GetHostMachineId();

    if (isGuest) {
        // Guest mode: 1 local active player, target host machine at 0x2F8.
        // Note: +0xF0..+0xF7 is the 8-byte s_player_identifier copied by haloreach.dll (State 6,
        // RVA 0xE804) into 0x182A1E94C and sent in join-request. It MUST be unique per client so
        // c_network_session::can_accept_player_join_request (RVA 0x3973BC -> 0x39D59C) on the Host
        // does not see it colliding with Host Player 0 (0x000000003F59F3BE) and reject with 0x16.
        uint64_t guestPlayerId = localMachineId ^ 0x3F59F3BE00000000ULL;
        if (guestPlayerId == 0 || guestPlayerId == 0x000000003F59F3BEULL) {
            guestPlayerId = localMachineId | 0x504C520000000001ULL;
        }
        block[0] &= ~0x08; // bit 3 cleared for guest
        *reinterpret_cast<uint64_t*>(&block[0x60]) = 0; // Host machine id is 0 at 0x60 on guest
        block[0xE8] = 0x01; // 1 active local player
        *reinterpret_cast<uint64_t*>(&block[0xF0]) = guestPlayerId;
        *reinterpret_cast<uint64_t*>(&block[0xF8]) = localMachineId; // Local player 0 machine ID
        // Slot 0 is reserved for remote Host
        block[0x100] = 0xFF;
        block[0x101] = 0xFF;
        block[0x102] = 0xFF;
        block[0x103] = 0xFF;
        block[0x104] = 0x00;
        block[0x108] = 0xFF;
        block[0x109] = 0xFF;
        block[0x10A] = 0xFF;
        block[0x10B] = 0xFF;
        // Slot 1 is bound to local controller 0 for this Guest
        block[0x120] = 0x00;
        block[0x121] = 0x00;
        block[0x122] = 0x00;
        block[0x123] = 0x00;
        block[0x124] = 0x02;
        block[0x128] = 0x00;
        block[0x129] = 0x00;
        block[0x12A] = 0x00;
        block[0x12B] = 0x00;
        // Slots 2..15 are unassigned
        for (size_t s = 2; s < 16; ++s) {
            size_t off = 0x100 + (s * 0x20);
            block[off + 0] = 0xFF;
            block[off + 1] = 0xFF;
            block[off + 2] = 0xFF;
            block[off + 3] = 0xFF;
            block[off + 4] = 0x00;
            for (size_t b = 8; b < 16; ++b) block[off + b] = 0xFF;
        }
        block[0x2F0] = 0x00;
        *reinterpret_cast<uint32_t*>(&block[0x2F4]) = 0xFFFFFFFF;
        *reinterpret_cast<uint64_t*>(&block[0x2F8]) = hostMachineId; // uint64 Host Machine ID
        block[0x2BF14] = 0xF3;
        block[0x2BF15] = 0x01;
        NOBLE_INFO("BlamEngine: Configured Guest Launch Block targeting host {}:{} (Local: 0x{:X}, Host: 0x{:X}, PlayerId: 0x{:X})",
            targetHostIp, targetHostPort, localMachineId, hostMachineId, guestPlayerId);
    } else {
        // Host mode: 1 local active player, host machine at 0x60 and 0xF8
        block[0] |= 0x08; // bit 3 set for host
        *reinterpret_cast<uint64_t*>(&block[0x60]) = localMachineId;
        block[0xE8] = 0x01; // 1 active local player
        block[0xF0] = 0xBE;
        block[0xF1] = 0xF3;
        block[0xF2] = 'Y';
        block[0xF3] = '?';
        *reinterpret_cast<uint64_t*>(&block[0xF8]) = localMachineId;
        block[0x100] = 0x00;
        block[0x101] = 0x00;
        block[0x102] = 0x00;
        block[0x103] = 0x00;
        block[0x104] = 0x02;
        block[0x108] = 0x00;
        block[0x109] = 0x00;
        block[0x10A] = 0x00;
        block[0x10B] = 0x00;

        // Pre-allocate slots 1..15 loaded in the chamber for mid-match join-in-progress / direct connect
        for (size_t s = 1; s < 16; ++s) {
            size_t off = 0x100 + (s * 0x20);
            block[off + 0] = 0xFF;
            block[off + 1] = 0xFF;
            block[off + 2] = 0xFF;
            block[off + 3] = 0xFF;
            block[off + 4] = 0x00;
            for (size_t b = 8; b < 16; ++b) block[off + b] = 0xFF;
        }

        block[0x2F0] = 0x01;
        *reinterpret_cast<uint32_t*>(&block[0x2F4]) = 0;
        *reinterpret_cast<uint64_t*>(&block[0x2F8]) = 0;
    }

    if (!loadedPrebuilt) {
        // Default variant buffer init at offset 0x1D308 (RVA 0x6BFF8)
        if (m_hModule) {
            SafeCallVariantInit(m_hModule, &block[0x1D308]);
        }

        // Map ID & Mode tagging (from host-probe::set_map lines 78-92)
        *reinterpret_cast<uint32_t*>(&block[0x10]) = effectiveMapId;
        *reinterpret_cast<uint32_t*>(&block[0x14]) = effectiveMapId;
        *reinterpret_cast<uint32_t*>(&block[0x18]) = 0x88880000;
        *reinterpret_cast<uint64_t*>(&block[0x1C]) = 0;
        *reinterpret_cast<uint32_t*>(&block[0x1D5C0]) = effectiveMapId;
        *reinterpret_cast<uint32_t*>(&block[0x1D5C4]) = 0x88880000;
        *reinterpret_cast<uint64_t*>(&block[0x1D5C8]) = 0;
        // +0x1D334 / +0x1D5BC hold the MAP ID (e.g. Sword Base 1000), not a game-mode tag. With 0 here the
        // engine resolves an empty scenario name ("haloreach\maps\.map") and has nothing to render.
        // Menu and multiplayer ids are known; campaign/firefight keep the previous value until verified.
        uint32_t mapInfoId = MapIdForLaunchIndex(effectiveMapId);
        // Campaign blocks keep the menu/Boardwalk map-variant id (1035) here; the mission itself is
        // selected by the launch index at +0x10/+0x14/+0x1D5C0 (verified against the engine's captured
        // campaign blocks, which differ from its MP block only in +0x0C, +0x10/+0x14, +0x28,
        // +0x34 and +0x1D5C0).
        // Applies to a Firefight canvas launched in Forge too: MapIdForLaunchIndex() has no entry for 210..219,
        // so the block would carry a game-mode tag as its map id and the engine answers 'map and game
        // incompatible' (the capture convention is to keep the menu map-variant id here).
        if (isCampaign || isFirefight || (mapId >= 210 && mapId <= 219)) mapInfoId = kMainMenuMapId;
        if (mapInfoId == 0) mapInfoId = gameModeTag;
        if (isCampaign) {
            block[0x28] = 0x01;
            block[0x29] = 0x00;
            block[0x2A] = 0x11;
            block[0x2B] = 0x11;
        }
        *reinterpret_cast<uint32_t*>(&block[0x1D334]) = mapInfoId;
        *reinterpret_cast<uint32_t*>(&block[0x1D5BC]) = mapInfoId;
        NOBLE_INFO("BlamEngine: launch block map index {} -> map id {}", static_cast<int32_t>(effectiveMapId), mapInfoId);
    }

    // Synchronized Network Session ID at +0x50 and local machine ID at +0x58
    if (!isMainMenu || !loadedPrebuilt) {
        localMachineId = NetworkTransport::Get().GetLocalMachineId();
        hostMachineId = NetworkTransport::Get().GetHostMachineId();
        *reinterpret_cast<uint64_t*>(&block[0x50]) = NetworkTransport::Get().GetSessionId();
        *reinterpret_cast<uint64_t*>(&block[0x58]) = localMachineId;
        if (!isMainMenu && loadedPrebuilt) {
            if (isGuest) {
                uint64_t guestPlayerId = localMachineId ^ 0x3F59F3BE00000000ULL;
                if (guestPlayerId == 0 || guestPlayerId == 0x000000003F59F3BEULL) {
                    guestPlayerId = localMachineId | 0x504C520000000001ULL;
                }
                block[0] &= ~0x08;
                *reinterpret_cast<uint64_t*>(&block[0x60]) = 0;
                block[0xE8] = 0x01;
                *reinterpret_cast<uint64_t*>(&block[0xF0]) = guestPlayerId;
                *reinterpret_cast<uint64_t*>(&block[0xF8]) = localMachineId;
                // Slot 0 is reserved for remote Host
                block[0x100] = 0xFF;
                block[0x101] = 0xFF;
                block[0x102] = 0xFF;
                block[0x103] = 0xFF;
                block[0x104] = 0x00;
                block[0x108] = 0xFF;
                block[0x109] = 0xFF;
                block[0x10A] = 0xFF;
                block[0x10B] = 0xFF;
                // Slot 1 is bound to local controller 0 for this Guest
                block[0x120] = 0x00;
                block[0x121] = 0x00;
                block[0x122] = 0x00;
                block[0x123] = 0x00;
                block[0x124] = 0x02;
                block[0x128] = 0x00;
                block[0x129] = 0x00;
                block[0x12A] = 0x00;
                block[0x12B] = 0x00;
                block[0x2F0] = 0x00;
                *reinterpret_cast<uint32_t*>(&block[0x2F4]) = 0xFFFFFFFF;
                *reinterpret_cast<uint64_t*>(&block[0x2F8]) = hostMachineId;
                block[0x2BF14] = 0xF3;
                block[0x2BF15] = 0x01;
            } else {
                block[0] |= 0x08;
                *reinterpret_cast<uint64_t*>(&block[0x60]) = localMachineId;
                block[0xE8] = 0x01;
                *reinterpret_cast<uint64_t*>(&block[0xF8]) = localMachineId;
                // The host's player identifier and slot descriptor are needed by campaign/firefight too: without
                // +0xF0 the local player is not bound, the intro spawns out of bounds and the mission sits on a
                // black screen. This was gated to multiplayer in 145510a and is the campaign regression.
                {
                    *reinterpret_cast<uint64_t*>(&block[0xF0]) = 0x000000003F59F3BEULL;
                    block[0x100] = 0x00;
                    block[0x101] = 0x00;
                    block[0x102] = 0x00;
                    block[0x103] = 0x00;
                    block[0x104] = 0x02;
                    block[0x108] = 0x00;
                    block[0x109] = 0x00;
                    block[0x10A] = 0x00;
                    block[0x10B] = 0x00;
                    block[0x2F0] = 0x01;
                    *reinterpret_cast<uint32_t*>(&block[0x2F4]) = 0;
                    *reinterpret_cast<uint64_t*>(&block[0x2F8]) = 0;
                    block[0x2BF14] = 0xF3;
                    block[0x2BF15] = 0x01;
                }
            }

            // Reset unassigned slots on host and guest so remote peers can be registered. Campaign/Firefight
            // included: 5+ co-op needs every free descriptor, and the authentic block already has 0xFF here.
            {
                size_t startSlot = isGuest ? 2 : 1;
                for (size_t slot = startSlot; slot < 16; ++slot) {
                    size_t offset = 0x100 + (slot * 0x20);
                    block[offset + 0] = 0xFF;
                    block[offset + 1] = 0xFF;
                    block[offset + 2] = 0xFF;
                    block[offset + 3] = 0xFF;
                    block[offset + 4] = 0x00;
                    block[offset + 5] = 0x00;
                    block[offset + 6] = 0x00;
                    block[offset + 7] = 0x00;
                    for (size_t b = 8; b < 16; ++b) {
                        block[offset + b] = 0xFF;
                    }
                }
            }
        }
    }

    if (!isMainMenu) {
        uint32_t mapInfoId = MapIdForLaunchIndex(effectiveMapId);
        // Firefight canvases keep the menu/Boardwalk map-variant id and are selected by their launch index (as the
        // captured Firefight blocks do). Applies in Forge too: MapIdForLaunchIndex() is 0 for 210..219, so the
        // block would otherwise carry a game-mode tag as its map id and the engine answers 'map and game
        // incompatible'.
        if (isCampaign || isFirefight || (mapId >= 210 && mapId <= 219)) mapInfoId = kMainMenuMapId;
        if (isCampaign) {
            block[0x28] = 0x01;
            block[0x29] = 0x00;
            block[0x2A] = 0x11;
            block[0x2B] = 0x11;
        }
        *reinterpret_cast<uint32_t*>(&block[0x1D334]) = mapInfoId;
        *reinterpret_cast<uint32_t*>(&block[0x1D5BC]) = mapInfoId;
    }

    if (isCampaign || isFirefight) {
        // Campaign / Firefight: difficulty only at +0x34 (0 easy .. 3 legendary); +0x38 stays 0 as in the engine's blocks.
        block[0x34] = static_cast<uint8_t>(clampedDiff);
    } else if (!isMainMenu) {
        // Apply difficulty override
        block[0x34] = static_cast<uint8_t>(clampedDiff);
        *reinterpret_cast<uint32_t*>(&block[0x38]) = static_cast<uint32_t>(clampedDiff);
    }
    // Skulls: u64 at +0x48 in MCC's skull numbering (bit = the skull's index in MCC's
    // alphabetical list: Iron 18, Black Eye 3, Tough Luck 36, Catch 7, Cloud 13, Famine 11,
    // Thunderstorm 34, Tilt 35, Mythic 23, Blind 4, Cowbell 8, Grunt Birthday Party 16, IWHBYD 19).
    // haloreach+0x3A22C converts it to the engine's own skull bits (game options +0x1D8).
    // (+0x3C, used before, is a different field the engine reads as u16.)
    if (!isMainMenu) {
        memcpy(&block[0x48], &skulls, sizeof(skulls));
    }

    // Dynamic Custom Map Variant (.mvar) Injection (RVA 0x17AF00 parse_map_variant)
    if (!customMvarPath.empty() && std::filesystem::exists(customMvarPath) && m_hModule) {
        NOBLE_INFO("BlamEngine: Parsing custom Forge map variant from '{}'...", customMvarPath);
        std::ifstream file(customMvarPath, std::ios::binary);
        if (file) {
            std::vector<uint8_t> mvarData((std::istreambuf_iterator<char>(file)),
                                           std::istreambuf_iterator<char>());
            if (mvarData.size() >= 0x7329) {
                std::vector<uint8_t> parsedVariant(0x20000, 0); // 128 KB buffer
                SafeCallVariantInit(m_hModule, parsedVariant.data());
                char status = 0;
                if (SafeCallMapVariantParse(m_hModule, mvarData.data(), 0x7329, parsedVariant.data(), &status) && status != 0) {
                    // Dev: keep the file and the structure the engine parsed it into, so the mapping between them can
                    // be worked out (that is what a save has to invert - the engine's own save produces no bytes here).
                    if (GetEnvironmentVariableW(L"NOBLE_MVAR_PAIR", nullptr, 0) || GetCommandLineW() != nullptr) {
                        static int s_pair = 0;
                        if (s_pair < 8) {
                            const std::filesystem::path out = std::filesystem::path("data") / "mvar_pair";
                            std::error_code ec;
                            std::filesystem::create_directories(out, ec);
                            char tag[32] = {};
                            std::snprintf(tag, sizeof(tag), "pair_%02d", ++s_pair);
                            std::ofstream(out / (std::string(tag) + "_file.bin"), std::ios::binary)
                                .write(reinterpret_cast<const char*>(mvarData.data()), static_cast<std::streamsize>(mvarData.size()));
                            std::ofstream(out / (std::string(tag) + "_parsed.bin"), std::ios::binary)
                                .write(reinterpret_cast<const char*>(parsedVariant.data()), static_cast<std::streamsize>(parsedVariant.size()));
                            NOBLE_INFO("BlamEngine: map variant pair '{}' written (file {} bytes, parsed {} bytes)", tag,
                                       mvarData.size(), parsedVariant.size());
                        }
                    }
                    memcpy(&block[0x1D308], parsedVariant.data(), 0xE800);
                    uint32_t mvarMapId = *reinterpret_cast<uint32_t*>(&block[0x1D5BC]);
                    NOBLE_INFO("BlamEngine: Custom map variant parsed successfully. Target MapId: {}", mvarMapId);
                } else {
                    NOBLE_WARN("BlamEngine: Failed to parse map variant header (status=0x{:X})", status);
                }
            } else {
                NOBLE_WARN("BlamEngine: Map variant file size ({} bytes) is less than required 0x7329 bytes", mvarData.size());
            }
        }
    }

    // Game variant at +0x300 (0xFC04 bytes), decoded by the engine's own decoder (RVA 0x373A0),
    // as the reference host does. A raw .bin file is NOT the in-memory layout, so never memcpy it.
    // Without a variant the engine cannot resolve what to load (it requests "haloreach\maps\.map"
    // and stalls at 4%). Default: MCC's stock slayer_054.bin from the player's own install.
    // NOTE: Authentic pre-built Campaign launch blocks already contain authentic campaign state
    // and mission variants at +0x300, and must NOT be overwritten with multiplayer Slayer!
    // The main menu (mainmenu_all.map) never carries a game variant, so it skips this block entirely
    // (before 2026-10-29 it logged a bogus "No game variant found (looked for '')" on every menu boot).
    // Campaign blocks normally carry the mission's own variant, but a mission launched from the campaign screen
    // also hands one over ("m20.bin" and friends) and the engine stalls at 4% without it. Decode that one - it
    // only ever replaces the block's variant with the mission's own, never with a multiplayer mode.
    const bool campaignVariant = isCampaign && !customVariantPath.empty() &&
                                 customVariantPath != "Campaign" && customVariantPath != "campaign";
    if (!isMainMenu && (!isCampaign || campaignVariant)) {
        std::filesystem::path variantPath;
        std::optional<VariantRules::Preset> gameMode;
        if (!customVariantPath.empty()) {
            variantPath = ResolveVariantFile(customVariantPath);
            // A player-made game mode: the shipped variant it starts from, plus its changed rules (applied after decoding).
            if (!variantPath.empty() && VariantRules::IsPresetFile(variantPath)) {
                VariantRules::Preset loaded;
                if (VariantRules::Load(variantPath, loaded)) {
                    gameMode = std::move(loaded);
                    NOBLE_INFO("BlamEngine: game mode '{}' from '{}' on base '{}', {} rule(s) changed", gameMode->name, variantPath.string(), gameMode->base, gameMode->values.size());
                    variantPath = ResolveVariantFile(gameMode->base);
                } else {
                    NOBLE_WARN("BlamEngine: game mode file '{}' could not be read", variantPath.string());
                    variantPath.clear();
                }
            }
            const std::filesystem::path requestedPath = variantPath;
            // mapId here is the launch index (190..209); the engine's compatibility field is
            // the map-info id (e.g. Boneyard 1080, Spire 1200, Breakpoint 2002).
            variantPath = SelectMapSpecificVariant(variantPath, MapIdForLaunchIndex(mapId));
            if (!requestedPath.empty() && variantPath != requestedPath) {
                NOBLE_INFO("BlamEngine: '{}' is not compatible with this map; using map-specific '{}' instead",
                           requestedPath.filename().string(), variantPath.filename().string());
            }
            if (variantPath.empty()) {
                NOBLE_WARN("BlamEngine: requested game variant '{}' was not found; refusing to substitute Slayer. "
                           "The match will start with the launch block's embedded mode instead.",
                           customVariantPath);
            }
        } else if (!isCampaign) {
            // Firefight needs a Firefight game variant (otherwise the engine reports "no game
            // selected"); the engine likewise loads <MCC>\haloreach\game_variants\<mode>.bin for it.
            // Forge needs 00_basic_editing_054.bin for item spawning and editing.
            // Campaign blocks already contain their own mission variant and are left untouched.
            const char* defaultVariant = isFirefight ? "hr_ff_standard_1set.bin"
                                       : isForge     ? "00_basic_editing_054.bin"
                                       : "slayer_054.bin";
            variantPath = ReachHost::Get().GetReachDir() / "game_variants" / defaultVariant;
        }

        if (!variantPath.empty() && std::filesystem::exists(variantPath) && m_hModule) {
            std::ifstream varFile(variantPath, std::ios::binary);
            std::vector<uint8_t> varData((std::istreambuf_iterator<char>(varFile)),
                                          std::istreambuf_iterator<char>());
            if (varData.size() < 0x5329) {
                NOBLE_WARN("BlamEngine: Game variant '{}' is too small ({} bytes)", variantPath.string(), varData.size());
            } else {
                std::vector<uint8_t> decoded(0xFC04, 0);
                alignas(16) uint8_t status[256] = {};
                if (SafeCallGameVariantDecode(m_hModule, varData.data(), decoded.data(), status)) {
                    VariantFields fields;
                    if (fields.Read(decoded.data(), decoded.size())) {
                        if (!s_variantPatch.empty()) {
                            ApplyVariantPatch(s_variantPatch, fields);
                            fields.Write(decoded.data(), decoded.size());
                            NOBLE_INFO("BlamEngine: applied variant patch '{}' (time {} score {} rounds {})",
                                       s_variantPatch, fields.timeLimit, fields.scoreToWin, fields.roundsToWin);
                        }
                        if (s_variantOverrides) {
                            fields = *s_variantOverrides;
                            fields.Write(decoded.data(), decoded.size());
                            NOBLE_INFO("BlamEngine: applied host variant overrides (time {}, score {}, rounds {})",
                                       fields.timeLimit, fields.scoreToWin, fields.roundsToWin);
                        }
                        NOBLE_INFO("BlamEngine: variant time {} score {} rounds {} respawn {} traits {}",
                                   fields.timeLimit, fields.scoreToWin, fields.roundsToWin,
                                   fields.respawnTime, fields.traitsDuration);
                    }
                    if (gameMode) VariantRules::Apply(*gameMode, decoded.data());
                    for (const VariantRules::Rule& rule : VariantRules::Rules()) {
                        if (std::strcmp(rule.key, "teams") == 0) {
                            s_gameTeamsEnabled.store(VariantRules::Read(rule, decoded.data()) != 0, std::memory_order_relaxed);
                            break;
                        }
                    }
                    // Test option "-variant-poke 319=5,2BE=2": sets decoded bytes, to find out what an offset does in a real match.
                    {
                        int argc = 0;
                        LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);
                        for (int i = 1; argv && i + 1 < argc; ++i) {
                            if (_wcsicmp(argv[i], L"-variant-poke") != 0) continue;
                            for (const wchar_t* c = argv[i + 1]; *c;) {
                                wchar_t* end = nullptr;
                                const unsigned long offset = wcstoul(c, &end, 16);
                                if (*end != L'=') break;
                                const unsigned long value = wcstoul(end + 1, &end, 16);
                                if (offset < decoded.size()) decoded[offset] = static_cast<uint8_t>(value);
                                NOBLE_INFO("BlamEngine: -variant-poke {:X} = {:X}", offset, value);
                                c = *end == L',' ? end + 1 : end;
                                if (*c == 0 || c == end) break;
                            }
                        }
                        if (argv) LocalFree(argv);
                    }
                    memcpy(&block[0x300], decoded.data(), decoded.size());
                    NOBLE_INFO("BlamEngine: Decoded game variant '{}' into launch block (+0x300)", variantPath.string());
                } else {
                    NOBLE_ERROR("BlamEngine: haloreach.dll could not decode game variant '{}'", variantPath.string());
                }
            }
        } else if (!variantPath.empty()) {
            NOBLE_WARN("BlamEngine: No game variant found for launch block (looked for '{}')", variantPath.string());
        }
    }

    // Test switch "--guest-block": lay out a guest's block the way the reference host does
    // (host-probe.exe host_probe::net::patch_launch_block, "--as-guest", read 2026-10-29): no local players
    // or player descriptors (+0xE8..+0x10F zero, +0x100 = 0xFFFFFFFF, +0x108 = -1), +0x2F0..+0x2FF zero with
    // +0x2F4 = 0xFFFFFFFF, everything from +0x300 on zeroed (no game variant: the host's comes over the
    // session), +0x2BF14 = F3 01, then +0x50 session, +0x58 self, +0x2F8 host. Noble's default guest block sets
    // one local player, a player id at +0xF0 and the decoded variant instead; guests have been stuck at
    // launch state 9 with traffic flowing both ways, so this lets the two layouts be compared.
    if (isGuest && !isMainMenu && std::wcsstr(GetCommandLineW(), L"--guest-block") != nullptr) {
        const uint64_t self = NetworkTransport::Get().GetLocalMachineId();
        const uint64_t host = NetworkTransport::Get().GetHostMachineId();
        const uint64_t session = NetworkTransport::Get().GetSessionId();
        block[0] &= static_cast<uint8_t>(~0x08);
        *reinterpret_cast<uint64_t*>(&block[0x60]) = 0;
        std::memset(&block[0xE8], 0, 0x110 - 0xE8);
        *reinterpret_cast<uint32_t*>(&block[0x100]) = 0xFFFFFFFFu;
        *reinterpret_cast<uint64_t*>(&block[0x108]) = ~0ULL;
        std::memset(&block[0x2F0], 0, 0x10);
        *reinterpret_cast<uint32_t*>(&block[0x2F4]) = 0xFFFFFFFFu;
        std::memset(&block[0x300], 0, 0x1D308 - 0x300);
        block[0x2BF14] = 0xF3;
        block[0x2BF15] = 0x01;
        *reinterpret_cast<uint64_t*>(&block[0x50]) = session;
        *reinterpret_cast<uint64_t*>(&block[0x58]) = self;
        *reinterpret_cast<uint64_t*>(&block[0x2F8]) = host;
        NOBLE_INFO("BlamEngine: --guest-block: guest block laid out like the engine (self 0x{:X}, host 0x{:X}, session 0x{:X})",
                   self, host, session);
    }

    // Test-only: "-launch-poke 0x3C=0xFFFFFFFF,0x40=1" writes u32 values into the block (for finding
    // fields such as skulls by experiment). Not used by the menu.
    {
        int argc = 0;
        LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);
        for (int i = 1; argv && i + 1 < argc; ++i) {
            if (std::wstring(argv[i]) != L"-launch-poke") continue;
            std::wstring list = argv[i + 1];
            size_t pos = 0;
            while (pos < list.size()) {
                size_t comma = list.find(L',', pos);
                if (comma == std::wstring::npos) comma = list.size();
                const std::wstring item = list.substr(pos, comma - pos);
                const size_t eq = item.find(L'=');
                if (eq != std::wstring::npos) {
                    const unsigned long off = wcstoul(item.substr(0, eq).c_str(), nullptr, 0);
                    const unsigned long val = wcstoul(item.substr(eq + 1).c_str(), nullptr, 0);
                    if (off + 4 <= block.size()) {
                        *reinterpret_cast<uint32_t*>(&block[off]) = static_cast<uint32_t>(val);
                        NOBLE_INFO("BlamEngine: -launch-poke +0x{:X} = 0x{:X}", off, val);
                    }
                }
                pos = comma + 1;
            }
        }
        if (argv) LocalFree(argv);
    }

    // Theater Mode saved film playback: launch_block + 0x2BF20 holds a const char* pointer to the .film path.
    // When non-null, haloreach.dll (RVA 0x100C4 -> 0x5A1BC) opens the .film file, loads the embedded flmh
    // game_options (map, game variant, map variant, players, seed) and activates c_saved_film_director.
    if (!s_savedFilmPath.empty()) {
        static std::string s_activeFilmPathBuf;
        std::error_code ec;
        const std::filesystem::path srcPath(s_savedFilmPath);
        if (std::filesystem::is_regular_file(srcPath, ec)) {
            // Read the film's chdr/flmh mode byte so launch_block +0x0C/+0x28 match the recorded engine mode.
            std::ifstream filmIn(srcPath, std::ios::binary);
            if (filmIn.is_open()) {
                uint8_t headerBuf[0x0900] = {};
                filmIn.read(reinterpret_cast<char*>(headerBuf), sizeof(headerBuf));
                const std::streamsize got = filmIn.gcount();
                uint32_t filmEngineMode = 0;
                if (got >= 0x08EC && headerBuf[0x0340] == 'f' && headerBuf[0x0341] == 'l' &&
                    headerBuf[0x0342] == 'm' && headerBuf[0x0343] == 'h') {
                    const uint8_t flmhMode = headerBuf[0x08E8];
                    if (flmhMode == 1 || flmhMode == 3 || flmhMode == 5) filmEngineMode = flmhMode;
                }
                if (filmEngineMode == 0 && got >= 0x0070) {
                    const uint8_t chdrMode = headerBuf[0x006A];
                    if (chdrMode == 3) filmEngineMode = 1;      // Campaign
                    else if (chdrMode == 4) filmEngineMode = 5; // Firefight
                    else if (chdrMode == 2) filmEngineMode = 3; // Multiplayer / Forge
                }
                if (filmEngineMode != 0) {
                    *reinterpret_cast<uint32_t*>(&block[0x0C]) = filmEngineMode;
                    *reinterpret_cast<uint32_t*>(&block[0x28]) = filmEngineMode;
                    if (filmEngineMode == 1) {
                        block[0x2A] = 0x11;
                        block[0x2B] = 0x11;
                    }
                }
            }

            // Stage the .film into data\sandbox\theater\<filename> and data\sandbox\<filename>
            // because main.cpp sets the working directory to data\sandbox.
            const std::filesystem::path sandboxDir = ReachHost::Get().GetDataDir() / "sandbox";
            const std::filesystem::path sandboxTheaterDir = sandboxDir / "theater";
            std::filesystem::create_directories(sandboxTheaterDir, ec);
            std::string safeName = srcPath.filename().string();
            if (safeName.empty() || safeName.size() > 120) safeName = "playback.film";
            const std::filesystem::path stagedTheaterPath = sandboxTheaterDir / safeName;
            const std::filesystem::path stagedRootPath = sandboxDir / safeName;
            std::filesystem::copy_file(srcPath, stagedTheaterPath, std::filesystem::copy_options::overwrite_existing, ec);
            std::filesystem::copy_file(srcPath, stagedRootPath, std::filesystem::copy_options::overwrite_existing, ec);

            const std::string relPath = "theater\\" + safeName;
            DWORD attr = GetFileAttributesA(relPath.c_str());
            if (attr != INVALID_FILE_ATTRIBUTES) {
                s_activeFilmPathBuf = relPath;
            } else {
                s_activeFilmPathBuf = stagedTheaterPath.string();
            }

            *reinterpret_cast<const char**>(&block[0x2BF20]) = s_activeFilmPathBuf.c_str();
            NOBLE_INFO("BlamEngine: Configured Theater Saved Film playback '{}' (staged as '{}') at launch_block+0x2BF20",
                       srcPath.string(), s_activeFilmPathBuf);
            s_isSavedFilmPlayback = true;
        } else {
            NOBLE_ERROR("BlamEngine: Requested Theater Saved Film '{}' does not exist", s_savedFilmPath);
            s_isSavedFilmPlayback = false;
        }
        s_savedFilmPath.clear();
    }

    return block;
}

bool BlamEngine::ExecuteCommand(const std::string& command) {
    if (!m_engineInstance || !m_fnExecuteCommand) {
        NOBLE_WARN("BlamEngine: Cannot execute command '{}' - Engine not ready.", command);
        return false;
    }

    NOBLE_INFO("BlamEngine: Dispatching command -> '{}'", command);
    int32_t status = m_fnExecuteCommand(m_engineInstance, command.c_str());
    NOBLE_INFO("BlamEngine: Command status: 0x{:X}", status);
    return true;
}

static bool ReadEngineLaunchMapStateSEH(HMODULE hMod, int32_t* outLaunchState, int32_t* outMapState) {
    if (!hMod) return false;
    __try {
        const uintptr_t base = reinterpret_cast<uintptr_t>(hMod);
        if (outLaunchState) *outLaunchState = *reinterpret_cast<const int32_t*>(base + 0xC1A114);
        if (outMapState) *outMapState = *reinterpret_cast<const int32_t*>(base + 0xC1A118);
        return true;
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        return false;
    }
}

bool BlamEngine::LaunchMap(const std::string& mapName, const std::string& gameMode, int difficulty, uint64_t skulls,
                           const std::string& customMvarPath, const std::string& customVariantPath,
                           const std::string& connectAddress, int expectedPeers) {
    // A fresh launch starts with no remote players in membership; the host's Command 11 barrier counts msg 27s.
    noble::LobbyPatches::ResetRemotePlayerJoined();
    s_lastLiveVariantSource.store(nullptr, std::memory_order_release);
    s_lastCopyVariantSource.store(nullptr, std::memory_order_release);
    if (!m_isInitialized || !m_engineInstance) return false;

    std::string effectiveMapName = mapName;
    std::string effectiveGameMode = gameMode;

    if (!s_savedFilmPath.empty()) {
        std::string filmScenario, filmMode;
        uint32_t filmMapId = 0;
        if (InspectSavedFilm(s_savedFilmPath, filmScenario, filmMode, filmMapId)) {
            if (effectiveMapName.empty() || effectiveMapName == "mainmenu" || effectiveMapName == "menu" || ResolveMapId(effectiveMapName) == 0) {
                effectiveMapName = filmScenario;
            }
            if (effectiveGameMode.empty() || effectiveGameMode == "mainmenu" || effectiveGameMode == "menu") {
                effectiveGameMode = filmMode;
            }
            NOBLE_INFO("BlamEngine: Theater film inspect resolved map '{}', mode '{}' (from film '{}')",
                       effectiveMapName, effectiveGameMode, s_savedFilmPath);
        }
    }

    m_currentMap = effectiveMapName;
    m_currentMode = effectiveGameMode;
    m_isGameRunning = true;
    PauseMenu::SetLoadingMap(effectiveMapName);

    NOBLE_INFO("=================================================");
    NOBLE_INFO(" LAUNCHING HALO: REACH MISSION / MAP: {}", effectiveMapName);
    NOBLE_INFO(" Mode: {} | Difficulty: {} | Skulls: 0x{:X}", effectiveGameMode, difficulty, skulls);
    if (!customMvarPath.empty()) NOBLE_INFO(" Custom MVAR: {}", customMvarPath);
    if (!customVariantPath.empty()) NOBLE_INFO(" Custom Variant: {}", customVariantPath);
    if (!s_savedFilmPath.empty()) NOBLE_INFO(" Theater Film: {}", s_savedFilmPath);
    NOBLE_INFO("=================================================");

    uint32_t mapId = ResolveMapId(effectiveMapName);

    // DIAGNOSTIC (temporary): NOBLE_MAP_NAME_OVERRIDE=<name> rewrites the engine's own map-name
    // string for the launch index, so the engine builds maps\<name>.map and maps\info\<name>.mapinfo
    // paths. Used to verify that a modded map file can stand in for a stock map slot.
    char* mapNameOverride = nullptr;
    size_t mapNameOverrideLen = 0;
    if (_dupenv_s(&mapNameOverride, &mapNameOverrideLen, "NOBLE_MAP_NAME_OVERRIDE") == 0 && mapNameOverride) {
        if (mapNameOverride[0] != '\0' && m_hModule && mapId >= 178 && mapId <= 219) {
            using MapNameFn = const char* (*)(int);
            auto engineMapName = reinterpret_cast<MapNameFn>(reinterpret_cast<uintptr_t>(m_hModule) + 0x38E98);
            const char* existing = engineMapName(static_cast<int>(mapId));
            if (existing) {
                const size_t len = std::strlen(mapNameOverride);
                DWORD oldProtect = 0;
                if (VirtualProtect(const_cast<char*>(existing), len + 1, PAGE_READWRITE, &oldProtect)) {
                    std::memcpy(const_cast<char*>(existing), mapNameOverride, len + 1);
                    VirtualProtect(const_cast<char*>(existing), len + 1, oldProtect, &oldProtect);
                    NOBLE_WARN("BlamEngine: DIAGNOSTIC map name override for index {} -> '{}'", mapId, mapNameOverride);
                }
            }
        }
        std::free(mapNameOverride);
    }

    uint32_t gameModeTag = 2; // Default multiplayer (Slayer, CTF, Invasion, all Megalo variants)
    const bool forgeMode = _stricmp(effectiveGameMode.c_str(), "forge") == 0 ||
                           _stricmp(effectiveGameMode.c_str(), "sandbox") == 0;
    // An explicitly requested Forge wins over the map's own mode. Firefight scenarios (Installation 04 =
    // cex_ff_halo, Glacier = ff60_icecave) are the bases of several Forge AI mods, and testing the map range
    // first forced them into Firefight no matter what was asked for.
    if (forgeMode) {
        // Forge is a multiplayer Megalo mode, not the special map-editor engine (0x04).
        gameModeTag = 3;
    } else if (_stricmp(effectiveGameMode.c_str(), "campaign") == 0) {
        gameModeTag = 1;    // Explicit Campaign mode (including Firefight-canvas campaign mods)
    } else if (_stricmp(effectiveGameMode.c_str(), "firefight") == 0 ||
               (_stricmp(effectiveGameMode.c_str(), "multiplayer") != 0 && mapId >= 210 && mapId <= 219)) {
        gameModeTag = 1035; // 0x40B Firefight
    } else if (mapId >= 178 && mapId <= 189) {
        gameModeTag = 1;    // Campaign
    } else if (mapId == static_cast<uint32_t>(-2) || mapId == 0 ||
               _stricmp(effectiveGameMode.c_str(), "mainmenu") == 0 || _stricmp(effectiveGameMode.c_str(), "menu") == 0) {
        gameModeTag = 0;    // Main Menu
    }

    if (gameModeTag == 0 && s_savedFilmPath.empty()) {
        s_isSavedFilmPlayback = false;
    }

    if (!s_savedFilmPath.empty() && gameModeTag == 0) {
        if (_stricmp(effectiveGameMode.c_str(), "campaign") == 0) gameModeTag = 1;
        else if (_stricmp(effectiveGameMode.c_str(), "firefight") == 0) gameModeTag = 1035;
        else if (_stricmp(effectiveGameMode.c_str(), "forge") == 0) gameModeTag = 3;
        else gameModeTag = 2;
    }

    // A multiplayer mod map built on a Firefight canvas is a Forge map in Reach terms: its world is only loaded by the
    // Forge/multiplayer engine. Do this before the map info is presented, since the type written there follows the
    // engine mode.
    const std::string overrideCanvas = noble::VFS::MapOverrideCanvas();
    const bool firefightCanvas =
        noble::VFS::IsFirefightCanvas(effectiveMapName) ||
        (!overrideCanvas.empty() &&
         (noble::VFS::IsFirefightCanvas(overrideCanvas) || overrideCanvas.rfind("ff_", 0) == 0 ||
          overrideCanvas.rfind("ff60", 0) == 0 || overrideCanvas.rfind("cex_ff", 0) == 0));
    if (noble::VFS::HasActiveMod() && gameModeTag == 2 && firefightCanvas) {
        NOBLE_WARN("BlamEngine: '{}' is a multiplayer mod map on a Firefight canvas ({}); launching it through the Forge engine",
                   effectiveMapName, overrideCanvas.empty() ? effectiveMapName : overrideCanvas);
        gameModeTag = 3;
    }

    // Diagnostic: force the engine mode tag (1 campaign, 3 forge, 5 campaign-like, 1035 firefight).
    if (const char* forcedTag = std::getenv("NOBLE_GAMEMODE_TAG")) {
        gameModeTag = std::atoi(forcedTag);
        NOBLE_WARN("BlamEngine: engine mode tag forced to {}", gameModeTag);
    }

    // A mod map built on a Firefight canvas has to be presented to the engine as a multiplayer map, whichever way the
    // launch came in (command line or the pre-game lobby): the engine reads the type from
    // mapsinfo<scenario>.mapinfo before the map loads and refuses a Firefight entry for a multiplayer launch.
    if (noble::VFS::HasActiveMod() && (gameModeTag == 3 || gameModeTag == 2)) {
        noble::VFS::EnsureMultiplayerMapInfo(effectiveMapName);
    }

    // Campaign & Firefight co-op: keep joining peers even when the engine decides their progress is incompatible
    // (see LobbyPatches::Hook_BootPeer) - otherwise the mission ends with "Error hosting co-op".
    noble::LobbyPatches::SetCampaignCoop(gameModeTag == 1 || gameModeTag == 1035 || gameModeTag == 5);
    {
        // Noble Actual and Lone Wolf are the only stock missions with no gameplay. Their "mission end"
        // happens right after the cutscene, so do not let the premature-end guard swallow it.
        std::string lowerMap = effectiveMapName;
        for (char& ch : lowerMap) ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
        const bool cinematicOnly = (gameModeTag == 1 || gameModeTag == 5) &&
                                   (lowerMap == "m05" || lowerMap == "m70_bonus");
        noble::EnginePatches::SetCinematicOnlyMission(cinematicOnly);
        if (cinematicOnly)
            NOBLE_INFO("BlamEngine: '{}' is a cinematic-only mission; allowing its mission end", effectiveMapName);
    }

    // Career pays by base type, so modded maps/variants earn exactly what their stock type earns.
    if (gameModeTag != 0 && !IsSavedFilmPlayback()) Career::BeginMatch(Career::BaseTypeFromEngineMode(gameModeTag), effectiveMapName, difficulty, skulls);

    auto launchBlock = BuildLaunchBlock(mapId, gameModeTag, difficulty, skulls, customMvarPath, customVariantPath, connectAddress);
    // Campaign/Firefight co-op: the launch block declares how many players this session has (0xE8).
    // A host launched with known guests must declare all of them, otherwise the engine only creates the
    // local player and the co-op state machine fails the mission about 60 s in (GameResults, channel close).
    // Removed once in e817d2f and re-proven by test; do not reduce this to 1 for a co-op host.
    if (gameModeTag == 1 || gameModeTag == 1035 || gameModeTag == 5) {
        int declaredPlayers = 1;
        const bool hostWithGuests = connectAddress.empty() && expectedPeers > 0;
        if (hostWithGuests) declaredPlayers = 1 + expectedPeers;
        // Exactly the players this launch is for. Declaring more than are present leaves the engine waiting at load
        // state 8 for participants that never arrive (a solo launch with 4 declared hangs on the loading screen), and
        // the lobby already passes the full roster, so a three-player lobby declares three and starts.
        // 5+ co-op: the session has to be created with room for the extra players, otherwise a player joining over
        // the launch size ends the mission instead of being seated. Only widen when the roster is already over the
        // engine's stock 4-player co-op size, so the proven 1-4 player launches keep their exact block.
        if (hostWithGuests && declaredPlayers > 4) {
            constexpr int kCoopCapacity = 8;
            declaredPlayers = std::min(16, std::max(declaredPlayers, kCoopCapacity));
            NOBLE_INFO("BlamEngine: co-op roster is over 4; creating the session with capacity {} (0xE8)", declaredPlayers);
        }
        launchBlock[0xE8] = static_cast<uint8_t>(declaredPlayers);
        NOBLE_INFO("BlamEngine: launch block declares {} player(s) (0xE8), {} expected peer(s), mode {}",
                   declaredPlayers, expectedPeers, gameModeTag);
    }

    // Request corresponding wireframe loading screen .dots
    LoadScreen::Get().RequestMapDots(effectiveMapName, ReachHost::Get().GetDataDir());

    if (m_fnStart) {
        void* hostPtr = ReachHost::Get().GetInterface();
        // Campaign missions are chosen by the launch block, not by the requested map: the block carries the map id at
        // 0x10/0x14 (179 for m10, 180 for the next mission). A mod map served into a campaign slot therefore never
        // loads; retarget the block at the override's own map id so the campaign engine asks for that map instead.
        if (const char* retarget = std::getenv("NOBLE_BLOCK_IDPATCH")) {
            (void)retarget;
            const std::string canvas = noble::VFS::MapOverrideCanvas();
            const uint32_t canvasId = canvas.empty() ? 0xFFFFFFFFu : ResolveMapId(canvas);
            NOBLE_INFO("BlamEngine: block retarget check: canvas='{}' id={} launch={} mapId={}", canvas, canvasId, mapName,
                       mapId);
            if (canvasId != 0xFFFFFFFFu && canvasId != mapId) {
                std::memcpy(launchBlock.data() + 0x10, &canvasId, sizeof(canvasId));
                std::memcpy(launchBlock.data() + 0x14, &canvasId, sizeof(canvasId));
                noble::VFS::SetMapOverride(canvas, noble::VFS::MapOverrideFile());
                NOBLE_WARN("BlamEngine: campaign launch block retargeted from map {} to '{}' ({})", mapId, canvas,
                           canvasId);
            }
        }

        NOBLE_INFO("Invoking c_game_engine Start(engine, host:0x{:X}, launch_block) [Map: {}]...",
            reinterpret_cast<uintptr_t>(hostPtr), mapName);
        int32_t startStatus = m_fnStart(m_engineInstance, hostPtr, launchBlock.data());
        NOBLE_INFO("c_game_engine Start returned: 0x{:X}", startStatus);
    }

    const bool isGuest = !connectAddress.empty();
    if (!isGuest) {
        const bool isOnlineHost = noble::NetworkTransport::Get().IsActive() && noble::NetworkTransport::Get().IsHost();
        const bool settleLonger = (gameModeTag == 1 || gameModeTag == 3 || gameModeTag == 1035); // campaign co-op / Forge / Firefight
        const int peersToWaitFor = (isOnlineHost && expectedPeers > 0) ? expectedPeers : 0;

        std::thread([this, isOnlineHost, peersToWaitFor, settleLonger, gameModeTag]() {
            // Post Command 10 (Event: Start) promptly so the engine loads the map tags and opens the game session.
            const int startDelay = (peersToWaitFor > 0) ? 600 : 300;
            std::this_thread::sleep_for(std::chrono::milliseconds(startDelay));

            if (m_engineInstance && m_fnProcessEvent) {
                NOBLE_INFO("BlamEngine: Posting Command 10 (Event: Start) via vtbl+0x18 after {}ms delay...", startDelay);
                int32_t status10 = m_fnProcessEvent(m_engineInstance, 10, 0);
                NOBLE_INFO("BlamEngine: PostCommand(10) returned: 0x{:X}", status10);

                // For online co-op / multiplayer / Forge with expected peers:
                // Wait for remote players to finish loading and join Blam! membership (msg 27)
                // BEFORE posting Command 11 (simulation start) so state replication matches all clients.
                if (peersToWaitFor > 0) {
                    const size_t targetPeers = static_cast<size_t>(peersToWaitFor);
                    const ULONGLONG waitStart = GetTickCount64();
                    const ULONGLONG deadline = waitStart + 45000;
                    ULONGLONG lastLog = 0;
                    s_waitingRemoteExpected.store(static_cast<int>(targetPeers), std::memory_order_relaxed);
                    s_waitingRemoteJoined.store(0, std::memory_order_relaxed);
                    s_waitingForRemotePlayers.store(true, std::memory_order_relaxed);
                    NOBLE_INFO("BlamEngine: waiting for {} remote player(s) to finish loading and join membership before starting simulation...",
                               targetPeers);
                    while (GetTickCount64() < deadline) {
                        if (!m_isGameRunning || !m_engineInstance || !m_fnProcessEvent) {
                            s_waitingForRemotePlayers.store(false, std::memory_order_relaxed);
                            return;
                        }
                        const size_t playersJoined = LobbyPatches::GetRemotePlayerAddCount();
                        const size_t channelPeers = NetworkTransport::Get().GetGamePeerCount();
                        s_waitingRemoteJoined.store(static_cast<int>(std::max(playersJoined, channelPeers)),
                                                    std::memory_order_relaxed);
                        const ULONGLONG now = GetTickCount64();
                        const ULONGLONG elapsed = now - waitStart;
                        if (playersJoined >= targetPeers || (elapsed >= 8000ULL && channelPeers >= targetPeers)) {
                            const int settleMs = settleLonger ? 1200 : 500;
                            NOBLE_INFO("BlamEngine: {}/{} remote player(s) in session (membership: {}, engine peers: {}, elapsed {} ms); settling {} ms for world replication",
                                       std::max(playersJoined, channelPeers), targetPeers, playersJoined, channelPeers, elapsed, settleMs);
                            std::this_thread::sleep_for(std::chrono::milliseconds(settleMs));
                            break;
                        }
                        if (lastLog == 0 || now - lastLog >= 3000) {
                            lastLog = now;
                            NOBLE_INFO("BlamEngine: membership wait: {}/{} player(s) in session (engine peers: {})",
                                       playersJoined, targetPeers, channelPeers);
                        }
                        std::this_thread::sleep_for(std::chrono::milliseconds(100));
                    }
                    if (LobbyPatches::GetRemotePlayerAddCount() < targetPeers && NetworkTransport::Get().GetGamePeerCount() < targetPeers) {
                        NOBLE_WARN("BlamEngine: membership wait timed out with {}/{} player(s); starting simulation anyway",
                                   LobbyPatches::GetRemotePlayerAddCount(), targetPeers);
                    }
                    s_waitingForRemotePlayers.store(false, std::memory_order_relaxed);
                } else {
                    const int simDelay = 200;
                    std::this_thread::sleep_for(std::chrono::milliseconds(simDelay));
                }

                if (!m_isGameRunning || !m_engineInstance || !m_fnProcessEvent) return;

                NOBLE_INFO("BlamEngine: Posting Command 11 via vtbl+0x18...");
                int32_t status11 = m_fnProcessEvent(m_engineInstance, 11, 0);
                NOBLE_INFO("BlamEngine: PostCommand(11) returned: 0x{:X}", status11);
            }
        }).detach();
    } else {
        if (gameModeTag == 1 || BlamEngine::GuestPostStart()) {
            NOBLE_INFO("BlamEngine: [GUEST Campaign] Starting campaign co-op background sequence (dispatching Command 10/11)...");
            std::thread([this]() {
                // Post Command 10 promptly so the engine loads scenario map tags and initializes the network session
                std::this_thread::sleep_for(std::chrono::milliseconds(500));
                if (!m_isGameRunning || !m_engineInstance || !m_fnProcessEvent) return;

                int32_t launchState = -1;
                int32_t mapState = -1;
                if (ReadEngineLaunchMapStateSEH(m_hModule, &launchState, &mapState) &&
                    (launchState >= 11 || mapState >= 2)) {
                    NOBLE_INFO("BlamEngine: [GUEST Campaign] Engine already running via host session sync (launch={}, map={}); skipping manual PostCommand(10/11).",
                               launchState, mapState);
                    return;
                }

                NOBLE_INFO("BlamEngine: [GUEST Campaign] Posting Command 10 (Event: Start) via vtbl+0x18...");
                int32_t status10 = m_fnProcessEvent(m_engineInstance, 10, 0);
                NOBLE_INFO("BlamEngine: [GUEST Campaign] PostCommand(10) returned 0x{:X}", status10);

                // Wait for map loading to finish OR for host session sync to start simulation directly.
                // Note: On Campaign maps, LoadProgress can remain at 5.0% while haloreach.dll advances
                // launchState -> 11 and mapState -> 2 ("running") via host session parameters. Re-posting
                // Command 11 after the 30s timeout while Co-Op simulation is already running disrupts gameplay.
                const ULONGLONG deadline = GetTickCount64() + 30000;
                while (GetTickCount64() < deadline) {
                    if (!m_isGameRunning || !m_engineInstance || !m_fnProcessEvent) return;
                    if (ReadEngineLaunchMapStateSEH(m_hModule, &launchState, &mapState) &&
                        (launchState >= 11 || mapState >= 2)) {
                        NOBLE_INFO("BlamEngine: [GUEST Campaign] Simulation started via host session sync (launch={}, map={}); skipping manual PostCommand(11).",
                                   launchState, mapState);
                        return;
                    }
                    if (LoadScreen::Get().GetLoadProgress() >= 0.95f) {
                        NOBLE_INFO("BlamEngine: [GUEST Campaign] Map load reached {:.1f}%; settling for host session sync",
                                   LoadScreen::Get().GetLoadProgress() * 100.0f);
                        break;
                    }
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                }

                // Join handshake: wait for the host's session_sync (repeated every couple of seconds while its
                // simulation runs, so a guest joining mid-match receives it promptly). Falls back to a short settle
                // if the host never reports - older hosts, or a LAN / direct connect join without the relay.
                {
                    const bool isDirectConnect = (wcsstr(GetCommandLineW(), L"-connect") != nullptr) || !noble::NetworkTransport::Get().IsRelayConnected();
                    const ULONGLONG syncDeadline = GetTickCount64() + (isDirectConnect ? 500 : 20000);
                    while (GetTickCount64() < syncDeadline &&
                           !noble::NetworkTransport::Get().HostSimulationRunning()) {
                        std::this_thread::sleep_for(std::chrono::milliseconds(50));
                    }
                    if (noble::NetworkTransport::Get().HostSimulationRunning()) {
                        NOBLE_INFO("BlamEngine: [GUEST Campaign] host reports its simulation running (frame {}); starting",
                                   noble::NetworkTransport::Get().HostSimulationFrame());
                    } else {
                        NOBLE_INFO("BlamEngine: [GUEST Campaign] fast settle active (direct connect/local); proceeding with simulation start");
                    }
                }

                if (!m_isGameRunning || !m_engineInstance || !m_fnProcessEvent) return;
                if (ReadEngineLaunchMapStateSEH(m_hModule, &launchState, &mapState) &&
                    (launchState >= 11 || mapState >= 2)) {
                    NOBLE_INFO("BlamEngine: [GUEST Campaign] Simulation already active after settle (launch={}, map={}); skipping manual PostCommand(11).",
                               launchState, mapState);
                    return;
                }

                NOBLE_INFO("BlamEngine: [GUEST Campaign] Posting Command 11 (Simulation Start) via vtbl+0x18...");
                int32_t status11 = m_fnProcessEvent(m_engineInstance, 11, 0);
                NOBLE_INFO("BlamEngine: [GUEST Campaign] PostCommand(11) returned 0x{:X}", status11);
            }).detach();
        } else {
            NOBLE_INFO("BlamEngine: [GUEST] Remote join active - skipping manual PostCommand(10/11); waiting for host session sync.");
        }
    }

    return true;
}

void BlamEngine::Update(float deltaTime) {
    (void)deltaTime;
}

void BlamEngine::Render() {
}


bool BlamEngine::DumpGameVariant(const std::string& path) {
    std::ifstream in(std::filesystem::path(path), std::ios::binary);
    if (!in || !m_hModule) { NOBLE_WARN("DumpGameVariant: cannot open {} or engine not loaded", path); return false; }
    std::vector<uint8_t> file((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    if (file.empty()) return false;
    std::vector<uint8_t> decoded(0xFC04);
    char status[64]{};
    const bool ok = SafeCallGameVariantDecode(m_hModule, file.data(), decoded.data(), status);
    const std::string outPath = (std::filesystem::current_path() / (std::filesystem::path(path).filename().string() + ".decoded.bin")).string();
    std::ofstream out(std::filesystem::path(outPath), std::ios::binary);
    if (out) out.write(reinterpret_cast<const char*>(decoded.data()), static_cast<std::streamsize>(decoded.size()));
    NOBLE_INFO("DumpGameVariant: {} -> {} (ok={}, status='{}', {} bytes in)", path, outPath, ok, status, file.size());
    if (ok) {
        uint32_t first = 0;
        std::memcpy(&first, decoded.data(), sizeof(first));
        NOBLE_INFO("DumpGameVariant: first u32 = 0x{:08X}", first);
        if (decoded.size() > 0x300) {
            const auto byte = [&](size_t offset) { return static_cast<unsigned>(decoded[offset]); };
            NOBLE_INFO("Variant fields: roundsToWin {} time {} roundLimit {} sudden {} grace {}",
                       byte(0x2BD), byte(0x2BE), byte(0x2BF), byte(0x2C0), byte(0x2C2));
            NOBLE_INFO("Variant respawn: flags {} lives {} teamLives {} time {} suicide {} betrayal {} growth {} cam {} traits {}",
                       byte(0x2C4), byte(0x2C5), byte(0x2C6), byte(0x2C7), byte(0x2C8), byte(0x2C9),
                       byte(0x2CA), byte(0x2CB), byte(0x2CC));
            NOBLE_INFO("Variant traits: health {} shield {} speed {} gravity {} radar {} weapon {} ability {} mapFlags {} social {}",
                       byte(0x2D0), byte(0x2D3), byte(0x2E8), byte(0x2E9), byte(0x2F8), byte(0x2E5),
                       byte(0x2E7), byte(0x300), byte(0x2FC));
            if (decoded.size() > 0xF842) {
                uint16_t score = 0;
                std::memcpy(&score, decoded.data() + 0xF840, sizeof(score));
                NOBLE_INFO("Variant score to win: {}", score);
            }
        }
    }
    return ok;
}

void BlamEngine::Shutdown() {
    if (s_active == this) s_active = nullptr;
    s_lastLiveVariantSource.store(nullptr, std::memory_order_release);
    s_lastCopyVariantSource.store(nullptr, std::memory_order_release);
    if (!m_isInitialized) return;

    NOBLE_INFO("Shutting down Blam! engine network transport...");
    NetworkTransport::Get().Shutdown();
    // Do not call m_fnShutdown(m_engineInstance) (vtable[0], c_game_engine's scalar deleting destructor)
    // while haloreach.dll's engine thread is still running (such as when the menu process exits to launch
    // a match): freeing c_game_engine's critical sections and heap while the engine thread is active
    // caused the ntdll.dll access violation on exit. Process termination reclaims all engine memory cleanly.
    m_engineInstance = nullptr;
    m_isInitialized = false;
}

} // namespace noble
