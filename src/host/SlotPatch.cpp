#include "SlotPatch.hpp"

#include "common/Logger.hpp"

#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <iterator>
#include <shellapi.h>

namespace noble::slots {

namespace {

struct Site {
    uint32_t rva;
    uint8_t original[7];   // imul r64, r64, imm32
    uint32_t caveLength;
    uint8_t cave[48];
    uint32_t constantAt;   // imm32 of `add dst, C`
    uint32_t backAt1, backAt2;   // rel32 of the two `jmp back`
    uint32_t record;       // 0xB0 player, 0x158 peer
    uint32_t threshold;    // first slot that does not exist in the original object
};

// Pointer-stepping loops (`add ptr, R` -> cave that moves ptr into the shadow object at the threshold slot).
struct Step {
    uint32_t rva;
    uint8_t original[7];   // add r64, imm32
    uint32_t caveLength;
    uint8_t cave[32];
    uint32_t constantAt;   // imm32 of `add ptr, C`
    uint32_t backAt;       // rel32 of `jmp back`
    uint32_t record;
    uint32_t threshold;
};

// Loop bounds / masks changed only in 32-player mode.
struct Bound {
    uint32_t rva;
    uint32_t length;
    uint8_t original[8];
    uint8_t patched[8];
};

const Site kSites[] = {
#include "SlotPatchTable.inc"
};

const Step kSteps[] = {
#include "SlotStepTable.inc"
};

const Bound kBounds[] = {
#include "SlotBoundsTable.inc"
};

// C12-L step 3, bounds review #5/#7: 0x390DB0 keeps 16 qword player-name pointers at rbp+0x150, directly below a
// 16-word name buffer and the GS cookie, so the table cannot grow in place. The frame grows by 0x100 (rbp moves
// with rsp, so every other local keeps its offset and the cookie stays at frame+0x2F0) and the table moves to the
// new space at rbp+0x1F8 (32 x 8 = 0x100 of the 0x108 free bytes). Its only consumer, 0x38DA74 (called twice from
// 0x390DB0), scans 32 entries. The loop bounds inside 0x390DB0 stay 16 until its six untraced imul sites are caved.
const Bound kFrame390[] = {
    { 0x390DC8, 7, { 0x48, 0x8D, 0xA8, 0xD8, 0xFD, 0xFF, 0xFF }, { 0x48, 0x8D, 0xA8, 0xD8, 0xFC, 0xFF, 0xFF } },  // lea rbp,[rax-0x228] -> -0x328
    { 0x390DCF, 7, { 0x48, 0x81, 0xEC, 0x00, 0x03, 0x00, 0x00 }, { 0x48, 0x81, 0xEC, 0x00, 0x04, 0x00, 0x00 } },  // sub rsp,0x300 -> 0x400
    { 0x390EA9, 7, { 0x48, 0x8D, 0x8D, 0x50, 0x01, 0x00, 0x00 }, { 0x48, 0x8D, 0x8D, 0xF8, 0x01, 0x00, 0x00 } },  // lea rcx,[rbp+0x150] (memset)
    { 0x390EB0, 6, { 0x41, 0xB8, 0x80, 0x00, 0x00, 0x00 }, { 0x41, 0xB8, 0x00, 0x01, 0x00, 0x00 } },              // mov r8d,0x80 -> 0x100
    { 0x390F55, 8, { 0x48, 0x89, 0x84, 0xF5, 0x50, 0x01, 0x00, 0x00 }, { 0x48, 0x89, 0x84, 0xF5, 0xF8, 0x01, 0x00, 0x00 } },
    { 0x39107B, 8, { 0x4A, 0x8B, 0x8C, 0xD5, 0x50, 0x01, 0x00, 0x00 }, { 0x4A, 0x8B, 0x8C, 0xD5, 0xF8, 0x01, 0x00, 0x00 } },
    { 0x391203, 7, { 0x4C, 0x8D, 0x8D, 0x50, 0x01, 0x00, 0x00 }, { 0x4C, 0x8D, 0x8D, 0xF8, 0x01, 0x00, 0x00 } },  // lea r9 -> 0x38DA74
    { 0x391231, 8, { 0x4C, 0x89, 0xA4, 0xCD, 0x50, 0x01, 0x00, 0x00 }, { 0x4C, 0x89, 0xA4, 0xCD, 0xF8, 0x01, 0x00, 0x00 } },
    { 0x391332, 8, { 0x4A, 0x8B, 0x94, 0xD5, 0x50, 0x01, 0x00, 0x00 }, { 0x4A, 0x8B, 0x94, 0xD5, 0xF8, 0x01, 0x00, 0x00 } },
    { 0x3913FB, 7, { 0x4C, 0x8D, 0x8D, 0x50, 0x01, 0x00, 0x00 }, { 0x4C, 0x8D, 0x8D, 0xF8, 0x01, 0x00, 0x00 } },  // lea r9 -> 0x38DA74
    { 0x39140A, 8, { 0x4A, 0x89, 0xBC, 0xED, 0x50, 0x01, 0x00, 0x00 }, { 0x4A, 0x89, 0xBC, 0xED, 0xF8, 0x01, 0x00, 0x00 } },
    { 0x391586, 8, { 0x4C, 0x8D, 0x9C, 0x24, 0x00, 0x03, 0x00, 0x00 }, { 0x4C, 0x8D, 0x9C, 0x24, 0x00, 0x04, 0x00, 0x00 } },  // epilogue lea r11,[rsp+0x300]
    { 0x38DAE6, 4, { 0x48, 0x83, 0xF9, 0x10 }, { 0x48, 0x83, 0xF9, 0x20 } },  // 0x38DA74: cmp rcx,0x10 -> 0x20 (review #5)
};
// 0x390DB0's unwind info (shared with 0x35B1E4, so it is copied, not edited): UWOP_ALLOC_LARGE 0x300 and the
// rbx/rsi/rdi save slots 0x338/0x340/0x348 move by 0x100. GS handler data (cookie at frame+0x2F0) is unchanged.
constexpr uint32_t kFrame390Begin = 0x390DB0, kFrame390End = 0x3915A7;
constexpr uint8_t kFrame390Unwind[40] = {
    0x19, 0x37, 0x0D, 0x00, 0x26, 0x74, 0x69, 0x00, 0x26, 0x64, 0x68, 0x00, 0x26, 0x34, 0x67, 0x00,
    0x26, 0x01, 0x60, 0x00, 0x18, 0xF0, 0x16, 0xE0, 0x14, 0xD0, 0x12, 0xC0, 0x10, 0x50, 0x00, 0x00,
    0x0E, 0x37, 0x79, 0x00, 0xF0, 0x02, 0x00, 0x00 };
constexpr struct { uint8_t at, from, to; } kFrame390UnwindEdits[] = {
    { 6, 0x69, 0x89 }, { 10, 0x68, 0x88 }, { 14, 0x67, 0x87 }, { 18, 0x60, 0x80 } };

// Whole-block copies and clears (scope count in docs/limits-players-loops.md). Each `call memcpy/memset` below is
// redirected to a Noble wrapper that makes the original call and then mirrors it onto the shadow slots when the
// destination is a static session: clear -> clear the covered shadow arrays; static -> static copy -> copy the
// source session's shadow arrays; copy from any other block (#1, decoded from the network) -> clear the
// destination's shadow arrays and log it. A destination outside the static sessions is left alone.
constexpr uint32_t kMemcpyRva = 0x794276, kMemsetRva = 0x78932A;
struct CallSite { uint32_t rva; uint32_t target; const char* what; };
constexpr CallSite kCopySites[] = {
    { 0x39C4D5, kMemcpyRva, "#1 0x39C484 memcpy 0x4710 (caller block -> session+0x30)" },
    { 0x39C4F3, kMemsetRva, "#2 0x39C484 memset 0x2210 (half B)" },
    { 0x39C71A, kMemsetRva, "#3 0x39C6B4 memset 0x4710 (membership reset)" },
    { 0x39C7C4, kMemsetRva, "#4 0x39C7AC memset 0x220C (half-B reset)" },
    { 0x396CBE, kMemcpyRva, "#7 0x396828 memcpy 0x2210 (half A -> half B)" },
    { 0x16FEA9, kMemsetRva, "#8 0x16FE2C memset 0x16D8 (peer array)" },
};

constexpr uintptr_t kSessionsRva = 0x2B07418;   // 4 static session membership objects
constexpr uintptr_t kSessionStride = 0x61668;
constexpr int kSessions = 4;

// Record constructors the session constructor (0x3CF864) runs on every inline slot; run them on the shadow slots
// too so overflow records start in the same state (ids -1, cleared names) instead of all zero.
constexpr uintptr_t kPeerCtorRva = 0x16FC60;     // this = peer record (0x158)
constexpr uintptr_t kPlayerCtorRva = 0x3CFA38;   // this = player record (0xB0)
struct Array { uint32_t offset; uint32_t record; uint32_t firstShadowSlot; bool peer; };
constexpr Array kArrays[] = {
    { 0x0060, 0x158, 17, true },  { 0x1740, 0xB0, 16, false },   // block A
    { 0x2270, 0x158, 17, true },  { 0x3950, 0xB0, 16, false },   // block B
};
constexpr uint32_t kSlots32 = 32;   // players and peers in 32-player mode (the peer-valid mask is a DWORD)

// Set once by Install (32-player mode) before any call site is redirected.
uintptr_t gSessions = 0;
uint8_t* gShadow = nullptr;

// Session index and offset of `p` inside the static sessions, or -1.
int StaticSession(const void* p, uint32_t& offset) {
    const auto a = reinterpret_cast<uintptr_t>(p);
    if (!gSessions || a < gSessions || a >= gSessions + kSessions * kSessionStride) return -1;
    offset = static_cast<uint32_t>((a - gSessions) % kSessionStride);
    return static_cast<int>((a - gSessions) / kSessionStride);
}

uint8_t* ShadowSlots(int k, const Array& a) { return gShadow + k * kSessionStride + a.offset; }
size_t ShadowBytes(const Array& a) { return static_cast<size_t>(kSlots32 - a.firstShadowSlot) * a.record; }

// Inline arrays fully inside [offset, offset + n) - only those were really copied or cleared.
template <typename F> void ForCoveredArrays(uint32_t offset, size_t n, F f) {
    for (const Array& a : kArrays)
        if (a.offset >= offset && a.offset + static_cast<size_t>(a.firstShadowSlot) * a.record <= offset + n) f(a);
}

void MirrorClear(void* dst, int value, size_t n) {
    uint32_t o = 0;
    const int k = StaticSession(dst, o);
    if (k < 0) return;
    ForCoveredArrays(o, n, [&](const Array& a) { std::memset(ShadowSlots(k, a), value, ShadowBytes(a)); });
}

void MirrorCopy(void* dst, const void* src, size_t n) {
    uint32_t od = 0, os = 0;
    const int kd = StaticSession(dst, od);
    if (kd < 0) return;
    const int ks = StaticSession(src, os);
    bool cleared = false;
    ForCoveredArrays(od, n, [&](const Array& a) {
        const uint32_t from = a.offset - od + os;   // the same array in the source block
        const Array* sa = nullptr;
        for (const Array& b : kArrays) if (b.offset == from && b.record == a.record) sa = &b;
        if (ks >= 0 && sa) std::memmove(ShadowSlots(kd, a), ShadowSlots(ks, *sa), ShadowBytes(a));
        else { std::memset(ShadowSlots(kd, a), 0, ShadowBytes(a)); cleared = true; }
    });
    // Network-decoded blocks (#1) carry no shadow slots yet: drop slots 16..31 rather than keep stale players.
    static LONG logged = 0;
    if (cleared && InterlockedIncrement(&logged) <= 20)
        NOBLE_WARN("SlotPatch: block copied into session {} from a non-session block; shadow slots 16-31 cleared "
                   "(network encoding not widened yet)", kd);
}

void* __cdecl MemcpyMirror(void* dst, const void* src, size_t n) {
    void* r = std::memcpy(dst, src, n);
    MirrorCopy(dst, src, n);
    return r;
}

void* __cdecl MemsetMirror(void* dst, int value, size_t n) {
    void* r = std::memset(dst, value, n);
    MirrorClear(dst, value, n);
    return r;
}

// Memory within +-2 GB of `target`, so rel32 jumps and the imm32 shadow constant reach it.
void* AllocateNear(uintptr_t target, size_t size, DWORD protect) {
    SYSTEM_INFO si{};
    GetSystemInfo(&si);
    const uintptr_t step = si.dwAllocationGranularity;
    for (uintptr_t delta = step; delta < 0x7FFF0000ull; delta += step) {
        const uintptr_t candidates[2] = { target + delta, target > delta ? target - delta : 0 };
        for (const uintptr_t candidate : candidates) {
            if (!candidate) continue;
            if (void* p = VirtualAlloc(reinterpret_cast<void*>(candidate & ~(step - 1)), size, MEM_RESERVE | MEM_COMMIT, protect))
                return p;
        }
    }
    return nullptr;
}

void WriteCode(uintptr_t at, const void* bytes, size_t n);
void Put32(uint8_t* p, int64_t v);

// UNWIND_INFO is addressed by a 32-bit RVA from the image base, so a copy has to live above the image, within 4 GB.
void* AllocateAbove(uintptr_t base, size_t size) {
    const auto* nt = reinterpret_cast<const IMAGE_NT_HEADERS*>(base + reinterpret_cast<const IMAGE_DOS_HEADER*>(base)->e_lfanew);
    SYSTEM_INFO si{};
    GetSystemInfo(&si);
    const uintptr_t step = si.dwAllocationGranularity;
    const uintptr_t first = (base + nt->OptionalHeader.SizeOfImage + step - 1) & ~(step - 1);
    for (uintptr_t at = first; at + size - base < 0xFFFF0000ull; at += step)
        if (void* p = VirtualAlloc(reinterpret_cast<void*>(at), size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE)) return p;
    return nullptr;
}

// The loaded engine's own function-table entry for 0x390DB0 (V16/Full carry their own .pdata, so it is looked up,
// not taken from a stock file offset). nullptr when it is not the expected entry and unwind block.
RUNTIME_FUNCTION* Frame390Entry(uintptr_t base) {
    DWORD64 imageBase = 0;
    auto* rf = RtlLookupFunctionEntry(base + kFrame390Begin, &imageBase, nullptr);
    if (!rf || imageBase != base || rf->BeginAddress != kFrame390Begin || rf->EndAddress != kFrame390End) return nullptr;
    if (std::memcmp(reinterpret_cast<void*>(base + rf->UnwindData), kFrame390Unwind, sizeof(kFrame390Unwind)) != 0) return nullptr;
    return rf;
}

bool Frame390Matches(uintptr_t base) {
    for (const Bound& b : kFrame390)
        if (std::memcmp(reinterpret_cast<void*>(base + b.rva), b.original, b.length) != 0) {
            NOBLE_WARN("SlotPatch: 0x390DB0 frame code at +0x{:X} does not match", b.rva);
            return false;
        }
    if (!Frame390Entry(base)) { NOBLE_WARN("SlotPatch: 0x390DB0 unwind entry does not match"); return false; }
    return true;
}

// Call only after Frame390Matches. The unwind copy goes in first, so a stack walk never sees the grown code with
// the old frame size (Install runs before the engine's first frame, so 0x390DB0 is not on any stack).
bool ApplyFrame390(uintptr_t base, void* unwindCopy) {
    RUNTIME_FUNCTION* rf = Frame390Entry(base);
    if (!rf || !unwindCopy) return false;
    auto* u = static_cast<uint8_t*>(unwindCopy);
    std::memcpy(u, kFrame390Unwind, sizeof(kFrame390Unwind));
    for (const auto& e : kFrame390UnwindEdits) u[e.at] = e.to;
    DWORD old = 0;
    VirtualProtect(u, sizeof(kFrame390Unwind), PAGE_READONLY, &old);
    const auto rva = static_cast<DWORD>(reinterpret_cast<uintptr_t>(u) - base);
    WriteCode(reinterpret_cast<uintptr_t>(&rf->UnwindData), &rva, sizeof(rva));
    for (const Bound& b : kFrame390) WriteCode(base + b.rva, b.patched, b.length);
    return true;
}

// `call rel32` at the site must reach the stock memcpy/memset.
bool CopySitesMatch(uintptr_t base) {
    for (const CallSite& s : kCopySites) {
        const auto* p = reinterpret_cast<const uint8_t*>(base + s.rva);
        int32_t rel = 0;
        std::memcpy(&rel, p + 1, 4);
        if (p[0] != 0xE8 || s.rva + 5 + rel != s.target) {
            NOBLE_WARN("SlotPatch: block copy call at +0x{:X} does not match ({})", s.rva, s.what);
            return false;
        }
    }
    return true;
}

// Two `jmp [rip+0]; dq target` stubs near the engine, then every site's rel32 points at its stub.
bool ApplyCopySites(uintptr_t base, uint8_t* stubs) {
    if (!stubs) return false;
    const void* targets[2] = { reinterpret_cast<const void*>(&MemcpyMirror), reinterpret_cast<const void*>(&MemsetMirror) };
    for (int i = 0; i < 2; ++i) {
        uint8_t* s = stubs + i * 16;
        const uint8_t jmp[6] = { 0xFF, 0x25, 0, 0, 0, 0 };
        std::memcpy(s, jmp, 6);
        std::memcpy(s + 6, &targets[i], 8);
    }
    DWORD old = 0;
    VirtualProtect(stubs, 32, PAGE_EXECUTE_READ, &old);
    FlushInstructionCache(GetCurrentProcess(), stubs, 32);
    for (const CallSite& s : kCopySites) {
        const uint8_t* stub = stubs + (s.target == kMemcpyRva ? 0 : 16);
        uint8_t rel[4];
        Put32(rel, reinterpret_cast<intptr_t>(stub) - static_cast<intptr_t>(base + s.rva + 5));
        WriteCode(base + s.rva + 1, rel, 4);
    }
    return true;
}

bool Requested() {
    wchar_t env[8] = {};
    if (GetEnvironmentVariableW(L"NOBLE_SLOT_PATCH", env, 8) > 0 && env[0] == L'1') return true;
    int argc = 0;
    LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);
    bool on = false;
    for (int i = 1; argv && i < argc; ++i) if (_wcsicmp(argv[i], L"-slot-patch") == 0) on = true;
    if (argv) LocalFree(argv);
    if (on) SetEnvironmentVariableW(L"NOBLE_SLOT_PATCH", L"1");   // carried into the relaunched match process
    return on;
}

// -slot-players 32 (NOBLE_SLOT_PLAYERS=32): also install the step caves and raise the loop bounds.
// Dev only until the fixed tables listed in docs/limits-players-loops.md are handled.
bool Players32Requested() {
    wchar_t env[8] = {};
    if (GetEnvironmentVariableW(L"NOBLE_SLOT_PLAYERS", env, 8) > 0 && _wtoi(env) == 32) return true;
    int argc = 0;
    LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);
    bool on = false;
    for (int i = 1; argv && i + 1 < argc; ++i)
        if (_wcsicmp(argv[i], L"-slot-players") == 0 && _wtoi(argv[i + 1]) == 32) on = true;
    if (argv) LocalFree(argv);
    if (on) SetEnvironmentVariableW(L"NOBLE_SLOT_PLAYERS", L"32");
    return on;
}

