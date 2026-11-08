#include "SpawnTest.hpp"

#if defined(NOBLE_DEV_BUILD)
#include "common/Logger.hpp"
#include "hooks/HookManager.hpp"

#include <algorithm>
#include <atomic>
#include <bit>
#include <cstdint>
#include <cmath>
#include <cstring>
#include <intrin.h>
#include <string>
#include <shellapi.h>
#endif

namespace noble::spawn {

#if defined(NOBLE_DEV_BUILD)
namespace {

// RVAs in haloreach.dll 1.3528.0.0. Identical in stock, V16 and V16+particles (checked byte for byte).
constexpr uintptr_t kTickRva = 0x61A38;            // per-player control tick, game thread
constexpr uintptr_t kPlacementNewRva = 0x49CC24;   // object_placement_data_new(placement*, tag, owner, ptr)
constexpr uintptr_t kObjectNewRva = 0x46D6E0;      // object_new(placement*) -> object handle or -1
constexpr uintptr_t kTlsIndexRva = 0xC17B18;       // TLS slot of the engine's game-state root
constexpr uintptr_t kAiPlaceRva = 0x6C2B10;        // ai_place(ai value, count (-1 = all), flag); bounds-checks squads
constexpr uintptr_t kScenarioRva = 0xC1A230;       // global: current scenario; +0x398 = squad count
constexpr size_t kPlacementSize = 0x1A8;
constexpr uint32_t kDataArraySignature = 0x64407440;

const uint8_t kTickHead[] = { 0x48, 0x8B, 0xC4, 0x55, 0x53, 0x56, 0x57, 0x41, 0x54, 0x41, 0x55, 0x41, 0x56, 0x41, 0x57, 0x48 };
const uint8_t kPlacementHead[] = { 0x48, 0x89, 0x5C, 0x24, 0x08, 0x48, 0x89, 0x6C, 0x24, 0x10, 0x48, 0x89, 0x74, 0x24, 0x18, 0x57 };
const uint8_t kAiPlaceHead[] = { 0x48, 0x89, 0x5C, 0x24, 0x18, 0x48, 0x89, 0x7C, 0x24, 0x20, 0x55, 0x48, 0x8B, 0xEC, 0x48, 0x83 };
const uint8_t kObjectNewHead[] = { 0x40, 0x53, 0x55, 0x56, 0x57, 0x41, 0x54, 0x41, 0x55, 0x41, 0x56, 0x41, 0x57, 0x48, 0x81, 0xEC };

using TickFn = uint64_t(__fastcall*)(int32_t, int32_t, int32_t, float, float);
using PlacementNewFn = void(__fastcall*)(void* placement, int32_t tagIndex, int32_t owner, void* extra);
using ObjectNewFn = int32_t(__fastcall*)(void* placement);
using AiPlaceFn = void(__fastcall*)(uint32_t aiValue, int32_t count, int32_t flag);

uintptr_t s_base = 0;
TickFn s_originalTick = nullptr;
PlacementNewFn s_placementNew = nullptr;
ObjectNewFn s_objectNew = nullptr;
AiPlaceFn s_aiPlace = nullptr;
std::atomic<int> s_aiPlaceRequests{ 0 };   // F7: place every squad in the map
bool s_f7Down = false;
ULONGLONG s_lastF7 = 0;

std::atomic<int> s_pending{ 0 };
// F8: a crowd of 128 copies of the player's biped (128-player render test), kept alive: the engine treats
// bodies without a player as corpses and cleans them up, so missing ones are put back twice a second.
constexpr int kCrowd = 128;
bool s_crowdOn = false;
bool s_f8Down = false;
ULONGLONG s_lastF8 = 0, s_lastCrowdCheck = 0;
int32_t s_bipedTag = -1;
float s_refForward[2] = { 1.0f, 0.0f };
int32_t s_crowdHandle[kCrowd];
float s_crowdPos[kCrowd][3];
int s_crowdRespawns = 0;
int s_autoCount = 0;          // -spawn-test <count>
// V16 has 16,384 scheduler jobs. Without the scheduler guard about 6,500 moving objects crash the engine
// (haloreach+0x68B21F); with it the ceiling is about 8,800, where one object update asks for more jobs than
// the whole scheduler holds. Stop short of that unless -spawn-max says otherwise.
int s_maxObjects = 8000;
ULONGLONG s_firstTick = 0;
bool s_autoQueued = false;
bool s_f9Down = false;
int s_spawned = 0, s_refused = 0, s_refusedInARow = 0;
int32_t s_refTag = -1;
float s_refPos[3] = {};

struct ObjectPool {
    uint8_t* header = nullptr;
    uint8_t* records = nullptr;
    const uint32_t* bitmap = nullptr;
    int32_t capacity = 0;
};

// The engine's object data array, read through its thread-local game-state root (game thread only).
bool GetObjectPool(ObjectPool& pool) {
    const uint32_t tlsIndex = *reinterpret_cast<const uint32_t*>(s_base + kTlsIndexRva);
    auto** tls = reinterpret_cast<uint8_t**>(__readgsqword(0x58));
    if (!tls || !tls[tlsIndex]) return false;
    uint8_t* header = *reinterpret_cast<uint8_t**>(tls[tlsIndex] + 0x10);
    if (!header || std::strncmp(reinterpret_cast<const char*>(header), "object", 32) != 0) return false;
    if (*reinterpret_cast<const uint32_t*>(header + 0x28) != kDataArraySignature) return false;
    pool.header = header;
    pool.capacity = *reinterpret_cast<const int32_t*>(header + 0x2C);
    pool.records = *reinterpret_cast<uint8_t**>(header + 0x50);
    pool.bitmap = reinterpret_cast<const uint32_t*>(header + *reinterpret_cast<const int64_t*>(header + 0x68));
    return pool.records != nullptr && pool.capacity > 0;
}

int UsedSlots(const ObjectPool& pool) {
    int used = 0;
    for (int i = 0; i < (pool.capacity + 31) / 32; ++i) used += std::popcount(pool.bitmap[i]);
    return used;
}

bool SlotUsed(const ObjectPool& pool, int i) { return (pool.bitmap[i >> 5] >> (i & 31)) & 1; }

// Picks what to copy and where: the tag of a weapon (type 2) if the match has one, otherwise a biped, and
// the position of a biped (the player) so the grid appears where you are. Objects without a world position
// (for example a weapon that only exists in the Forge palette) report (0, 0, 0) and are skipped for the position.
bool PickReference(const ObjectPool& pool) {
    auto at = [&](int i) -> const uint8_t* {
        if (!SlotUsed(pool, i)) return nullptr;
        const uint8_t* record = pool.records + static_cast<size_t>(i) * 24;   // 24-byte object header
        return *reinterpret_cast<uint8_t* const*>(record + 0x10);
    };
    auto type = [&](int i) { return pool.records[static_cast<size_t>(i) * 24 + 4]; };
    auto placed = [](const uint8_t* object) {
        float p[3];
        std::memcpy(p, object + 0x44, sizeof(p));
        return p[0] != 0.0f || p[1] != 0.0f || p[2] != 0.0f;
    };
    int32_t tag = -1, tagType = -1;
    bool havePos = false;
    for (int wanted : { 2, 0 }) {
        for (int i = 0; i < pool.capacity && tag == -1; ++i) {
            const uint8_t* object = at(i);
            if (!object || type(i) != wanted) continue;
            const int32_t t = *reinterpret_cast<const int32_t*>(object);   // definition (tag) index
            if (t != -1) { tag = t; tagType = wanted; }
        }
    }
    for (int i = 0; i < pool.capacity && !havePos; ++i) {                    // a placed biped = the player
        const uint8_t* object = at(i);
        if (object && type(i) == 0 && placed(object)) {
            std::memcpy(s_refPos, object + 0x44, sizeof(s_refPos));
            s_bipedTag = *reinterpret_cast<const int32_t*>(object);
            float fwd[3];
            std::memcpy(fwd, object + 0x50, sizeof(fwd));                        // facing (placement +0x28 -> object +0x50)
            const float len = std::sqrt(fwd[0] * fwd[0] + fwd[1] * fwd[1]);
            if (len > 0.01f) { s_refForward[0] = fwd[0] / len; s_refForward[1] = fwd[1] / len; }
            havePos = true;
        }
    }
    for (int i = 0; i < pool.capacity && !havePos; ++i) {                    // otherwise any placed object
        const uint8_t* object = at(i);
        if (object && placed(object)) { std::memcpy(s_refPos, object + 0x44, sizeof(s_refPos)); havePos = true; }
    }
    if (tag == -1 || !havePos) return false;
    s_refTag = tag;
    NOBLE_INFO("SpawnTest: copying object type {} (tag index 0x{:08X}), grid around ({:.1f}, {:.1f}, {:.1f})",
        tagType, static_cast<uint32_t>(tag), s_refPos[0], s_refPos[1], s_refPos[2]);
    return true;
}

bool HandleAlive(const ObjectPool& pool, int32_t handle) {
    if (handle == -1) return false;
    const int index = handle & 0xFFFF;
    if (index >= pool.capacity || !SlotUsed(pool, index)) return false;
    const uint16_t salt = *reinterpret_cast<const uint16_t*>(pool.records + static_cast<size_t>(index) * 24);
    return salt == static_cast<uint16_t>(static_cast<uint32_t>(handle) >> 16);
}

int32_t SpawnBodyAt(const float pos[3]) {
    alignas(16) uint8_t placement[kPlacementSize + 16];
    std::memset(placement, 0, sizeof(placement));
    s_placementNew(placement, s_bipedTag, -1, nullptr);
    std::memcpy(placement + 0x1C, pos, sizeof(float) * 3);
    const float facing[3] = { -s_refForward[0], -s_refForward[1], 0.0f };    // face back toward the player
    std::memcpy(placement + 0x28, facing, sizeof(facing));
    return s_objectNew(placement);
}

// Lays out the crowd: 8 rows of 16, starting 5 m in front of the player along the way they face.
void StartCrowd() {
    const float fx = s_refForward[0], fy = s_refForward[1];
    const float rx = fy, ry = -fx;                                              // right-hand side
    for (int k = 0; k < kCrowd; ++k) {
        const float ahead = 5.0f + static_cast<float>(k / 16) * 1.6f;
        const float side = (static_cast<float>(k % 16) - 7.5f) * 1.2f;
        s_crowdPos[k][0] = s_refPos[0] + fx * ahead + rx * side;
        s_crowdPos[k][1] = s_refPos[1] + fy * ahead + ry * side;
        s_crowdPos[k][2] = s_refPos[2] + 0.3f;
        s_crowdHandle[k] = -1;
    }
    s_crowdRespawns = 0;
}

// Puts back any crowd member the engine removed. Called at most twice a second.
void MaintainCrowd(const ObjectPool& pool) {
    int placed = 0, alive = 0;
    for (int k = 0; k < kCrowd; ++k) {
        if (HandleAlive(pool, s_crowdHandle[k])) { ++alive; continue; }
        const bool first = s_crowdHandle[k] == -1;
        s_crowdHandle[k] = SpawnBodyAt(s_crowdPos[k]);
        if (s_crowdHandle[k] != -1) { ++placed; if (!first) ++s_crowdRespawns; }
    }
    if (placed > 0 && alive == 0) {
        NOBLE_INFO("SpawnTest: crowd of {} Spartan bodies in front of you (F8 again to remove) | object pool {} / {}",
            placed, UsedSlots(pool), pool.capacity);
    }
}

// Actor pool (game-state root +0x08) usage, for the F7 log line.
bool ActorUsage(int& used, int& capacity) {
    const uint32_t tlsIndex = *reinterpret_cast<const uint32_t*>(s_base + kTlsIndexRva);
    auto** tls = reinterpret_cast<uint8_t**>(__readgsqword(0x58));
    if (!tls || !tls[tlsIndex]) return false;
    uint8_t* header = *reinterpret_cast<uint8_t**>(tls[tlsIndex] + 0x08);
    if (!header || std::strncmp(reinterpret_cast<const char*>(header), "actor", 32) != 0) return false;
    capacity = *reinterpret_cast<const int32_t*>(header + 0x2C);
    const auto* bitmap = reinterpret_cast<const uint32_t*>(header + *reinterpret_cast<const int64_t*>(header + 0x68));
    used = 0;
    for (int i = 0; i < (capacity + 31) / 32; ++i) used += std::popcount(bitmap[i]);
    return true;
}

// F7: places every squad defined in the map (Firefight and campaign maps have them; Forge World has none).
void PlaceAllSquads() {
    const uint8_t* scenario = *reinterpret_cast<uint8_t* const*>(s_base + kScenarioRva);
    if (!scenario) return;
    const int32_t squads = *reinterpret_cast<const int32_t*>(scenario + 0x398);
    int before = 0, after = 0, capacity = 0;
    ActorUsage(before, capacity);
    if (squads <= 0) {
        NOBLE_WARN("SpawnTest: this map has no AI squads (Forge World has none); use a Firefight or campaign map for F7");
        return;
    }
    for (int32_t i = 0; i < squads; ++i) s_aiPlace(0x20000000u | static_cast<uint32_t>(i), -1, 0);
    ActorUsage(after, capacity);
    NOBLE_INFO("SpawnTest: F7 placed all {} squads | actors {} -> {} / {}", squads, before, after, capacity);
}

void SpawnBatch() {
    ObjectPool pool;
    if (!GetObjectPool(pool)) return;
    if (s_refTag == -1 && !PickReference(pool)) return;   // nothing to copy yet (menu or loading)
    if (s_crowdOn && s_crowdHandle[0] == -2) StartCrowd();
    if (s_crowdOn) {
        if (s_bipedTag == -1) {
            NOBLE_WARN("SpawnTest: no placed Spartan found to copy; F8 needs you in the match as a Spartan");
            s_crowdOn = false;
        } else if (GetTickCount64() - s_lastCrowdCheck > 500) {
            s_lastCrowdCheck = GetTickCount64();
            MaintainCrowd(pool);
        }
    }
    if (s_pending <= 0) return;

    const int inUse = UsedSlots(pool);
    if (inUse >= s_maxObjects) {
        NOBLE_INFO("SpawnTest: stopping at {} objects in use (cap {}; V16 + scheduler guard tops out near 8,800 moving objects; -spawn-max N to change)",
            inUse, s_maxObjects);
        s_pending = 0;
        return;
    }
    alignas(16) uint8_t placement[kPlacementSize + 16];
    const int batch = (std::min)({ s_pending.load(), 50, s_maxObjects - inUse });
    for (int n = 0; n < batch; ++n) {
        std::memset(placement, 0, sizeof(placement));
        s_placementNew(placement, s_refTag, -1, nullptr);
        constexpr int kSide = 40;
        const int k = s_spawned + s_refused;
        float pos[3] = {
            s_refPos[0] + (static_cast<float>(k % kSide) - kSide / 2) * 1.2f,
            s_refPos[1] + (static_cast<float>((k / kSide) % kSide) - kSide / 2) * 1.2f,
            s_refPos[2] + 2.0f + static_cast<float>(k / (kSide * kSide)) * 1.2f,
        };
        std::memcpy(placement + 0x1C, pos, sizeof(pos));
        const int32_t handle = s_objectNew(placement);
        if (handle == -1) {
            ++s_refused;
            ++s_refusedInARow;
        } else {
            ++s_spawned;
            s_refusedInARow = 0;
        }
        const int left = s_pending.fetch_sub(1) - 1;
        if (s_refusedInARow >= 25 || left <= 0 || (s_spawned + s_refused) % 250 == 0) {
            NOBLE_INFO("SpawnTest: spawned {}, refused {} | object pool {} / {}{}", s_spawned, s_refused,
                UsedSlots(pool), pool.capacity, s_refusedInARow >= 25 ? " | engine refused 25 in a row: pool is full" : "");
        }
        if (s_refusedInARow >= 25) {
            s_pending = 0;
            s_refusedInARow = 0;
            break;
        }
        if (left <= 0) break;
    }
}

uint64_t __fastcall Hook_Tick(int32_t a, int32_t b, int32_t c, float d, float e) {
    const uint64_t result = s_originalTick(a, b, c, d, e);
    if (!s_firstTick) s_firstTick = GetTickCount64();
    const bool f9 = (GetAsyncKeyState(VK_F9) & 0x8000) != 0 && GetForegroundWindow() != nullptr;
    if (f9 && !s_f9Down) {
        s_pending += 500;
        NOBLE_INFO("SpawnTest: F9, queued 500 objects");
    }
    s_f9Down = f9;
    const bool f8 = (GetAsyncKeyState(VK_F8) & 0x8000) != 0 && GetForegroundWindow() != nullptr;
    if (f8 && !s_f8Down && GetTickCount64() - s_lastF8 > 600) {   // one press = one toggle
        s_lastF8 = GetTickCount64();
        s_crowdOn = !s_crowdOn;
        if (s_crowdOn) {
            s_refTag = -1;            // re-read the player's position and facing now
            s_crowdHandle[0] = -2;    // marks "lay out on next batch"
        }
        NOBLE_INFO("SpawnTest: F8, crowd of {} Spartan bodies {}", kCrowd, s_crowdOn ? "on" : "off (bodies will be cleaned up by the engine)");
        if (!s_crowdOn) NOBLE_INFO("SpawnTest: crowd was kept alive with {} respawns", s_crowdRespawns);
    }
    s_f8Down = f8;
    const bool f7 = (GetAsyncKeyState(VK_F7) & 0x8000) != 0 && GetForegroundWindow() != nullptr;
    if (f7 && !s_f7Down && GetTickCount64() - s_lastF7 > 600) {
        s_lastF7 = GetTickCount64();
        if (s_aiPlace) PlaceAllSquads();
    }
    s_f7Down = f7;
    if (s_autoCount > 0 && !s_autoQueued && GetTickCount64() - s_firstTick > 8000) {
        s_pending += s_autoCount;
        s_autoQueued = true;
        NOBLE_INFO("SpawnTest: -spawn-test queued {} objects", s_autoCount);
    }
    if (s_pending > 0 || s_crowdOn) SpawnBatch();
    return result;
}

} // namespace
#endif

void Install(HMODULE engine) {
#if defined(NOBLE_DEV_BUILD)
    s_base = reinterpret_cast<uintptr_t>(engine);
    if (!s_base) return;
    auto head = [](uintptr_t rva, const uint8_t* expect, size_t n) { return std::memcmp(reinterpret_cast<void*>(s_base + rva), expect, n) == 0; };
    if (!head(kTickRva, kTickHead, sizeof(kTickHead)) || !head(kPlacementNewRva, kPlacementHead, sizeof(kPlacementHead)) ||
        !head(kObjectNewRva, kObjectNewHead, sizeof(kObjectNewHead))) {
        NOBLE_WARN("SpawnTest: haloreach.dll code does not match the expected build; spawn test disabled");
        return;
    }
    s_placementNew = reinterpret_cast<PlacementNewFn>(s_base + kPlacementNewRva);
    s_objectNew = reinterpret_cast<ObjectNewFn>(s_base + kObjectNewRva);
    if (head(kAiPlaceRva, kAiPlaceHead, sizeof(kAiPlaceHead))) s_aiPlace = reinterpret_cast<AiPlaceFn>(s_base + kAiPlaceRva);
    {
        // Matches run in a relaunched process that does not get the menu's command line, so the options are
        // also passed on through environment variables (inherited by the relaunched process).
        wchar_t env[32] = {};
        if (GetEnvironmentVariableW(L"NOBLE_SPAWN_TEST", env, 32) > 0) s_autoCount = _wtoi(env);
        if (GetEnvironmentVariableW(L"NOBLE_SPAWN_MAX", env, 32) > 0) s_maxObjects = _wtoi(env);
        int argc = 0;
        LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);
        for (int i = 1; argv && i + 1 < argc; ++i) {
            if (_wcsicmp(argv[i], L"-spawn-test") == 0) {
                s_autoCount = _wtoi(argv[i + 1]);
                SetEnvironmentVariableW(L"NOBLE_SPAWN_TEST", argv[i + 1]);
            }
            if (_wcsicmp(argv[i], L"-spawn-max") == 0) {
                s_maxObjects = _wtoi(argv[i + 1]);
                SetEnvironmentVariableW(L"NOBLE_SPAWN_MAX", argv[i + 1]);
            }
        }
        if (argv) LocalFree(argv);
        if (s_maxObjects <= 0) s_maxObjects = 8000;
    }
    if (!HookManager::InstallHook(reinterpret_cast<void*>(s_base + kTickRva), reinterpret_cast<void*>(&Hook_Tick),
            reinterpret_cast<void**>(&s_originalTick), "spawn_test_tick")) {
        NOBLE_WARN("SpawnTest: could not hook the game tick; spawn test disabled");
        return;
    }
    NOBLE_INFO("SpawnTest: ready (F9 = 500 objects, F8 = crowd of 128 Spartans on/off, F7 = place every AI squad; stops at {} objects{})", s_maxObjects,
        s_autoCount > 0 ? ", -spawn-test " + std::to_string(s_autoCount) : std::string());
#else
    (void)engine;
#endif
}

} // namespace noble::spawn
