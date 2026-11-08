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
#include <vector>
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
// F6 horde: put placed actors on foot around the player (same calls the ai_teleport script command makes).
constexpr uintptr_t kUnitExitVehicleRva = 0x4F9A74;   // unit_exit_vehicle(unit, mode 0 = normal)
constexpr uintptr_t kObjectSetPositionRva = 0x46CB38; // (object, position*, forward*, up*, 0, false)
constexpr uintptr_t kObjectRefreshRva = 0x46C8B4;     // re-links the object to the map after a move
constexpr uintptr_t kActorTeleportedRva = 0x717F50;   // (actor handle) resets the actor's movement state
// The real cinematic state. Reach's script commands cinematic_start (thunk 0x1AF484 -> 0xD3240) and
// cinematic_stop (thunk 0x1AF4A0 -> 0xD3444) toggle a byte in the game-state root: both load the TLS root
// ([gs:0x58] indexed by the TLS slot, the same root the game-state data arrays hang from) and write the byte
// behind root+0x130 (1 on start, 0 on stop; root+0x140 -> word +0x14 and root+0xE0 -> byte +5 move with it).
// The old 0x86CBC/0x85DE0 pair is the shared compiled-out command stub (like drop / cheat_all_*), so it is not
// used. Falls back to the timer when the root is not readable yet (menu/loading).
constexpr uintptr_t kCinematicFlagRootOffset = 0x130;
// Core functions behind the ai_set_* script commands (thunks 0x1A25DC / 0x1A1998 / 0x1A1950): they take the
// resolved ai handle and the value, e.g. ai_set_team(actor, team).
constexpr uintptr_t kSetActorTeamRva = 0x6C1C14;
constexpr uintptr_t kSetActorBlindRva = 0x6BF204;
constexpr uintptr_t kSetActorDeafRva = 0x6BF0EC;
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
using UnitExitVehicleFn = void(__fastcall*)(int32_t unit, int16_t mode);
using ObjectSetPositionFn = void(__fastcall*)(int32_t object, const float* position, const float* forward, const float* up, uint64_t, uint8_t);
using ObjectRefreshFn = void(__fastcall*)(int32_t object);
using ActorTeleportedFn = void(__fastcall*)(int32_t actor);
using SetActorTeamFn = void(__fastcall*)(uint32_t actor, int32_t team);
using SetActorFlagFn = void(__fastcall*)(uint32_t actor, uint8_t value);

uintptr_t s_base = 0;
TickFn s_originalTick = nullptr;
PlacementNewFn s_placementNew = nullptr;
ObjectNewFn s_objectNew = nullptr;
AiPlaceFn s_aiPlace = nullptr;
UnitExitVehicleFn s_unitExitVehicle = nullptr;
ObjectSetPositionFn s_objectSetPosition = nullptr;
ObjectRefreshFn s_objectRefresh = nullptr;
ActorTeleportedFn s_actorTeleported = nullptr;
SetActorTeamFn s_setActorTeam = nullptr;
SetActorFlagFn s_setActorBlind = nullptr;
SetActorFlagFn s_setActorDeaf = nullptr;
int s_hordeTeam = -1;         // -horde-team N: put every placed actor on this team (-1 = leave as placed)
bool s_hordeNoTeleport = false;   // -horde-no-teleport: leave placed actors where the engine puts them
bool s_lastCinematic = false;
bool s_cinematicKnown = false;   // the game-state root + cinematic flag are readable
bool s_cinematicFallbackLogged = false;
bool s_f6Down = false;
ULONGLONG s_lastF6 = 0, s_lastHordeStep = 0;
int s_hordePending = 0;       // actors still to add
int s_hordeSquad = 0;         // next squad to place (round robin)
int s_hordeAdded = 0, s_hordeMoved = 0;
constexpr int kHordeActorsPerStep = 25;   // keep placing squads until this many new actors appeared
bool s_hordeCapLogged = false;            // "capped at the free slots" line, once per queue
bool s_hordeNoSquadsLogged = false;       // "this map has no squads" warning, once
bool s_hordeNoProgressLogged = false;     // "all squads produced nothing" line, once per queue
struct HordeActor { int32_t actor; ULONGLONG since; bool exitAsked; float pos[3]; };
std::vector<HordeActor> s_hordeMoving;
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
int s_autoHorde = 0;          // -horde <count>: auto-queue this many actors after the intro
int s_autoDelay = 10;         // -horde-delay N: explicit timer override; 0/absent = wait for gameplay
bool s_autoDelayExplicit = false;
bool s_autoHordeQueued = false;
bool s_campaignMode = false;  // launch mode is campaign: missions open with cutscenes
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