void Put32(uint8_t* p, int64_t v) { const auto x = static_cast<int32_t>(v); std::memcpy(p, &x, 4); }

size_t Rounded(uint32_t n) { return (n + 15) & ~15u; }

void WriteCode(uintptr_t at, const void* bytes, size_t n) {
    DWORD old = 0;
    VirtualProtect(reinterpret_cast<void*>(at), n, PAGE_EXECUTE_READWRITE, &old);
    std::memcpy(reinterpret_cast<void*>(at), bytes, n);
    VirtualProtect(reinterpret_cast<void*>(at), n, old, &old);
}

void JumpTo(uintptr_t at, const uint8_t* cave) {
    uint8_t jump[7] = { 0xE9, 0, 0, 0, 0, 0x90, 0x90 };
    Put32(jump + 1, reinterpret_cast<intptr_t>(cave) - static_cast<intptr_t>(at + 5));
    WriteCode(at, jump, sizeof(jump));
}

void InitShadowRecords(uintptr_t base, uint8_t* shadow, bool players32) {
    using Ctor = void* (*)(void*);
    const auto peerCtor = reinterpret_cast<Ctor>(base + kPeerCtorRva);
    const auto playerCtor = reinterpret_cast<Ctor>(base + kPlayerCtorRva);
    for (int k = 0; k < kSessions; ++k) {
        uint8_t* object = shadow + k * kSessionStride;
        for (const Array& a : kArrays) {
            // Shadow slot j sits at the same array offset as slot 0 (C = delta - threshold * record).
            const uint32_t count = players32 ? kSlots32 - a.firstShadowSlot : 0;
            for (uint32_t j = 0; j < count; ++j) (a.peer ? peerCtor : playerCtor)(object + a.offset + j * a.record);
        }
    }
}

} // namespace

void Install(HMODULE engine, bool extendedProfile) {
    if (!engine || !Requested()) return;
    if (!extendedProfile) {
        NOBLE_WARN("SlotPatch: -slot-patch needs ENGINE LIMITS set to Extended; ignored");
        return;
    }
    const auto base = reinterpret_cast<uintptr_t>(engine);
    for (const Site& s : kSites) {   // all or nothing: any other engine build leaves every site untouched
        if (std::memcmp(reinterpret_cast<void*>(base + s.rva), s.original, sizeof(s.original)) != 0) {
            NOBLE_WARN("SlotPatch: engine code at +0x{:X} does not match; nothing patched", s.rva);
            return;
        }
    }
    bool players32 = Players32Requested();
    if (players32) {
        for (const Step& s : kSteps) {
            if (std::memcmp(reinterpret_cast<void*>(base + s.rva), s.original, sizeof(s.original)) != 0) {
                NOBLE_WARN("SlotPatch: step loop at +0x{:X} does not match; 32-player mode off", s.rva);
                players32 = false;
                break;
            }
        }
        for (const Bound& b : kBounds) {
            if (!players32) break;
            if (std::memcmp(reinterpret_cast<void*>(base + b.rva), b.original, b.length) != 0) {
                NOBLE_WARN("SlotPatch: loop bound at +0x{:X} does not match; 32-player mode off", b.rva);
                players32 = false;
            }
        }
        if (players32 && !Frame390Matches(base)) players32 = false;
        if (players32 && !CopySitesMatch(base)) players32 = false;
    }
    const uintptr_t sessions = base + kSessionsRva;
    // Shadow objects: same stride as the real ones, so shadow(obj) = obj + delta for all four.
    auto* shadow = static_cast<uint8_t*>(AllocateNear(sessions, kSessions * kSessionStride, PAGE_READWRITE));
    size_t caveBytes = 0;
    for (const Site& s : kSites) caveBytes += Rounded(s.caveLength);
    if (players32) for (const Step& s : kSteps) caveBytes += Rounded(s.caveLength);
    auto* caves = static_cast<uint8_t*>(AllocateNear(base, caveBytes, PAGE_READWRITE));
    void* unwindCopy = players32 ? AllocateAbove(base, sizeof(kFrame390Unwind)) : nullptr;
    auto* copyStubs = players32 ? static_cast<uint8_t*>(AllocateNear(base, 32, PAGE_READWRITE)) : nullptr;
    if (!shadow || !caves || (players32 && (!unwindCopy || !copyStubs))) {
        NOBLE_ERROR("SlotPatch: could not allocate memory near the engine; nothing patched");
        if (shadow) VirtualFree(shadow, 0, MEM_RELEASE);
        if (caves) VirtualFree(caves, 0, MEM_RELEASE);
        if (unwindCopy) VirtualFree(unwindCopy, 0, MEM_RELEASE);
        if (copyStubs) VirtualFree(copyStubs, 0, MEM_RELEASE);
        return;
    }
    InitShadowRecords(base, shadow, players32);
    const int64_t delta = reinterpret_cast<intptr_t>(shadow) - static_cast<intptr_t>(sessions);

    // Fill the caves (rel32 back jumps and the shadow constant), then make them executable.
    uint8_t* cave = caves;
    for (const Site& s : kSites) {
        std::memcpy(cave, s.cave, s.caveLength);
        Put32(cave + s.constantAt, delta - static_cast<int64_t>(s.threshold) * s.record);
        const uintptr_t back = base + s.rva + sizeof(s.original);
        Put32(cave + s.backAt1, static_cast<int64_t>(back) - reinterpret_cast<intptr_t>(cave + s.backAt1 + 4));
        Put32(cave + s.backAt2, static_cast<int64_t>(back) - reinterpret_cast<intptr_t>(cave + s.backAt2 + 4));
        cave += Rounded(s.caveLength);
    }
    uint8_t* const stepCaves = cave;
    if (players32) {
        for (const Step& s : kSteps) {
            std::memcpy(cave, s.cave, s.caveLength);
            Put32(cave + s.constantAt, delta - static_cast<int64_t>(s.threshold) * s.record);
            const uintptr_t back = base + s.rva + sizeof(s.original);
            Put32(cave + s.backAt, static_cast<int64_t>(back) - reinterpret_cast<intptr_t>(cave + s.backAt + 4));
            cave += Rounded(s.caveLength);
        }
    }
    DWORD old = 0;
    VirtualProtect(caves, caveBytes, PAGE_EXECUTE_READ, &old);
    FlushInstructionCache(GetCurrentProcess(), caves, caveBytes);

    // Point the engine at the caves.
    cave = caves;
    for (const Site& s : kSites) { JumpTo(base + s.rva, cave); cave += Rounded(s.caveLength); }
    if (players32) {
        cave = stepCaves;
        for (const Step& s : kSteps) { JumpTo(base + s.rva, cave); cave += Rounded(s.caveLength); }
        for (const Bound& b : kBounds) WriteCode(base + b.rva, b.patched, b.length);
        ApplyFrame390(base, unwindCopy);   // checked above (Frame390Matches); cannot fail here
        gSessions = sessions;
        gShadow = shadow;
        ApplyCopySites(base, copyStubs);   // checked above (CopySitesMatch)
    }
    FlushInstructionCache(GetCurrentProcess(), reinterpret_cast<void*>(base), 0x1000000);
    NOBLE_INFO("SlotPatch: {} slot address sites redirected (players >= 16, peers >= 17 -> shadow sessions at {:p}, "
               "delta 0x{:X})", std::size(kSites), static_cast<void*>(shadow), static_cast<uint64_t>(delta));
    if (players32)
        NOBLE_WARN("SlotPatch: 32-player mode (dev): {} step loops, {} loop bounds raised to 32, 0x390DB0 frame +0x100 "
                   "(32-entry name table), {} block copy/clear sites mirrored onto the shadow; other tables in docs/limits-players-loops.md are still 16/17 and the "
                   "players pool is not raised here", std::size(kSteps), std::size(kBounds), std::size(kCopySites));
    else
        NOBLE_INFO("SlotPatch: 16-player mode; inert until -slot-players 32");
}

} // namespace noble::slots