struct ActorPool { uint8_t* records = nullptr; const uint32_t* bitmap = nullptr; int32_t capacity = 0; };

bool GetActorPool(ActorPool& pool) {
    const uint32_t tlsIndex = *reinterpret_cast<const uint32_t*>(s_base + kTlsIndexRva);
    auto** tls = reinterpret_cast<uint8_t**>(__readgsqword(0x58));
    if (!tls || !tls[tlsIndex]) return false;
    uint8_t* header = *reinterpret_cast<uint8_t**>(tls[tlsIndex] + 0x08);
    if (!header || std::strncmp(reinterpret_cast<const char*>(header), "actor", 32) != 0) return false;
    pool.capacity = *reinterpret_cast<const int32_t*>(header + 0x2C);
    pool.records = *reinterpret_cast<uint8_t**>(header + 0x50);
    pool.bitmap = reinterpret_cast<const uint32_t*>(header + *reinterpret_cast<const int64_t*>(header + 0x68));
    return pool.records && pool.capacity > 0;
}

constexpr size_t kActorSize = 0xD00;
bool ActorSlotUsed(const ActorPool& a, int i) { return (a.bitmap[i >> 5] >> (i & 31)) & 1; }
int32_t ActorHandle(const ActorPool& a, int i) {
    const uint16_t salt = *reinterpret_cast<const uint16_t*>(a.records + static_cast<size_t>(i) * kActorSize);
    return static_cast<int32_t>((static_cast<uint32_t>(salt) << 16) | static_cast<uint32_t>(i));
}
bool ActorAlive(const ActorPool& a, int32_t handle) {
    const int i = handle & 0xFFFF;
    return i < a.capacity && ActorSlotUsed(a, i) && ActorHandle(a, i) == handle;
}
int32_t ActorUnit(const ActorPool& a, int32_t handle) {
    return *reinterpret_cast<const int32_t*>(a.records + static_cast<size_t>(handle & 0xFFFF) * kActorSize + 0x1C);
}

const uint8_t* ObjectData(const ObjectPool& pool, int32_t handle) {
    if (!HandleAlive(pool, handle)) return nullptr;
    return *reinterpret_cast<uint8_t* const*>(pool.records + static_cast<size_t>(handle & 0xFFFF) * 24 + 0x10);
}

// The local player's body: players pool (game-state root +0x18, 0x490-byte records), unit handle at +0x28.
bool PlayerPosition(const ObjectPool& objects, float pos[3], float fwd[2]) {
    const uint32_t tlsIndex = *reinterpret_cast<const uint32_t*>(s_base + kTlsIndexRva);
    auto** tls = reinterpret_cast<uint8_t**>(__readgsqword(0x58));
    if (!tls || !tls[tlsIndex]) return false;
    uint8_t* header = *reinterpret_cast<uint8_t**>(tls[tlsIndex] + 0x18);
    if (!header || std::strncmp(reinterpret_cast<const char*>(header), "players", 32) != 0) return false;
    const int32_t cap = *reinterpret_cast<const int32_t*>(header + 0x2C);
    const uint8_t* records = *reinterpret_cast<uint8_t**>(header + 0x50);
    const auto* bitmap = reinterpret_cast<const uint32_t*>(header + *reinterpret_cast<const int64_t*>(header + 0x68));
    for (int i = 0; records && i < cap; ++i) {
        if (!((bitmap[i >> 5] >> (i & 31)) & 1)) continue;
        const uint8_t* body = ObjectData(objects, *reinterpret_cast<const int32_t*>(records + static_cast<size_t>(i) * 0x490 + 0x28));
        if (!body) continue;
        std::memcpy(pos, body + 0x44, sizeof(float) * 3);
        float f[3];
        std::memcpy(f, body + 0x50, sizeof(f));
        const float len = std::sqrt(f[0] * f[0] + f[1] * f[1]);
        fwd[0] = len > 0.01f ? f[0] / len : 1.0f;
        fwd[1] = len > 0.01f ? f[1] / len : 0.0f;
        return true;
    }
    return false;
}

// One horde step (10 times a second): place squads one after another until this step has created at least
// kHordeActorsPerStep actors or every squad has been tried once, then move the actors they created onto the
// ground in a ring 10-30 m around the player, in front of them. Actors placed in vehicle seats are told to get
// out first. A squad can create several actors at once and some create none, which is why the loop counts new
// actor slots instead of squads.
void HordeStep() {
    ObjectPool objects;
    ActorPool actors;
    if (!GetObjectPool(objects) || !GetActorPool(actors)) return;
    float player[3], fwd[2];
    if (!PlayerPosition(objects, player, fwd)) return;
    const uint8_t* scenario = *reinterpret_cast<uint8_t* const*>(s_base + kScenarioRva);
    const int32_t squads = scenario ? *reinterpret_cast<const int32_t*>(scenario + 0x398) : 0;

    if (s_hordePending > 0 && squads <= 0) {
        if (!s_hordeNoSquadsLogged) {   // Forge World and similar maps have no squads at all
            NOBLE_WARN("SpawnTest: horde needs AI squads; this map has none in its scenario (use a Firefight or campaign map)");
            s_hordeNoSquadsLogged = true;
        }
        s_hordePending = 0;
        return;
    }
    if (s_hordePending > 0 && squads > 0) {
        std::vector<uint32_t> before(actors.bitmap, actors.bitmap + (actors.capacity + 31) / 32);
        int used = 0;
        for (uint32_t w : before) used += std::popcount(w);
        const int freeSlots = actors.capacity - used;
        if (freeSlots <= 0) {
            NOBLE_INFO("SpawnTest: horde stopped, actor pool full ({} / {})", used, actors.capacity);
            s_hordePending = 0;
            s_hordeCapLogged = false;
        } else {
            if (s_hordePending > freeSlots) {   // never queue more than the actor pool can hold
                if (!s_hordeCapLogged) {
                    NOBLE_INFO("SpawnTest: horde capped at the {} free actor slots ({} were queued, actor pool {})",
                        freeSlots, s_hordePending, actors.capacity);
                    s_hordeCapLogged = true;
                }
                s_hordePending = freeSlots;
            }
            int created = 0, tried = 0;
            while (s_hordePending > 0 && created < kHordeActorsPerStep && tried < squads) {
                // Set the team on the squad before placing it, so its actors spawn on one team instead of
                // only being re-teamed after they exist (ai_set_team accepts the squad ai handle too).
                if (s_hordeTeam >= 0 && s_setActorTeam) s_setActorTeam(0x20000000u | static_cast<uint32_t>(s_hordeSquad), s_hordeTeam);
                s_aiPlace(0x20000000u | static_cast<uint32_t>(s_hordeSquad), -1, 0);
                s_hordeSquad = (s_hordeSquad + 1) % squads;
                ++tried;
                for (int i = 0; i < actors.capacity; ++i) {
                    if (!ActorSlotUsed(actors, i) || ((before[i >> 5] >> (i & 31)) & 1)) continue;
                    before[i >> 5] |= 1u << (i & 31);                                  // count each new actor once
                    const int32_t newHandle = ActorHandle(actors, i);
                    if (s_hordeTeam >= 0 && s_setActorTeam) s_setActorTeam(static_cast<uint32_t>(newHandle), s_hordeTeam);
                    if (s_setActorBlind) s_setActorBlind(static_cast<uint32_t>(newHandle), 1);
                    if (s_setActorDeaf) s_setActorDeaf(static_cast<uint32_t>(newHandle), 1);
                    const int k = s_hordeAdded++;
                    const float angle = static_cast<float>(k) * 2.39996f;              // golden-angle spiral
                    const float radius = 10.0f + static_cast<float>(k % 21);
                    const float cx = player[0] + fwd[0] * 15.0f, cy = player[1] + fwd[1] * 15.0f;
                    HordeActor h{ newHandle, GetTickCount64(), false,
                        { cx + std::cos(angle) * radius, cy + std::sin(angle) * radius, player[2] + 1.0f } };
                    if (!s_hordeNoTeleport) s_hordeMoving.push_back(h);
                    ++created;
                    if (--s_hordePending <= 0) break;
                }
            }
            if (created == 0 && tried >= squads && !s_hordeNoProgressLogged) {
                // Every squad produced nothing this round: log once and keep retrying (Firefight re-places
                // squads later). Without the flag this would print every 100 ms.
                NOBLE_INFO("SpawnTest: horde placed no new actors after trying all {} squads (they may all be dead or already placed); still queued {}",
                    squads, s_hordePending);
                s_hordeNoProgressLogged = true;
            }
        }
    }

    const ULONGLONG now = GetTickCount64();
    for (size_t n = 0; n < s_hordeMoving.size();) {
        HordeActor& h = s_hordeMoving[n];
        bool done = !ActorAlive(actors, h.actor) || now - h.since > 8000;
        if (!done) {
            const int32_t unit = ActorUnit(actors, h.actor);
            const uint8_t* body = ObjectData(objects, unit);
            if (!body) {
                done = now - h.since > 3000;
            } else if (*reinterpret_cast<const int32_t*>(body + 0x14) != -1) {   // still attached to a vehicle seat
                if (!h.exitAsked) { s_unitExitVehicle(unit, 0); h.exitAsked = true; }
            } else {
                // Actors that are already next to the player (within 30 m) stay where they are; teleporting
                // them again just drops them into geometry or off ledges and costs frames.
                float at[3];
                std::memcpy(at, body + 0x44, sizeof(at));
                const float dx = at[0] - player[0], dy = at[1] - player[1], dz = at[2] - player[2];
                if (dx * dx + dy * dy + dz * dz <= 30.0f * 30.0f) {
                    done = true;
                } else {
                    s_objectSetPosition(unit, h.pos, nullptr, nullptr, 0, 0);
                    s_objectRefresh(unit);
                    s_actorTeleported(h.actor);
                    ++s_hordeMoved;
                    done = true;
                }
            }
        }
        if (done) { s_hordeMoving[n] = s_hordeMoving.back(); s_hordeMoving.pop_back(); }
        else ++n;
    }
    if (s_hordePending <= 0 && s_hordeMoving.empty() && s_hordeAdded > 0) {
        int used = 0, cap = 0;
        ActorUsage(used, cap);
        NOBLE_INFO("SpawnTest: horde wave done | {} placed, {} moved next to you | actors {} / {}", s_hordeAdded, s_hordeMoved, used, cap);
        s_hordeAdded = s_hordeMoved = 0;
        s_hordeCapLogged = false;
        s_hordeNoProgressLogged = false;
    }
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

// Cinematic state from the game-state root (see kCinematicFlagRootOffset). Guarded: before the game-state
// exists the pointer is null, and a bad read must not take the harness down.
bool CinematicInProgress() {
    const uint32_t tlsIndex = *reinterpret_cast<const uint32_t*>(s_base + kTlsIndexRva);
    auto** tls = reinterpret_cast<uint8_t**>(__readgsqword(0x58));
    if (!tls || !tls[tlsIndex]) return false;
    const uint8_t* root = tls[tlsIndex];
    __try {
        const uint8_t* flag = *reinterpret_cast<uint8_t* const*>(root + kCinematicFlagRootOffset);
        if (!flag) return false;
        s_cinematicKnown = true;
        return *flag != 0;
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        return false;
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
    const bool f6 = (GetAsyncKeyState(VK_F6) & 0x8000) != 0 && GetForegroundWindow() != nullptr;
    if (f6 && !s_f6Down && GetTickCount64() - s_lastF6 > 600 && s_aiPlace && s_unitExitVehicle) {
        s_lastF6 = GetTickCount64();
        s_hordePending += 100;
        s_hordeCapLogged = false;
        s_hordeNoProgressLogged = false;
        NOBLE_INFO("SpawnTest: F6, horde +100 actors around you ({} queued)", s_hordePending);
    }
    s_f6Down = f6;
    // Wait for actual gameplay: the game-state cinematic flag is set by the engine's own cinematic_start /
    // cinematic_stop and is true for the whole of any cutscene, whatever its length. Placing actors during one
    // stalls the game for seconds per frame and the mission scripts clean them up afterwards.
    // -horde-delay N is an explicit override; the timer (campaign 60 s, else 10 s) is the fallback when the
    // game-state flag cannot be read yet.
    const ULONGLONG sinceTick = GetTickCount64() - s_firstTick;
    const bool cinematic = CinematicInProgress();
    if (cinematic != s_lastCinematic) {
        s_lastCinematic = cinematic;
        NOBLE_INFO("SpawnTest: cinematic_in_progress {} (auto-spawn {})", cinematic ? "true" : "false",
            cinematic ? "waiting" : "ready");
    }
    bool autoReady = false;
    if (s_cinematicKnown) {
        autoReady = !cinematic && sinceTick > 3000;   // a moment for the world to exist after the intro
    } else {
        if (!s_cinematicFallbackLogged && sinceTick > 5000) {
            s_cinematicFallbackLogged = true;
            NOBLE_WARN("SpawnTest: game-state cinematic flag not readable yet; auto-spawns use the timer");
        }
        autoReady = sinceTick > static_cast<ULONGLONG>((s_campaignMode ? 60 : s_autoDelay) * 1000);
    }
    if (s_autoDelayExplicit && sinceTick > static_cast<ULONGLONG>(s_autoDelay * 1000)) autoReady = true;
    if (s_autoHorde > 0 && !s_autoHordeQueued && autoReady) {
        s_autoHordeQueued = true;
        s_hordePending += s_autoHorde;
        s_hordeNoProgressLogged = false;
        NOBLE_INFO("SpawnTest: -horde queued {} actors", s_autoHorde);
    }
    // 10 steps a second; each step places several squads (see HordeStep).
    if ((s_hordePending > 0 || !s_hordeMoving.empty()) && GetTickCount64() - s_lastHordeStep > 100) {
        s_lastHordeStep = GetTickCount64();
        HordeStep();
    }
    if (s_autoCount > 0 && !s_autoQueued && autoReady) {
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
        static const uint8_t exitHead[] = { 0x48, 0x89, 0x5C, 0x24, 0x08, 0x55, 0x48, 0x8B, 0xEC, 0x48, 0x83, 0xEC, 0x70 };
        static const uint8_t setPosHead[] = { 0x40, 0x53, 0x55, 0x56, 0x57, 0x48, 0x81, 0xEC, 0x98, 0x00, 0x00, 0x00 };
        static const uint8_t refreshHead[] = { 0x48, 0x89, 0x5C, 0x24, 0x08, 0x48, 0x89, 0x6C, 0x24, 0x10, 0x48, 0x89, 0x74, 0x24, 0x18, 0x57 };
        static const uint8_t teleportedHead[] = { 0x48, 0x83, 0xEC, 0x28, 0x8B, 0x15 };
        static const uint8_t setTeamHead[] = { 0x48, 0x89, 0x5C, 0x24, 0x08, 0x48, 0x89, 0x74, 0x24, 0x10, 0x57, 0x48, 0x83, 0xEC, 0x70, 0x8B, 0xC1 };
        if (head(kSetActorTeamRva, setTeamHead, sizeof(setTeamHead)))
            s_setActorTeam = reinterpret_cast<SetActorTeamFn>(s_base + kSetActorTeamRva);
        else
            NOBLE_WARN("SpawnTest: ai_set_team function not found; -horde-team ignored");
        static const uint8_t setFlagHead[] = { 0x83, 0xF9, 0xFF, 0x0F, 0x84, 0x0E, 0x01, 0x00, 0x00, 0x48, 0x89, 0x5C, 0x24, 0x10, 0x48, 0x89 };
        if (head(kSetActorBlindRva, setFlagHead, sizeof(setFlagHead)))
            s_setActorBlind = reinterpret_cast<SetActorFlagFn>(s_base + kSetActorBlindRva);
        else
            NOBLE_WARN("SpawnTest: ai_set_blind function not found; horde actors stay sighted");
        if (head(kSetActorDeafRva, setFlagHead, sizeof(setFlagHead)))
            s_setActorDeaf = reinterpret_cast<SetActorFlagFn>(s_base + kSetActorDeafRva);
        else
            NOBLE_WARN("SpawnTest: ai_set_deaf function not found; horde actors stay hearing");
        if (head(kUnitExitVehicleRva, exitHead, sizeof(exitHead)) && head(kObjectSetPositionRva, setPosHead, sizeof(setPosHead)) &&
            head(kObjectRefreshRva, refreshHead, sizeof(refreshHead)) && head(kActorTeleportedRva, teleportedHead, sizeof(teleportedHead))) {
            s_unitExitVehicle = reinterpret_cast<UnitExitVehicleFn>(s_base + kUnitExitVehicleRva);
            s_objectSetPosition = reinterpret_cast<ObjectSetPositionFn>(s_base + kObjectSetPositionRva);
            s_objectRefresh = reinterpret_cast<ObjectRefreshFn>(s_base + kObjectRefreshRva);
            s_actorTeleported = reinterpret_cast<ActorTeleportedFn>(s_base + kActorTeleportedRva);
        }
    }
    // Cinematic flag: read from the game-state root (root+0x130 -> byte), the field cinematic_start/stop write.
    NOBLE_INFO("SpawnTest: cinematic state from the game-state root (+0x130); timer fallback if it is unreadable");
    {
        // Matches run in a relaunched process that does not get the menu's command line, so the options are
        // also passed on through environment variables (inherited by the relaunched process).
        wchar_t env[32] = {};
        if (GetEnvironmentVariableW(L"NOBLE_SPAWN_TEST", env, 32) > 0) s_autoCount = _wtoi(env);
        if (GetEnvironmentVariableW(L"NOBLE_SPAWN_MAX", env, 32) > 0) s_maxObjects = _wtoi(env);
        if (GetEnvironmentVariableW(L"NOBLE_HORDE", env, 32) > 0) s_autoHorde = _wtoi(env);
        if (GetEnvironmentVariableW(L"NOBLE_HORDE_DELAY", env, 32) > 0) { s_autoDelay = _wtoi(env); s_autoDelayExplicit = true; }
        if (GetEnvironmentVariableW(L"NOBLE_HORDE_TEAM", env, 32) > 0) s_hordeTeam = _wtoi(env);
        if (GetEnvironmentVariableW(L"NOBLE_HORDE_NO_TELEPORT", env, 32) > 0) s_hordeNoTeleport = env[0] == L'1';
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
            if (_wcsicmp(argv[i], L"-horde") == 0) {
                s_autoHorde = _wtoi(argv[i + 1]);
                SetEnvironmentVariableW(L"NOBLE_HORDE", argv[i + 1]);
            }
            if (_wcsicmp(argv[i], L"-horde-no-teleport") == 0) {
                s_hordeNoTeleport = true;
                SetEnvironmentVariableW(L"NOBLE_HORDE_NO_TELEPORT", L"1");
            }
            if (_wcsicmp(argv[i], L"-horde-team") == 0) {
                s_hordeTeam = _wtoi(argv[i + 1]);
                SetEnvironmentVariableW(L"NOBLE_HORDE_TEAM", argv[i + 1]);
            }
            if (_wcsicmp(argv[i], L"-horde-delay") == 0) {
                s_autoDelay = _wtoi(argv[i + 1]);
                s_autoDelayExplicit = true;
                SetEnvironmentVariableW(L"NOBLE_HORDE_DELAY", argv[i + 1]);
            }
            if (_wcsicmp(argv[i], L"-mode") == 0 && _wcsicmp(argv[i + 1], L"campaign") == 0) s_campaignMode = true;
        }
        if (argv) LocalFree(argv);
        if (s_maxObjects <= 0) s_maxObjects = 8000;
    }
    if (!HookManager::InstallHook(reinterpret_cast<void*>(s_base + kTickRva), reinterpret_cast<void*>(&Hook_Tick),
            reinterpret_cast<void**>(&s_originalTick), "spawn_test_tick")) {
        NOBLE_WARN("SpawnTest: could not hook the game tick; spawn test disabled");
        return;
    }
    if (s_autoHorde > 0 && (!s_aiPlace || !s_unitExitVehicle))
        NOBLE_WARN("SpawnTest: -horde needs ai_place, unit_exit_vehicle and the teleport calls, which this build does not have; horde disabled");
    NOBLE_INFO("SpawnTest: auto-spawns wait for gameplay ({}){}; F9 = 500 objects, F8 = crowd of 128 Spartans on/off, F7 = place every AI squad, F6 = horde +100 actors around you; stops at {} objects{}",
        "game-state cinematic flag, timer fallback",
        s_autoDelayExplicit ? ", override -horde-delay " + std::to_string(s_autoDelay) : "", s_maxObjects,
        (s_autoCount > 0 ? ", -spawn-test " + std::to_string(s_autoCount) : std::string()) +
        (s_autoHorde > 0 ? ", -horde " + std::to_string(s_autoHorde) : std::string()));
#else
    (void)engine;
#endif
}

} // namespace noble::spawn
