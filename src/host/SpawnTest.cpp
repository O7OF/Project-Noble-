#include "SpawnTest.hpp"
#include "common/Logger.hpp"
#include "hooks/HookManager.hpp"
#include "engine/DevCheats.hpp"
#include "engine/PelicanControls.hpp"
#include "engine/ReachHost.hpp"
#include "network/AuthClient.hpp"

#include <algorithm>
#include <atomic>
#include <bit>
#include <cstdint>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <intrin.h>
#include <mutex>
#include <string>
#include <vector>
#include <shellapi.h>

namespace noble::spawn {

void WatchAllies();   // defined with PlaceAllies below; called from the game tick
void WatchActors();   // -ai-watch, defined with WatchAllies
void PlayerAtStep();  // -player-at, defined with WatchAllies
void MpPathingStep(); // -mp-ai-pathing, defined with WatchAllies
bool EnableMpAiPlacement();   // MAP AI / -mp-ai, defined with ActorsInUse
void MapAiAtStep();           // -map-ai-at, defined with PlaceSquadAt
void PlaceAiRequestStep();    // Forge PLACE AI, defined after MapAiAtStep
void PlaceAiAtStep();         // -place-ai-at (dev test of PLACE AI), defined with it
void NavDumpStep();           // -nav-dump (step 3 research), defined before PlaceAiAtStep

namespace {

// RVAs in haloreach.dll 1.3528.0.0. Identical in stock, V16 and V16+particles (checked byte for byte).
constexpr uintptr_t kTickRva = 0x61A38;            // per-player control tick, game thread
constexpr uintptr_t kPlacementNewRva = 0x49CC24;   // object_placement_data_new(placement*, tag, owner, ptr)
constexpr uintptr_t kObjectNewRva = 0x46D6E0;      // object_new(placement*) -> object handle or -1
constexpr uintptr_t kTlsIndexRva = 0xC17B18;       // TLS slot of the engine's game-state root
constexpr uintptr_t kAiPlaceRva = 0x6C2B10;        // ai_place(ai value, count (-1 = all), flag); bounds-checks squads
constexpr uintptr_t kScenarioRva = 0xC1A230;       // global: current scenario; +0x398 = squad count
// Numpad 6 horde: put placed actors on foot around the player (same calls the ai_teleport script command makes).
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
// Cores behind the script commands ai_force_active (handler 0x1A2400) and ai_force_full_lod (handler 0x1A0CCC).
// Full LOD sets actor+0x88 = 1 for every actor of the ai value, so the AI update scheduler (0x70ABBC) gives
// them full updates every frame instead of reduced ones by distance/importance.
constexpr uintptr_t kAiForceActiveRva = 0x6C1A00;    // (ai value, bool)
constexpr uintptr_t kAiForceLodRva = 0x70AB58;       // (ai value, lod: 1 = full)
// object_destroy (handler 0x19A738): destroys the object only if 0x47A854(object) == -1 (not held by a player).
constexpr uintptr_t kObjectHeldRva = 0x47A854;
constexpr uintptr_t kObjectDestroyRva = 0x1EB650;
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
using AiForceActiveFn = void(__fastcall*)(uint32_t ai, uint8_t active);
using AiForceLodFn = void(__fastcall*)(uint32_t ai, int32_t lod);
using ObjectHeldFn = int32_t(__fastcall*)(int32_t object);
using ObjectDestroyFn = void(__fastcall*)(int32_t object);
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
AiForceActiveFn s_aiForceActive = nullptr;
AiForceLodFn s_aiForceLod = nullptr;
ObjectHeldFn s_objectHeld = nullptr;
int s_hordeSkippedSquads = 0;
ObjectDestroyFn s_objectDestroy = nullptr;
std::vector<uint8_t> s_vehicleSquad;   // -horde-infantry: squads seen spawning in a vehicle are never placed again
SetActorFlagFn s_setActorBlind = nullptr;
SetActorFlagFn s_setActorDeaf = nullptr;
int s_hordeTeam = -1;         // -horde-team N: put every placed actor on this team (-1 = leave as placed)
bool s_hordeNoTeleport = false;   // -horde-no-teleport: leave placed actors where the engine puts them
bool s_hordeFullLod = false;      // -horde-full-lod: ai_force_active + ai_force_full_lod on every placed squad, sighted
bool s_hordeInfantry = false;     // -horde-infantry: squads that spawn seated (dropships, vehicles) get their vehicle destroyed and are skipped from then on
bool s_lastCinematic = false;
bool s_cinematicKnown = false;   // the game-state root + cinematic flag are readable
bool s_cinematicFallbackLogged = false;
bool s_np6Down = false;
ULONGLONG s_lastNp6 = 0, s_lastHordeStep = 0;
int s_hordePending = 0;       // actors still to add
int s_hordeSquad = 0;         // next squad to place (round robin)
int s_hordeAdded = 0, s_hordeMoved = 0;
constexpr int kHordeActorsPerStep = 25;   // keep placing squads until this many new actors appeared
bool s_hordeCapLogged = false;            // "capped at the free slots" line, once per queue
bool s_hordeNoSquadsLogged = false;       // "this map has no squads" warning, once
bool s_hordeNoProgressLogged = false;     // "all squads produced nothing" line, once per queue
struct HordeActor { int32_t actor; ULONGLONG since; bool exitAsked; float pos[3]; };
std::vector<HordeActor> s_hordeMoving;
std::atomic<int> s_aiPlaceRequests{ 0 };   // Numpad 7: place every squad in the map
bool s_np7Down = false;
ULONGLONG s_lastNp7 = 0;

std::atomic<int> s_pending{ 0 };
// Numpad 8: a crowd of 128 copies of the player's biped (128-player render test), kept alive: the engine treats
// bodies without a player as corpses and cleans them up, so missing ones are put back twice a second.
constexpr int kCrowd = 128;
bool s_crowdOn = false;
bool s_np8Down = false;
ULONGLONG s_lastNp8 = 0, s_lastCrowdCheck = 0;
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
bool s_np9Down = false;
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
        NOBLE_INFO("SpawnTest: crowd of {} Spartan bodies in front of you (Numpad 8 again to remove) | object pool {} / {}",
            placed, UsedSlots(pool), pool.capacity);
    }
}

// Actor pool (game-state root +0x08) usage, for the Numpad 7 log line.
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
            if (s_hordeInfantry && s_vehicleSquad.size() != static_cast<size_t>(squads)) s_vehicleSquad.assign(static_cast<size_t>(squads), 0);
            while (s_hordePending > 0 && created < kHordeActorsPerStep && tried < squads) {
                if (s_hordeInfantry && s_vehicleSquad[static_cast<size_t>(s_hordeSquad)]) {
                    s_hordeSquad = (s_hordeSquad + 1) % squads;
                    ++tried;
                    continue;
                }
                // Set the team on the squad before placing it, so its actors spawn on one team instead of
                // only being re-teamed after they exist (ai_set_team accepts the squad ai handle too).
                if (s_hordeTeam >= 0 && s_setActorTeam) s_setActorTeam(0x20000000u | static_cast<uint32_t>(s_hordeSquad), s_hordeTeam);
                s_aiPlace(0x20000000u | static_cast<uint32_t>(s_hordeSquad), -1, 0);
                if (s_hordeFullLod && s_aiForceActive && s_aiForceLod) {
                    s_aiForceActive(0x20000000u | static_cast<uint32_t>(s_hordeSquad), 1);
                    s_aiForceLod(0x20000000u | static_cast<uint32_t>(s_hordeSquad), 1);
                }
                const int placedSquad = s_hordeSquad;
                s_hordeSquad = (s_hordeSquad + 1) % squads;
                ++tried;
                if (s_hordeInfantry && s_objectHeld && s_objectDestroy) {
                    // Any new actor of this squad sitting in a seat -> it's a vehicle/dropship squad: destroy the
                    // vehicles it came in (their riders go with them) and never place it again.
                    bool seated = false;
                    for (int i = 0; i < actors.capacity; ++i) {
                        if (!ActorSlotUsed(actors, i) || ((before[i >> 5] >> (i & 31)) & 1)) continue;
                        const uint8_t* body = ObjectData(objects, ActorUnit(actors, ActorHandle(actors, i)));
                        int32_t vehicle = body ? *reinterpret_cast<const int32_t*>(body + 0x14) : -1;
                        if (vehicle == -1) continue;
                        seated = true;
                        for (int depth = 0; depth < 4; ++depth) {   // a turret seat sits on the dropship: use the top object
                            const uint8_t* v = ObjectData(objects, vehicle);
                            const int32_t parent = v ? *reinterpret_cast<const int32_t*>(v + 0x14) : -1;
                            if (parent == -1) break;
                            vehicle = parent;
                        }
                        if (ObjectData(objects, vehicle) && s_objectHeld(vehicle) == -1) s_objectDestroy(vehicle);
                    }
                    if (seated) {
                        s_vehicleSquad[static_cast<size_t>(placedSquad)] = 1;
                        ++s_hordeSkippedSquads;
                        // Actors removed with their vehicle free their slots; mark whatever is left as seen.
                        for (int i = 0; i < actors.capacity; ++i)
                            if (ActorSlotUsed(actors, i)) before[i >> 5] |= 1u << (i & 31);
                        continue;
                    }
                }
                for (int i = 0; i < actors.capacity; ++i) {
                    if (!ActorSlotUsed(actors, i) || ((before[i >> 5] >> (i & 31)) & 1)) continue;
                    before[i >> 5] |= 1u << (i & 31);                                  // count each new actor once
                    const int32_t newHandle = ActorHandle(actors, i);
                    if (s_hordeTeam >= 0 && s_setActorTeam) s_setActorTeam(static_cast<uint32_t>(newHandle), s_hordeTeam);
                    if (s_setActorBlind && !s_hordeFullLod) s_setActorBlind(static_cast<uint32_t>(newHandle), 1);
                    if (s_setActorDeaf && !s_hordeFullLod) s_setActorDeaf(static_cast<uint32_t>(newHandle), 1);
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
        NOBLE_INFO("SpawnTest: horde wave done | {} placed, {} moved next to you | actors {} / {}{}", s_hordeAdded, s_hordeMoved, used, cap,
            s_hordeInfantry ? " | infantry only, " + std::to_string(s_hordeSkippedSquads) + " vehicle squads skipped" : std::string());
        s_hordeAdded = s_hordeMoved = 0;
        s_hordeCapLogged = false;
        s_hordeNoProgressLogged = false;
    }
}

// Numpad 7: places every squad defined in the map (Firefight and campaign maps have them; Forge World has none).
void PlaceAllSquads() {
    const uint8_t* scenario = *reinterpret_cast<uint8_t* const*>(s_base + kScenarioRva);
    if (!scenario) return;
    const int32_t squads = *reinterpret_cast<const int32_t*>(scenario + 0x398);
    int before = 0, after = 0, capacity = 0;
    ActorUsage(before, capacity);
    if (squads <= 0) {
        NOBLE_WARN("SpawnTest: this map has no AI squads (Forge World has none); use a Firefight or campaign map for Numpad 7");
        return;
    }
    for (int32_t i = 0; i < squads; ++i) s_aiPlace(0x20000000u | static_cast<uint32_t>(i), -1, 0);
    ActorUsage(after, capacity);
    NOBLE_INFO("SpawnTest: Numpad 7 placed all {} squads | actors {} -> {} / {}", squads, before, after, capacity);
}

void SpawnBatch() {
    ObjectPool pool;
    if (!GetObjectPool(pool)) return;
    if (s_refTag == -1 && !PickReference(pool)) return;   // nothing to copy yet (menu or loading)
    if (s_crowdOn && s_crowdHandle[0] == -2) StartCrowd();
    if (s_crowdOn) {
        if (s_bipedTag == -1) {
            NOBLE_WARN("SpawnTest: no placed Spartan found to copy; Numpad 8 needs you in the match as a Spartan");
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
    // FUN_180061A38(a, playerIndex, c, dt1, dt2): the second argument is the player index (it is
    // forwarded to player_control_update), so gate the per-frame work on player 0. The old check used
    // the first argument (a unit handle), so after 7f31cf2 the whole tick body never ran (horde,
    // spawn-test, dev cheats and cinematic gating were dead in every mode since 2026-11-09).
    if (b != 0) return result; // Process Noble ticks, hotkeys and spawner steps only once per frame (player 0)
#ifndef NOBLE_DEDICATED
    noble::DevCheats::GameTick(b);
#endif
    noble::pelican::GameTick(s_base);
    noble::ReachHost::GameThreadTick();   // queued squad placements (Firefight allies) need this thread
    WatchAllies();
    WatchActors();
    PlayerAtStep();
    MpPathingStep();
    MapAiAtStep();
    PlaceAiAtStep();
    PlaceAiRequestStep();
    NavDumpStep();
    if (!s_firstTick) s_firstTick = GetTickCount64();
#ifndef NOBLE_DEDICATED
    const bool isDev = noble::AuthClient::Get().IsDeveloper();
#else
    const bool isDev = false;   // dedicated server: no login, no dev hotkeys
#endif
    const bool np9 = isDev && (GetAsyncKeyState(VK_NUMPAD9) & 0x8000) != 0 && GetForegroundWindow() != nullptr;
    if (np9 && !s_np9Down) {
        s_pending += 500;
        NOBLE_INFO("SpawnTest: Numpad 9, queued 500 objects");
    }
    s_np9Down = np9;
    const bool np8 = isDev && (GetAsyncKeyState(VK_NUMPAD8) & 0x8000) != 0 && GetForegroundWindow() != nullptr;
    if (np8 && !s_np8Down && GetTickCount64() - s_lastNp8 > 600) {   // one press = one toggle
        s_lastNp8 = GetTickCount64();
        s_crowdOn = !s_crowdOn;
        if (s_crowdOn) {
            s_refTag = -1;            // re-read the player's position and facing now
            s_crowdHandle[0] = -2;    // marks "lay out on next batch"
        }
        NOBLE_INFO("SpawnTest: Numpad 8, crowd of {} Spartan bodies {}", kCrowd, s_crowdOn ? "on" : "off (bodies will be cleaned up by the engine)");
        if (!s_crowdOn) NOBLE_INFO("SpawnTest: crowd was kept alive with {} respawns", s_crowdRespawns);
    }
    s_np8Down = np8;
    const bool np7 = isDev && (GetAsyncKeyState(VK_NUMPAD7) & 0x8000) != 0 && GetForegroundWindow() != nullptr;
    if (np7 && !s_np7Down && GetTickCount64() - s_lastNp7 > 600) {
        s_lastNp7 = GetTickCount64();
        if (s_aiPlace) PlaceAllSquads();
    }
    s_np7Down = np7;
    const bool np6 = isDev && (GetAsyncKeyState(VK_NUMPAD6) & 0x8000) != 0 && GetForegroundWindow() != nullptr;
    if (np6 && !s_np6Down && GetTickCount64() - s_lastNp6 > 600 && s_aiPlace && s_unitExitVehicle) {
        s_lastNp6 = GetTickCount64();
        s_hordePending += 100;
        s_hordeCapLogged = false;
        s_hordeNoProgressLogged = false;
        NOBLE_INFO("SpawnTest: Numpad 6, horde +100 actors around you ({} queued)", s_hordePending);
    }
    s_np6Down = np6;
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

void Install(HMODULE engine) {
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
        static const uint8_t forceActiveHead[] = { 0x48, 0x89, 0x5C, 0x24, 0x10, 0x48, 0x89, 0x74, 0x24, 0x18, 0x57, 0x48, 0x83, 0xEC };
        static const uint8_t forceLodHead[] = { 0x40, 0x53, 0x48, 0x81, 0xEC, 0x90, 0x00, 0x00, 0x00, 0x48, 0x8B, 0x05 };
        if (head(kAiForceActiveRva, forceActiveHead, sizeof(forceActiveHead)) && head(kAiForceLodRva, forceLodHead, sizeof(forceLodHead))) {
            s_aiForceActive = reinterpret_cast<AiForceActiveFn>(s_base + kAiForceActiveRva);
            s_aiForceLod = reinterpret_cast<AiForceLodFn>(s_base + kAiForceLodRva);
        } else {
            NOBLE_WARN("SpawnTest: ai_force_active / ai_force_full_lod not found; -horde-full-lod ignored");
        }
        static const uint8_t heldHead[] = { 0x48, 0x89, 0x5C, 0x24, 0x08, 0x48, 0x89, 0x74, 0x24, 0x10, 0x48, 0x89, 0x7C, 0x24 };
        static const uint8_t destroyHead[] = { 0x48, 0x83, 0xEC, 0x28, 0x44, 0x8B, 0xD9, 0x83, 0xF9, 0xFF, 0x74, 0x3A };
        if (head(kObjectHeldRva, heldHead, sizeof(heldHead)) && head(kObjectDestroyRva, destroyHead, sizeof(destroyHead))) {
            s_objectHeld = reinterpret_cast<ObjectHeldFn>(s_base + kObjectHeldRva);
            s_objectDestroy = reinterpret_cast<ObjectDestroyFn>(s_base + kObjectDestroyRva);
        } else {
            NOBLE_WARN("SpawnTest: object_destroy not found; -horde-infantry ignored");
        }
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
        if (GetEnvironmentVariableW(L"NOBLE_HORDE_FULL_LOD", env, 32) > 0) s_hordeFullLod = env[0] == L'1';
        if (GetEnvironmentVariableW(L"NOBLE_HORDE_INFANTRY", env, 32) > 0) s_hordeInfantry = env[0] == L'1';
        int argc = 0;
        LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);
        // Read every argument (a flag can be the last one); options that take a value check that one follows.
        for (int i = 1; argv && i < argc; ++i) {
            const bool hasValue = i + 1 < argc;
            if (hasValue && _wcsicmp(argv[i], L"-spawn-test") == 0) {
                s_autoCount = _wtoi(argv[i + 1]);
                SetEnvironmentVariableW(L"NOBLE_SPAWN_TEST", argv[i + 1]);
            }
            if (hasValue && _wcsicmp(argv[i], L"-spawn-max") == 0) {
                s_maxObjects = _wtoi(argv[i + 1]);
                SetEnvironmentVariableW(L"NOBLE_SPAWN_MAX", argv[i + 1]);
            }
            if (hasValue && _wcsicmp(argv[i], L"-horde") == 0) {
                s_autoHorde = _wtoi(argv[i + 1]);
                SetEnvironmentVariableW(L"NOBLE_HORDE", argv[i + 1]);
            }
            if (_wcsicmp(argv[i], L"-horde-infantry") == 0) {
                s_hordeInfantry = true;
                SetEnvironmentVariableW(L"NOBLE_HORDE_INFANTRY", L"1");   // carried into the relaunched match process
            }
            if (_wcsicmp(argv[i], L"-horde-full-lod") == 0) {
                s_hordeFullLod = true;
                SetEnvironmentVariableW(L"NOBLE_HORDE_FULL_LOD", L"1");   // carried into the relaunched match process
            }
            if (_wcsicmp(argv[i], L"-horde-no-teleport") == 0) {
                s_hordeNoTeleport = true;
                SetEnvironmentVariableW(L"NOBLE_HORDE_NO_TELEPORT", L"1");
            }
            if (hasValue && _wcsicmp(argv[i], L"-player-at") == 0) {
                SetEnvironmentVariableW(L"NOBLE_PLAYER_AT", argv[i + 1]);   // "x,y,z@seconds"
            }
            if (hasValue && _wcsicmp(argv[i], L"-map-ai-at") == 0) {
                SetEnvironmentVariableW(L"NOBLE_MAP_AI_AT", argv[i + 1]);   // "squad|x,y,z@seconds"
            }
            if (_wcsicmp(argv[i], L"-mp-ai-pathing") == 0) {
                SetEnvironmentVariableW(L"NOBLE_MP_AI_PATHING", L"1");   // carried into the relaunched match process
            }
            if (_wcsicmp(argv[i], L"-ai-watch") == 0) {
                SetEnvironmentVariableW(L"NOBLE_AI_WATCH", L"1");   // carried into the relaunched match process
            }
            if (hasValue && _wcsicmp(argv[i], L"-horde-team") == 0) {
                s_hordeTeam = _wtoi(argv[i + 1]);
                SetEnvironmentVariableW(L"NOBLE_HORDE_TEAM", argv[i + 1]);
            }
            if (hasValue && _wcsicmp(argv[i], L"-horde-delay") == 0) {
                s_autoDelay = _wtoi(argv[i + 1]);
                s_autoDelayExplicit = true;
                SetEnvironmentVariableW(L"NOBLE_HORDE_DELAY", argv[i + 1]);
            }
            if (hasValue && _wcsicmp(argv[i], L"-mode") == 0 && _wcsicmp(argv[i + 1], L"campaign") == 0) s_campaignMode = true;
        }
        if (s_autoHorde > 0 || s_hordeTeam >= 0 || s_hordeNoTeleport || s_hordeFullLod || s_hordeInfantry)
            NOBLE_INFO("SpawnTest: horde options | team {} | teleport {} | full LOD {} | infantry only {}", s_hordeTeam,
                s_hordeNoTeleport ? "off" : "on", s_hordeFullLod ? "on" : "off", s_hordeInfantry ? "on" : "off");
        if (argv) LocalFree(argv);
        if (s_maxObjects <= 0) s_maxObjects = 8000;
    }
    {
        // -mp-ai (Phase 7, dev): ai_place (0x6C2B10) returns at once unless game_is_campaign_or_survival()
        // (0x58B5C: game globals +0x10 is 1 campaign or 2 survival; multiplayer and Forge are 3). Only that one call
        // site is changed, to "mov al, 1", so squads can be placed on MP maps; everything else that asks the same
        // question (team relations, damage, metagame) keeps the multiplayer answer.
        bool mpAi = false;
        int argc2 = 0;
        LPWSTR* argv2 = CommandLineToArgvW(GetCommandLineW(), &argc2);
        for (int i = 1; argv2 && i < argc2; ++i)
            if (_wcsicmp(argv2[i], L"-mp-ai") == 0) { mpAi = true; SetEnvironmentVariableW(L"NOBLE_MP_AI", L"1"); }
        if (argv2) LocalFree(argv2);
        wchar_t mpEnv[8] = {};
        if (GetEnvironmentVariableW(L"NOBLE_MP_AI", mpEnv, 8) > 0 && mpEnv[0] == L'1') mpAi = true;
        // -mp-ai-all (Phase 7 DIAGNOSTIC only): game_is_campaign_or_survival() itself returns true everywhere
        // (first bytes of 0x58B5C -> mov al, 1 ; ret). Changes team rules, damage and more; for finding which
        // caller keeps AI from moving in MP, never for play.
        bool mpAiAll = false;
        LPWSTR* argv3 = CommandLineToArgvW(GetCommandLineW(), &argc2);
        for (int i = 1; argv3 && i < argc2; ++i)
            if (_wcsicmp(argv3[i], L"-mp-ai-all") == 0) { mpAiAll = true; SetEnvironmentVariableW(L"NOBLE_MP_AI_ALL", L"1"); }
        if (argv3) LocalFree(argv3);
        if (GetEnvironmentVariableW(L"NOBLE_MP_AI_ALL", mpEnv, 8) > 0 && mpEnv[0] == L'1') mpAiAll = true;
        if (mpAiAll) {
            auto* fn = reinterpret_cast<uint8_t*>(s_base + 0x58B5C);
            static const uint8_t fnPatch[] = { 0xB0, 0x01, 0xC3 };
            DWORD old = 0;
            if (VirtualProtect(fn, sizeof(fnPatch), PAGE_EXECUTE_READWRITE, &old)) {
                std::memcpy(fn, fnPatch, sizeof(fnPatch));
                VirtualProtect(fn, sizeof(fnPatch), old, &old);
                FlushInstructionCache(GetCurrentProcess(), fn, sizeof(fnPatch));
                NOBLE_WARN("SpawnTest: -mp-ai-all: game_is_campaign_or_survival() forced true (diagnostic)");
            }
        }
        if (mpAi) noble::spawn::EnableMpAiPlacement();
    }
    if (!HookManager::InstallHook(reinterpret_cast<void*>(s_base + kTickRva), reinterpret_cast<void*>(&Hook_Tick),
            reinterpret_cast<void**>(&s_originalTick), "spawn_test_tick")) {
        NOBLE_WARN("SpawnTest: could not hook the game tick; spawn test disabled");
        return;
    }
    if (s_autoHorde > 0 && (!s_aiPlace || !s_unitExitVehicle))
        NOBLE_WARN("SpawnTest: -horde needs ai_place, unit_exit_vehicle and the teleport calls, which this build does not have; horde disabled");
    NOBLE_INFO("SpawnTest: auto-spawns wait for gameplay ({}){}; keys need Num Lock on: Numpad 9 = 500 objects, Numpad 8 = crowd of 128 Spartans on/off, Numpad 7 = place every AI squad, Numpad 6 = horde +100 actors around you; stops at {} objects{}",
        "game-state cinematic flag, timer fallback",
        s_autoDelayExplicit ? ", override -horde-delay " + std::to_string(s_autoDelay) : "", s_maxObjects,
        (s_autoCount > 0 ? ", -spawn-test " + std::to_string(s_autoCount) : std::string()) +
        (s_autoHorde > 0 ? ", -horde " + std::to_string(s_autoHorde) : std::string()));
}

// Allies placed this match, logged every 5 s (alive, distance to the local player) so a run shows whether they
// reached the player and survive the waves (i.e. fight on the players' side).
std::vector<int32_t> s_allyWatch;
ULONGLONG s_allyWatchLast = 0;
void WatchAllies() {
    if (s_allyWatch.empty() || GetTickCount64() - s_allyWatchLast < 5000) return;
    s_allyWatchLast = GetTickCount64();
    ObjectPool objects;
    ActorPool actors;
    if (!GetObjectPool(objects) || !GetActorPool(actors)) return;
    float player[3] = {}, fwd[2] = {};
    const bool havePlayer = PlayerPosition(objects, player, fwd);
    int alive = 0;
    std::string dist;
    for (const int32_t a : s_allyWatch) {
        if (!ActorAlive(actors, a)) continue;
        ++alive;
        const uint8_t* body = ObjectData(objects, ActorUnit(actors, a));
        if (body && havePlayer) {
            float at[3];
            std::memcpy(at, body + 0x44, sizeof(at));
            const float dx = at[0] - player[0], dy = at[1] - player[1], dz = at[2] - player[2];
            dist += " " + std::to_string(static_cast<int>(std::sqrt(dx * dx + dy * dy + dz * dz))) + "m";
        }
    }
    NOBLE_INFO("SpawnTest: allies alive {} / {}, distance to player:{}", alive, s_allyWatch.size(), dist);
}

// -ai-watch (Phase 7, dev): every 5 s, every live actor's body position compared with the last sample and with the
// local player. Shows unattended whether placed AI walk (moved), stay on the ground (z range) and approach the player.
int s_aiWatch = -1;   // -1 = not read yet; set from NOBLE_AI_WATCH (Install turns -ai-watch into it)
ULONGLONG s_aiWatchLast = 0;
struct WatchedActor { int32_t actor; float pos[3]; float start[3]; };
std::vector<WatchedActor> s_aiWatched;
void WatchActors() {
    if (s_aiWatch < 0) {
        wchar_t env[8] = {};
        s_aiWatch = (GetEnvironmentVariableW(L"NOBLE_AI_WATCH", env, 8) > 0 && env[0] == L'1') ? 1 : 0;
        if (s_aiWatch) NOBLE_INFO("SpawnTest: ai-watch on (actor positions every 5 s)");
    }
    if (!s_aiWatch || GetTickCount64() - s_aiWatchLast < 5000) return;
    s_aiWatchLast = GetTickCount64();
    ObjectPool objects;
    ActorPool actors;
    if (!GetObjectPool(objects) || !GetActorPool(actors)) return;
    float player[3] = {}, fwd[2] = {};
    const bool havePlayer = PlayerPosition(objects, player, fwd);
    std::vector<WatchedActor> now;
    int moved = 0, total = 0;
    float zMin = 1e9f, zMax = -1e9f, dMin = 1e9f, dMax = 0.0f, dSum = 0.0f, travelMax = 0.0f;
    std::string detail;
    for (int i = 0; i < actors.capacity; ++i) {
        if (!ActorSlotUsed(actors, i)) continue;
        const int32_t actor = ActorHandle(actors, i);
        const uint8_t* body = ObjectData(objects, ActorUnit(actors, actor));
        if (!body) continue;
        WatchedActor w{ actor, {}, {} };
        std::memcpy(w.pos, body + 0x44, sizeof(w.pos));
        std::memcpy(w.start, w.pos, sizeof(w.start));
        for (const WatchedActor& old : s_aiWatched) {
            if (old.actor != actor) continue;
            std::memcpy(w.start, old.start, sizeof(w.start));
            const float mx = w.pos[0] - old.pos[0], my = w.pos[1] - old.pos[1];
            if (mx * mx + my * my > 1.0f) ++moved;
            break;
        }
        const float tx = w.pos[0] - w.start[0], ty = w.pos[1] - w.start[1];
        travelMax = (std::max)(travelMax, std::sqrt(tx * tx + ty * ty));
        zMin = (std::min)(zMin, w.pos[2]);
        zMax = (std::max)(zMax, w.pos[2]);
        float d = 0.0f;
        if (havePlayer) {
            const float dx = w.pos[0] - player[0], dy = w.pos[1] - player[1], dz = w.pos[2] - player[2];
            d = std::sqrt(dx * dx + dy * dy + dz * dz);
            dMin = (std::min)(dMin, d); dMax = (std::max)(dMax, d); dSum += d;
        }
        if (total < 6) {
            char buf[96];
            std::snprintf(buf, sizeof(buf), " [%X %.1f,%.1f,%.1f d%.0f]", static_cast<unsigned>(i), w.pos[0], w.pos[1], w.pos[2], d);
            detail += buf;
        }
        ++total;
        now.push_back(w);
    }
    s_aiWatched = std::move(now);
    // Once per match, 20 s after actors first exist: hex dump of the first actor's record (0xD00 bytes) and its
    // body position, to find the actor's pathfinding location / sector fields by comparing maps (Phase 7 research).
    static ULONGLONG s_firstActorsAt = 0;
    static bool s_dumped = false;
    if (total > 0 && !s_firstActorsAt) s_firstActorsAt = GetTickCount64();
    if (total > 0 && !s_dumped && GetTickCount64() - s_firstActorsAt > 20000) {
        s_dumped = true;
        // The actor that has travelled furthest from where it was first seen (a walking one if any walk).
        int pick = -1; float pickTravel = -1.0f;
        for (const WatchedActor& w : s_aiWatched) {
            const float tx = w.pos[0] - w.start[0], ty = w.pos[1] - w.start[1];
            if (tx * tx + ty * ty > pickTravel) { pickTravel = tx * tx + ty * ty; pick = w.actor & 0xFFFF; }
        }
        NOBLE_INFO("SpawnTest: ai-watch: dumping actor slot {} (travelled {:.1f} m)", pick, std::sqrt((std::max)(0.0f, pickTravel)));
        // Runtime pathfinding data per BSP (collision_surface_get_sector 0x724D24 reads qword hr+0x27B7C18 + bsp*0x490;
        // 0 = no sectors for that BSP) and the picked actor's biped ground contact (biped_find_pathfinding_location
        // 0x4A4424: +0x9DC bsp, +0x9E4 collision surface, +0x9E8 sector ref).
        for (int bsp = 0; bsp < 4; ++bsp)
            NOBLE_INFO("SpawnTest: ai-watch: pathfinding data bsp {} = 0x{:X}", bsp,
                *reinterpret_cast<const uint64_t*>(s_base + 0x27B7C18 + static_cast<size_t>(bsp) * 0x490));
        // Research (Phase 7): squad 0's element (0x6C bytes) and the start of its spawn points block, to find the
        // spawn point layout (position offset, stride) for moving a squad's spawn point at runtime.
        {
            const uint8_t* scen = *reinterpret_cast<uint8_t* const*>(s_base + kScenarioRva);
            const auto* pages = reinterpret_cast<const uint64_t*>(s_base + 0x4E39F20);
            if (scen && *reinterpret_cast<const int32_t*>(scen + 0x398) > 0) {
                const uint32_t sp = *reinterpret_cast<const uint32_t*>(scen + 0x39C);
                const uint8_t* sq = reinterpret_cast<const uint8_t*>(pages[sp >> 28] + static_cast<uint64_t>(sp) * 4);
                auto hex = [](const uint8_t* p, size_t n) { std::string s; char b[4]; for (size_t i = 0; i < n; ++i) { std::snprintf(b, sizeof(b), "%02X ", p[i]); s += b; } return s; };
                NOBLE_INFO("SQDUMP squad0 {}", hex(sq, 0x6C));
                for (int off = 0x20; off <= 0x60; off += 4) {
                    const int32_t cnt = *reinterpret_cast<const int32_t*>(sq + off);
                    const uint32_t pk = *reinterpret_cast<const uint32_t*>(sq + off + 4);
                    if (cnt <= 0 || cnt > 128 || !pk) continue;
                    const uint8_t* el = reinterpret_cast<const uint8_t*>(pages[pk >> 28] + static_cast<uint64_t>(pk) * 4);
                    NOBLE_INFO("SQDUMP block at +0x{:X} count {} first 0xC0: {}", off, cnt, hex(el, 0xC0));
                    (void)el;
                }
            }
        }
        // The loader (0x97C70) only sets that pointer when: the BSP's bit is set in the active mask (hr+0x4E2FB2C),
        // the scenario's structure bsps[bsp].flags (element 0xAC bytes, u16 at +0x50) lacks 0x200 "no pathfinding",
        // and the BSP tag's resource (u16 index at bsp tag +0x520) holds a non-empty block.
        {
            const uint8_t* scen = *reinterpret_cast<uint8_t* const*>(s_base + kScenarioRva);
            const uint32_t mask = *reinterpret_cast<const uint32_t*>(s_base + 0x4E2FB2C);
            const auto* pages = reinterpret_cast<const uint64_t*>(s_base + 0x4E39F20);
            const uint8_t* tagTable = *reinterpret_cast<uint8_t* const*>(s_base + 0xC1A600);
            const int32_t count = scen ? *reinterpret_cast<const int32_t*>(scen + 0x50) : 0;
            const uint32_t packed = scen ? *reinterpret_cast<const uint32_t*>(scen + 0x54) : 0;
            (void)mask;
            for (int bsp = 0; scen && bsp < count && bsp < 4; ++bsp) {
                const uint32_t p = packed + static_cast<uint32_t>(bsp) * 0x2B;
                const uint8_t* el = reinterpret_cast<const uint8_t*>(pages[packed >> 28] + static_cast<uint64_t>(p) * 4);
                const uint16_t flags = *reinterpret_cast<const uint16_t*>(el + 0x50);
                const uint16_t tagIndex = *reinterpret_cast<const uint16_t*>(el + 0xC);
                const uint32_t tagPacked = tagTable ? *reinterpret_cast<const uint32_t*>(tagTable + 4 + static_cast<size_t>(tagIndex) * 8) : 0;
                const uint8_t* tag = tagPacked ? reinterpret_cast<const uint8_t*>(pages[tagPacked >> 28] + static_cast<uint64_t>(tagPacked) * 4) : nullptr;
                NOBLE_INFO("SpawnTest: ai-watch: bsp {} active {} | scenario flags 0x{:X} (no pathfinding {}) | bsp tag 0x{:X} resource index {}",
                    bsp, (mask >> bsp) & 1, flags, (flags & 0x200) ? "SET" : "clear", tagIndex,
                    tag ? *reinterpret_cast<const uint16_t*>(tag + 0x520) : 0xFFFF);
                (void)flags; (void)tag; (void)tagIndex;   // the log line is compiled out of noble-client
            }
        }
        if (pick >= 0 && ActorSlotUsed(actors, pick)) {
            const uint8_t* unit = ObjectData(objects, ActorUnit(actors, ActorHandle(actors, pick)));
            if (unit)
                NOBLE_INFO("SpawnTest: ai-watch: biped ground bsp {} surface {} sector 0x{:08X} | actor +0x500 (no location) {}",
                    *reinterpret_cast<const int16_t*>(unit + 0x9DC), *reinterpret_cast<const int32_t*>(unit + 0x9E4),
                    *reinterpret_cast<const uint32_t*>(unit + 0x9E8),
                    actors.records[static_cast<size_t>(pick) * kActorSize + 0x500]);
        }
        for (int i = (std::max)(0, pick); i < actors.capacity; ++i) {
            if (!ActorSlotUsed(actors, i)) continue;
            const uint8_t* rec = actors.records + static_cast<size_t>(i) * kActorSize;
            const uint8_t* body = ObjectData(objects, ActorUnit(actors, ActorHandle(actors, i)));
            float bp[3] = {};
            if (body) std::memcpy(bp, body + 0x44, sizeof(bp));
            NOBLE_INFO("SpawnTest: ai-watch: actor dump slot {} body {:.3f},{:.3f},{:.3f}", i, bp[0], bp[1], bp[2]);
            for (size_t off = 0; off < kActorSize; off += 32) {
                char line[32 * 3 + 16];
                int n = std::snprintf(line, sizeof(line), "%04zX:", off);
                for (size_t k = 0; k < 32; ++k) n += std::snprintf(line + n, sizeof(line) - n, " %02X", rec[off + k]);
                NOBLE_INFO("ADUMP {}", line);
            }
            break;
        }
    }
    if (total == 0) {
        // The ai_place gates (0x6C2B10 -> 0x694CB4): game globals +0x10 == 1 or 0x58BBC(), and AI globals +1 != 0.
        const uint32_t tlsIndex = *reinterpret_cast<const uint32_t*>(s_base + kTlsIndexRva);
        auto** tls = reinterpret_cast<uint8_t**>(__readgsqword(0x58));
        const uint8_t* root = (tls && tls[tlsIndex]) ? tls[tlsIndex] : nullptr;
        const uint8_t* ai = root ? *reinterpret_cast<uint8_t* const*>(root + 0x30) : nullptr;
        const uint8_t* game = root ? *reinterpret_cast<uint8_t* const*>(root + 0x48) : nullptr;
        NOBLE_INFO("SpawnTest: ai-watch: no actors | AI globals +0..+3 {} {} {} {} | game globals +0x10 {}",
            ai ? ai[0] : -1, ai ? ai[1] : -1, ai ? ai[2] : -1, ai ? ai[3] : -1, game ? game[0x10] : -1);
        (void)ai; (void)game;   // the log line is compiled out of noble-client
        return;
    }
    NOBLE_INFO("SpawnTest: ai-watch: actors {} | moved >1m in 5s {} | furthest from spawn {:.1f}m | z {:.1f}..{:.1f} | to player {:.0f}/{:.0f}/{:.0f}m (min/avg/max){} |{}",
        total, moved, travelMax, zMin, zMax, havePlayer ? dMin : 0.0f, havePlayer ? dSum / total : 0.0f, havePlayer ? dMax : 0.0f,
        havePlayer ? "" : " (no player)", detail);
    if (havePlayer) NOBLE_INFO("SpawnTest: ai-watch: player at {:.1f},{:.1f},{:.1f}", player[0], player[1], player[2]);
}

// -mp-ai-pathing (Phase 7, dev): HREK's cache builder packs the baked nav mesh into the structure BSP's resource
// only for SOLO scenarios (Firefight maps such as ff20 are solo; Forge World is multiplayer), so on an MP-built map
// the runtime pathfinding pointer (qword hr+0x27B7C18 + bsp*0x490, set by 0x97C70 when the BSP loads) stays 0 and no
// actor finds a sector. The fix pairs with a Forge World built with scenario type = solo: its BSP carries the nav
// mesh, and this switch sets the in-memory scenario type (u16 at scenario+0: 0 solo, 1 multiplayer) back to
// multiplayer at the first game tick of a multiplayer match, after the BSPs have loaded. Without it the solo type
// crashes player-view / HUD code (hr+0x30D5A8) in an MP match.
int s_mpPathingState = -1;   // -1 not read, 0 off, 1 on
const uint8_t* s_mpPathingScenario = nullptr;
void MpPathingStep() {
    if (s_mpPathingState < 0) {
        wchar_t env[8] = {};
        s_mpPathingState = (GetEnvironmentVariableW(L"NOBLE_MP_AI_PATHING", env, 8) > 0 && env[0] == L'1') ? 1 : 0;
    }
    auto* scen = *reinterpret_cast<uint8_t* const*>(s_base + kScenarioRva);
    static const uint8_t* s_headLogged = nullptr;
    if (scen && scen != s_headLogged && s_aiWatch == 1) {   // research: scenario tag head (u16 type at +0)
        s_headLogged = scen;
        NOBLE_INFO("SpawnTest: ai-watch: scenario type {} | pathfinding data bsp 0 = 0x{:X}", *reinterpret_cast<const uint16_t*>(scen),
            *reinterpret_cast<const uint64_t*>(s_base + 0x27B7C18));
    }
    if (s_mpPathingState != 1 || !scen || scen == s_mpPathingScenario) return;
    const uint32_t tlsIndex = *reinterpret_cast<const uint32_t*>(s_base + kTlsIndexRva);
    auto** tls = reinterpret_cast<uint8_t**>(__readgsqword(0x58));
    const uint8_t* game = (tls && tls[tlsIndex]) ? *reinterpret_cast<uint8_t* const*>(tls[tlsIndex] + 0x48) : nullptr;
    if (!game) return;
    s_mpPathingScenario = scen;
    auto* type = reinterpret_cast<uint16_t*>(scen);
    if (game[0x10] == 3 && *type == 0) {   // multiplayer match on a solo-built scenario
        DWORD old = 0;
        if (VirtualProtect(type, sizeof(*type), PAGE_READWRITE, &old)) {
            *type = 1;
            VirtualProtect(type, sizeof(*type), old, &old);
        } else {
            *type = 1;
        }
        NOBLE_INFO("SpawnTest: -mp-ai-pathing: solo-built scenario in a multiplayer match: type set to multiplayer | pathfinding data bsp 0 = 0x{:X}",
            *reinterpret_cast<const uint64_t*>(s_base + 0x27B7C18));
    }
}

// -player-at x,y,z@seconds (Phase 7, dev): moves the local player's body to a point once, that many seconds after the
// first game tick (repeated every 10 s while the player is more than 20 m away, so a respawn goes back too).
int s_playerAtState = -1;   // -1 = not read, 0 = off, 1 = on
float s_playerAt[3] = {};
ULONGLONG s_playerAtDelay = 0, s_playerAtFirst = 0, s_playerAtLast = 0;
void PlayerAtStep() {
    if (s_playerAtState < 0) {
        wchar_t env[96] = {};
        s_playerAtState = 0;
        if (GetEnvironmentVariableW(L"NOBLE_PLAYER_AT", env, 96) > 0) {
            float x = 0, y = 0, z = 0; int sec = 20;
            if (swscanf_s(env, L"%f,%f,%f@%d", &x, &y, &z, &sec) >= 3) {
                s_playerAt[0] = x; s_playerAt[1] = y; s_playerAt[2] = z;
                s_playerAtDelay = static_cast<ULONGLONG>(sec) * 1000;
                s_playerAtState = 1;
                NOBLE_INFO("SpawnTest: -player-at {:.1f},{:.1f},{:.1f} after {} s", x, y, z, sec);
            }
        }
    }
    if (s_playerAtState != 1 || !s_objectSetPosition || !s_objectRefresh) return;
    const ULONGLONG now = GetTickCount64();
    if (!s_playerAtFirst) s_playerAtFirst = now;
    if (now - s_playerAtFirst < s_playerAtDelay || now - s_playerAtLast < 10000) return;
    ObjectPool objects;
    if (!GetObjectPool(objects)) return;
    const uint32_t tlsIndex = *reinterpret_cast<const uint32_t*>(s_base + kTlsIndexRva);
    auto** tls = reinterpret_cast<uint8_t**>(__readgsqword(0x58));
    uint8_t* header = (tls && tls[tlsIndex]) ? *reinterpret_cast<uint8_t**>(tls[tlsIndex] + 0x18) : nullptr;
    if (!header || std::strncmp(reinterpret_cast<const char*>(header), "players", 32) != 0) return;
    const int32_t cap = *reinterpret_cast<const int32_t*>(header + 0x2C);
    const uint8_t* records = *reinterpret_cast<uint8_t**>(header + 0x50);
    const auto* bitmap = reinterpret_cast<const uint32_t*>(header + *reinterpret_cast<const int64_t*>(header + 0x68));
    for (int i = 0; records && i < cap; ++i) {
        if (!((bitmap[i >> 5] >> (i & 31)) & 1)) continue;
        const int32_t unit = *reinterpret_cast<const int32_t*>(records + static_cast<size_t>(i) * 0x490 + 0x28);
        const uint8_t* body = ObjectData(objects, unit);
        if (!body) continue;
        float at[3];
        std::memcpy(at, body + 0x44, sizeof(at));
        const float dx = at[0] - s_playerAt[0], dy = at[1] - s_playerAt[1];
        s_playerAtLast = now;
        if (dx * dx + dy * dy < 20.0f * 20.0f) return;
        s_objectSetPosition(unit, s_playerAt, nullptr, nullptr, 0, 0);
        s_objectRefresh(unit);
        NOBLE_INFO("SpawnTest: -player-at moved the player from {:.1f},{:.1f},{:.1f}", at[0], at[1], at[2]);
        return;
    }
}

bool ActorsInUse(int& used, int& capacity) { return ActorUsage(used, capacity); }

bool EnableMpAiPlacement() {
    static bool s_done = false;
    if (s_done) return true;
    if (!s_base) return false;
    constexpr uintptr_t kAiPlaceGateCallRva = 0x6C2B2B;   // E8 2C 60 99 FF  call 0x58B5C (game_is_campaign_or_survival)
    static const uint8_t expect[] = { 0xE8, 0x2C, 0x60, 0x99, 0xFF };
    static const uint8_t patch[] = { 0xB0, 0x01, 0x90, 0x90, 0x90 };   // mov al, 1 ; nop x3
    auto* site = reinterpret_cast<uint8_t*>(s_base + kAiPlaceGateCallRva);
    if (std::memcmp(site, patch, sizeof(patch)) == 0) { s_done = true; return true; }
    DWORD old = 0;
    if (std::memcmp(site, expect, sizeof(expect)) == 0 && VirtualProtect(site, sizeof(patch), PAGE_EXECUTE_READWRITE, &old)) {
        std::memcpy(site, patch, sizeof(patch));
        VirtualProtect(site, sizeof(patch), old, &old);
        FlushInstructionCache(GetCurrentProcess(), site, sizeof(patch));
        NOBLE_INFO("SpawnTest: map AI: ai_place allowed outside campaign / Firefight (patched hr+0x{:X})", kAiPlaceGateCallRva);
        s_done = true;
        return true;
    }
    NOBLE_WARN("SpawnTest: map AI: ai_place gate bytes at hr+0x{:X} do not match; not patched", kAiPlaceGateCallRva);
    return false;
}

bool MapHasAiPaths() {
    if (!s_base) return false;
    const uint8_t* scen = *reinterpret_cast<uint8_t* const*>(s_base + kScenarioRva);
    if (!scen || *reinterpret_cast<const int32_t*>(scen + 0x398) <= 0) return false;
    const int32_t bsps = (std::min)(*reinterpret_cast<const int32_t*>(scen + 0x50), 16);
    for (int bsp = 0; bsp < bsps; ++bsp)   // per-BSP runtime pathfinding pointer (set by 0x97C70 when the BSP loads)
        if (*reinterpret_cast<const uint64_t*>(s_base + 0x27B7C18 + static_cast<size_t>(bsp) * 0x490)) return true;
    return false;
}

bool NavGroundZ(float x, float y, float z, float& out);

int PlaceMapSquads() {
    if (!s_base || !s_aiPlace) return -1;
    const uint8_t* scen = *reinterpret_cast<uint8_t* const*>(s_base + kScenarioRva);
    if (!scen) return -1;
    const int32_t squads = *reinterpret_cast<const int32_t*>(scen + 0x398);
    int before = 0, after = 0, capacity = 0;
    if (!ActorUsage(before, capacity)) return -1;
    // Stand every spawn point on the nav mesh first: a hand-authored height under the terrain drops the actor through
    // the map (seen at z -47 on Forge World). The squad block layout is described above PlaceSquadAt.
    const auto* pages = reinterpret_cast<const uint64_t*>(s_base + 0x4E39F20);
    auto addr = [&](uint32_t packed) { return reinterpret_cast<uint8_t*>(pages[packed >> 28] + static_cast<uint64_t>(packed) * 4); };
    const uint32_t squadsPacked = *reinterpret_cast<const uint32_t*>(scen + 0x39C);
    int snapped = 0;
    for (int32_t i = 0; squadsPacked && i < squads && i < 256; ++i) {
        const uint8_t* sq = addr(squadsPacked) + static_cast<size_t>(i) * 0x6C;
        const int32_t points = *reinterpret_cast<const int32_t*>(sq + 0x3C);
        const uint32_t pointsPacked = *reinterpret_cast<const uint32_t*>(sq + 0x40);
        if (points <= 0 || points > 128 || !pointsPacked) continue;
        uint8_t* first = addr(pointsPacked);
        DWORD old = 0;
        const size_t span = static_cast<size_t>(points) * 0x7C;
        if (!VirtualProtect(first, span, PAGE_READWRITE, &old)) continue;
        for (int k = 0; k < points; ++k) {
            float* p = reinterpret_cast<float*>(first + static_cast<size_t>(k) * 0x7C + 0x14);
            float ground = 0.0f;
            if (NavGroundZ(p[0], p[1], p[2], ground) && std::fabs(ground + 0.4f - p[2]) > 0.5f) { p[2] = ground + 0.4f; ++snapped; }
        }
        VirtualProtect(first, span, old, &old);
    }
    if (snapped) NOBLE_INFO("SpawnTest: map AI: stood {} squad spawn points on the nav mesh", snapped);
    for (int32_t i = 0; i < squads; ++i) s_aiPlace(0x20000000u | static_cast<uint32_t>(i), -1, 0);
    ActorUsage(after, capacity);
    NOBLE_INFO("SpawnTest: map AI: placed {} squads | actors {} -> {} / {}", squads, before, after, capacity);
    return after - before;
}

// Ground height from the loaded nav mesh (Phase 7): the runtime pathfinding data (qword hr+0x27B7C18 + bsp*0x490) has
// the tag block layout: 12-byte block headers, vertices at +0x30 (count) / +0x34 (packed), 12 bytes per vertex. Picks,
// among vertices within 4 m in x/y, the one whose height is closest to `z`. False when nothing is near.
bool NavGroundZ(float x, float y, float z, float& out) {
    const auto* pages = reinterpret_cast<const uint64_t*>(s_base + 0x4E39F20);
    bool found = false;
    float best = 1e9f;
    for (int bsp = 0; bsp < 4; ++bsp) {
        const uint8_t* pf = reinterpret_cast<const uint8_t*>(*reinterpret_cast<const uint64_t*>(s_base + 0x27B7C18 + static_cast<size_t>(bsp) * 0x490));
        if (!pf) continue;
        const int32_t count = *reinterpret_cast<const int32_t*>(pf + 0x30);
        const uint32_t packed = *reinterpret_cast<const uint32_t*>(pf + 0x34);
        if (count <= 0 || count > 1000000 || !packed) continue;
        const auto* v = reinterpret_cast<const float*>(pages[packed >> 28] + static_cast<uint64_t>(packed) * 4);
        for (int i = 0; i < count; ++i) {
            const float dx = v[i * 3] - x, dy = v[i * 3 + 1] - y;
            if (dx * dx + dy * dy > 16.0f) continue;
            const float dz = std::fabs(v[i * 3 + 2] - z);
            if (dz < best) { best = dz; out = v[i * 3 + 2]; found = true; }
        }
    }
    return found;
}

// Squads in the scenario tag (runtime layout, Phase 7): scenario +0x398 count / +0x39C packed address, 0x6C bytes per
// squad, char[32] name at +0; spawn points block at squad +0x3C (count) / +0x40 (packed); spawn point elements 0x7C
// bytes, position (real_point3d) at +0x14.
int PlaceSquadAt(const char* squadName, const float pos[3]) {
    if (!s_base || !s_aiPlace || !squadName || !pos) return -1;
    const uint8_t* scen = *reinterpret_cast<uint8_t* const*>(s_base + kScenarioRva);
    if (!scen) return -1;
    const auto* pages = reinterpret_cast<const uint64_t*>(s_base + 0x4E39F20);
    auto addr = [&](uint32_t packed) { return reinterpret_cast<uint8_t*>(pages[packed >> 28] + static_cast<uint64_t>(packed) * 4); };
    const int32_t squads = *reinterpret_cast<const int32_t*>(scen + 0x398);
    const uint32_t squadsPacked = *reinterpret_cast<const uint32_t*>(scen + 0x39C);
    if (squads <= 0 || !squadsPacked) return -1;
    int index = -1;
    uint8_t* squad = nullptr;
    for (int i = 0; i < squads; ++i) {
        uint8_t* s = addr(squadsPacked) + static_cast<size_t>(i) * 0x6C;
        if (_strnicmp(reinterpret_cast<const char*>(s), squadName, 32) == 0) { index = i; squad = s; break; }
    }
    if (!squad) { NOBLE_WARN("SpawnTest: map AI: no squad named '{}'", squadName); return -1; }
    const int32_t points = *reinterpret_cast<const int32_t*>(squad + 0x3C);
    const uint32_t pointsPacked = *reinterpret_cast<const uint32_t*>(squad + 0x40);
    if (points <= 0 || points > 128 || !pointsPacked) { NOBLE_WARN("SpawnTest: map AI: squad '{}' has no spawn points", squadName); return -1; }
    uint8_t* first = addr(pointsPacked);
    std::vector<float> saved(static_cast<size_t>(points) * 3);
    DWORD old = 0;
    const size_t span = static_cast<size_t>(points) * 0x7C;
    const bool unprotected = VirtualProtect(first, span, PAGE_READWRITE, &old) != 0;
    for (int k = 0; k < points; ++k) {   // a small ring around the target, 1.2 m out
        float* p = reinterpret_cast<float*>(first + static_cast<size_t>(k) * 0x7C + 0x14);
        std::memcpy(&saved[static_cast<size_t>(k) * 3], p, sizeof(float) * 3);
        const float a = 6.2831853f * static_cast<float>(k) / static_cast<float>(points);
        p[0] = pos[0] + (points > 1 ? 1.2f * std::cos(a) : 0.0f);
        p[1] = pos[1] + (points > 1 ? 1.2f * std::sin(a) : 0.0f);
        float ground = 0.0f;   // a point inside the terrain drops the actor through the map: stand it on the nav mesh
        const bool onMesh = NavGroundZ(p[0], p[1], pos[2], ground);
        p[2] = onMesh ? ground + 0.4f : pos[2];
        NOBLE_INFO("SpawnTest: map AI: spawn point {} -> {:.1f},{:.1f},{:.1f} ({})", k, p[0], p[1], p[2], onMesh ? "nav mesh" : "no nav vertex near");
    }
    int before = 0, after = 0, capacity = 0;
    ActorUsage(before, capacity);
    s_aiPlace(0x20000000u | static_cast<uint32_t>(index), -1, 0);
    ActorUsage(after, capacity);
    for (int k = 0; k < points; ++k)   // put the map's own spawn points back
        std::memcpy(first + static_cast<size_t>(k) * 0x7C + 0x14, &saved[static_cast<size_t>(k) * 3], sizeof(float) * 3);
    if (unprotected) VirtualProtect(first, span, old, &old);
    NOBLE_INFO("SpawnTest: map AI: squad '{}' placed at {:.1f},{:.1f},{:.1f} | actors {} -> {}", squadName, pos[0], pos[1], pos[2], before, after);
    return after - before;
}

// -map-ai-at <squad> x,y,z@seconds (Phase 7, dev): places one map squad at a point once (test for Forge-menu AI).
void MapAiAtStep() {
    static int s_state = -1;
    static std::string s_squad;
    static float s_pos[3] = {};
    static ULONGLONG s_delay = 0, s_first = 0;
    if (s_state < 0) {
        s_state = 0;
        wchar_t env[160] = {};
        if (GetEnvironmentVariableW(L"NOBLE_MAP_AI_AT", env, 160) > 0) {
            wchar_t name[64] = {}; float x = 0, y = 0, z = 0; int sec = 20;
            if (swscanf_s(env, L"%63[^|]|%f,%f,%f@%d", name, static_cast<unsigned>(_countof(name)), &x, &y, &z, &sec) >= 4) {
                for (const wchar_t* c = name; *c; ++c) s_squad += static_cast<char>(*c);
                s_pos[0] = x; s_pos[1] = y; s_pos[2] = z;
                s_delay = static_cast<ULONGLONG>(sec) * 1000;
                s_state = 1;
                NOBLE_INFO("SpawnTest: -map-ai-at {} at {:.1f},{:.1f},{:.1f} after {} s", s_squad, x, y, z, sec);
            }
        }
    }
    if (s_state != 1) return;
    const ULONGLONG now = GetTickCount64();
    if (!s_first) s_first = now;
    if (now - s_first < s_delay) return;
    s_state = 2;
    EnableMpAiPlacement();
    PlaceSquadAt(s_squad.c_str(), s_pos);
}

// Forge "PLACE AI" (pause menu). The menu thread queues a kind; the game tick places that squad 6 m in front of the
// local player, on the nav mesh, and leaves a one-line result for the menu to show.
namespace {
constexpr const char* kForgeAiSquads[] = {"noble_ai_elite", "noble_ai_grunt", "noble_ai_jackal", "noble_ai_brute",
                                          "noble_ai_trooper"};
constexpr const char* kForgeAiNames[] = {"Elites", "Grunts", "Jackals", "Brutes", "Troopers"};
constexpr int kForgeAiKinds = static_cast<int>(sizeof(kForgeAiSquads) / sizeof(kForgeAiSquads[0]));
std::atomic<int> s_placeAiRequest{-1};
std::mutex s_placeAiMutex;
std::string s_placeAiResult;
uint32_t s_placeAiSerial = 0;

void SetPlaceAiResult(const std::string& text) {
    std::lock_guard<std::mutex> lock(s_placeAiMutex);
    s_placeAiResult = text;
    ++s_placeAiSerial;
    NOBLE_INFO("SpawnTest: PLACE AI: {}", text);
}
}  // namespace

void RequestPlaceAiInFront(int kind) {
    if (kind >= 0 && kind < kForgeAiKinds) s_placeAiRequest.store(kind);
}

std::string PlaceAiResult(uint32_t& serial) {
    std::lock_guard<std::mutex> lock(s_placeAiMutex);
    serial = s_placeAiSerial;
    return s_placeAiResult;
}

void PlaceAiRequestStep() {
    const int kind = s_placeAiRequest.exchange(-1);
    if (kind < 0) return;
    if (!MapHasAiPaths()) { SetPlaceAiResult("This map has no AI paths. Use an AI-ready map (Noble Forge World)."); return; }
    ObjectPool objects;
    float player[3], fwd[2];
    if (!GetObjectPool(objects) || !PlayerPosition(objects, player, fwd)) { SetPlaceAiResult("Could not find your position."); return; }
    // 6 m ahead first, then nearer / further along the same line, then where you stand. The Monitor flies, so each
    // try takes the walkable ground nearest to the player's height.
    static const float kDistances[] = {6.0f, 4.0f, 8.0f, 3.0f, 10.0f, 12.0f, 0.0f};
    float target[3] = {};
    bool found = false;
    for (const float d : kDistances) {
        float ground = 0.0f;
        const float x = player[0] + fwd[0] * d, y = player[1] + fwd[1] * d;
        if (NavGroundZ(x, y, player[2], ground)) { target[0] = x; target[1] = y; target[2] = ground; found = true; break; }
    }
    if (!found) {
        NOBLE_INFO("SpawnTest: PLACE AI: no nav mesh near {:.1f},{:.1f},{:.1f} facing {:.2f},{:.2f}", player[0], player[1],
                   player[2], fwd[0], fwd[1]);
        SetPlaceAiResult("No walkable ground in front of you. Face open ground and try again.");
        return;
    }
    EnableMpAiPlacement();
    const int made = PlaceSquadAt(kForgeAiSquads[kind], target);
    char msg[160];
    if (made > 0)
        std::snprintf(msg, sizeof(msg), "Placed %d %s in front of you.", made, kForgeAiNames[kind]);
    else
        std::snprintf(msg, sizeof(msg), "Could not place %s (the map has no '%s' squad, or the AI limit is reached).",
                      kForgeAiNames[kind], kForgeAiSquads[kind]);
    SetPlaceAiResult(msg);
}

// -nav-dump (Phase 7 step 3 research, once, 20 s in): the loaded nav mesh's runtime block table (per BSP, data at
// qword hr+0x27B7C18 + bsp*0x490; 12-byte block headers: count, packed address) and the tag page table
// (hr+0x4E39F20, 16 pages; address = pages[packed >> 28] + packed * 4). Element sizes are measured as the distance
// between consecutive blocks when they sit back to back.
void NavDumpStep() {
    static int s_state = -1;
    static ULONGLONG s_first = 0;
    if (s_state < 0) s_state = wcsstr(GetCommandLineW(), L"-nav-dump") ? 1 : 0;
    if (s_state != 1) return;
    const ULONGLONG now = GetTickCount64();
    if (!s_first) s_first = now;
    if (now - s_first < 20000) return;
    s_state = 2;
    const auto* pages = reinterpret_cast<const uint64_t*>(s_base + 0x4E39F20);
    std::string pageLine;
    for (int i = 0; i < 16; ++i) {
        char b[40];
        std::snprintf(b, sizeof(b), " [%d]%llX", i, static_cast<unsigned long long>(pages[i]));
        pageLine += b;
    }
    NOBLE_INFO("SpawnTest: nav-dump: tag pages{}", pageLine);
    static const char* const kBlocks[] = {"sectors", "links", "bsp2d refs", "bsp2d nodes", "vertices", "object refs",
                                          "hints", "instanced geometry refs"};
    for (int bsp = 0; bsp < 4; ++bsp) {
        const uint8_t* pf = reinterpret_cast<const uint8_t*>(*reinterpret_cast<const uint64_t*>(s_base + 0x27B7C18 + static_cast<size_t>(bsp) * 0x490));
        if (!pf) continue;
        NOBLE_INFO("SpawnTest: nav-dump: bsp {} data {}", bsp, static_cast<const void*>(pf));
        uint64_t prevAddr = 0; int prevCount = 0;
        for (int k = 0; k < 8; ++k) {
            const int32_t count = *reinterpret_cast<const int32_t*>(pf + k * 12);
            const uint32_t packed = *reinterpret_cast<const uint32_t*>(pf + k * 12 + 4);
            const uint64_t addr = packed ? pages[packed >> 28] + static_cast<uint64_t>(packed) * 4 : 0;
            const long long stride = (k > 0 && prevCount > 0 && addr > prevAddr) ? static_cast<long long>((addr - prevAddr) / prevCount) : -1;
            NOBLE_INFO("SpawnTest: nav-dump:   +0x{:02X} {:<24} count {:>6} packed {:08X} page {:>2} addr {:X} (previous block {} bytes / element)",
                       k * 12, kBlocks[k], count, packed, packed >> 28, addr, stride);
            prevAddr = addr; prevCount = count;
        }
        const int32_t refs = *reinterpret_cast<const int32_t*>(pf + 0x3C);
        NOBLE_INFO("SpawnTest: nav-dump:   checksum {:08X}, object refs {}", *reinterpret_cast<const uint32_t*>(pf + 0x60), refs);
    }
}

// -place-ai-at <kind>@<seconds> (dev test of the Forge PLACE AI row; kind 0 Elites .. 4 Troopers): presses the row
// once, the same request the menu makes.
void PlaceAiAtStep() {
    static int s_state = -1, s_kind = 0;
    static ULONGLONG s_delay = 0, s_first = 0;
    if (s_state < 0) {
        s_state = 0;
        const wchar_t* arg = wcsstr(GetCommandLineW(), L"-place-ai-at ");
        int kind = 0, sec = 0;
        if (arg && swscanf_s(arg + 13, L"%d@%d", &kind, &sec) == 2) {
            s_kind = kind; s_delay = static_cast<ULONGLONG>(sec) * 1000; s_state = 1;
            NOBLE_INFO("SpawnTest: -place-ai-at kind {} after {} s", kind, sec);
        }
    }
    if (s_state != 1) return;
    const ULONGLONG now = GetTickCount64();
    if (!s_first) s_first = now;
    if (now - s_first < s_delay) return;
    s_state = 2;
    RequestPlaceAiInFront(s_kind);
}

int AlliesAlive() {
    ActorPool actors;
    if (s_allyWatch.empty() || !GetActorPool(actors)) return 0;
    int alive = 0;
    for (const int32_t a : s_allyWatch)
        if (ActorAlive(actors, a)) ++alive;
    if (alive == 0) s_allyWatch.clear();   // the next placement starts a fresh list
    return alive;
}

// Firefight allies (Phase 5b). Game thread only. `place` performs one engine squad placement; every actor it creates
// is moved into scenario squad `destSquad` (ai_migrate's per-actor step), given that squad's runtime team, and walked
// next to the local player through the horde's teleport queue. The destination is a Firefight fireteam squad
// (sur_fireteam_0N): no spawn points of its own, player-side team, outside the wave's living count.
int PlaceAllies(const std::function<bool()>& place, int destSquad) {
    using MigrateFn = void(__fastcall*)(uint32_t actor, uint32_t squad);   // FUN_1806ab15c
    using ActorTeamFn = void(__fastcall*)(uint32_t actor, int32_t team);  // FUN_1806a7c38
    using SquadRefreshFn = void(__fastcall*)(uint32_t squad);            // FUN_1806944c0
    using MigrateDoneFn = void(__fastcall*)();                           // FUN_180697628
    if (!s_base || destSquad < 0) return -1;
    ObjectPool objects;
    ActorPool actors;
    if (!GetObjectPool(objects) || !GetActorPool(actors)) return -1;
    // Squad runtime records: game-state root +0x40 -> data array, records at +0x50, 0xEC bytes each (indexed like
    // the scenario squads), team byte at +0xE7 (what ai_set_team writes).
    const uint32_t tlsIndex = *reinterpret_cast<const uint32_t*>(s_base + kTlsIndexRva);
    auto** tls = reinterpret_cast<uint8_t**>(__readgsqword(0x58));
    uint8_t* squadArray = (tls && tls[tlsIndex]) ? *reinterpret_cast<uint8_t**>(tls[tlsIndex] + 0x40) : nullptr;
    uint8_t* squadRecords = squadArray ? *reinterpret_cast<uint8_t**>(squadArray + 0x50) : nullptr;
    if (!squadRecords) return -1;
    const int team = static_cast<int8_t>(squadRecords[static_cast<size_t>(destSquad) * 0xEC + 0xE7]);
    std::vector<uint32_t> before(actors.bitmap, actors.bitmap + (actors.capacity + 31) / 32);
    if (!place()) return -1;
    float player[3] = {}, fwd[2] = { 1.0f, 0.0f };
    const bool havePlayer = PlayerPosition(objects, player, fwd);
    auto migrate = reinterpret_cast<MigrateFn>(s_base + 0x6AB15C);
    auto setTeam = reinterpret_cast<ActorTeamFn>(s_base + 0x6A7C38);
    int created = 0;
    for (int i = 0; i < actors.capacity; ++i) {
        if (!ActorSlotUsed(actors, i) || ((before[i >> 5] >> (i & 31)) & 1)) continue;
        const int32_t actor = ActorHandle(actors, i);
        NOBLE_INFO("SpawnTest: allies: new actor 0x{:X} unit 0x{:X} definition 0x{:X}", static_cast<uint32_t>(actor),
                   static_cast<uint32_t>(ActorUnit(actors, actor)),
                   ObjectData(objects, ActorUnit(actors, actor))
                       ? *reinterpret_cast<const uint32_t*>(ObjectData(objects, ActorUnit(actors, actor))) : 0u);
        s_allyWatch.push_back(actor);
        migrate(static_cast<uint32_t>(actor), static_cast<uint32_t>(destSquad));
        if (team >= 0) setTeam(static_cast<uint32_t>(actor), team);
        if (havePlayer && s_objectSetPosition && s_objectRefresh && s_actorTeleported && s_unitExitVehicle) {
            // A small ring behind and beside the player (the queue leaves anyone already within 30 m alone).
            const float angle = static_cast<float>(created) * 2.39996f;
            const float radius = 3.0f + static_cast<float>(created % 4);
            const float cx = player[0] - fwd[0] * 3.0f, cy = player[1] - fwd[1] * 3.0f;
            s_hordeMoving.push_back(HordeActor{ actor, GetTickCount64(), false,
                { cx + std::cos(angle) * radius, cy + std::sin(angle) * radius, player[2] + 0.5f } });
        }
        ++created;
    }
    if (created > 0) {
        reinterpret_cast<SquadRefreshFn>(s_base + 0x6944C0)(static_cast<uint32_t>(destSquad));
        reinterpret_cast<MigrateDoneFn>(s_base + 0x697628)();
    }
    NOBLE_INFO("SpawnTest: allies: {} actors into squad {} (team {}){}", created, destSquad, team,
               havePlayer ? ", moving next to the player" : ", no local player to move them to");
    return created;
}

void TriggerAiHorde(int count) {
    if (s_aiPlace && s_unitExitVehicle) {
        s_hordePending += count;
        s_hordeCapLogged = false;
        s_hordeNoProgressLogged = false;
        NOBLE_INFO("SpawnTest: TriggerAiHorde({}), queued {}", count, s_hordePending);
    } else {
        NOBLE_WARN("SpawnTest: TriggerAiHorde failed: ai_place or unit_exit_vehicle not resolved");
    }
}

void TriggerPlaceAllSquads() {
    if (s_aiPlace) {
        PlaceAllSquads();
    } else {
        NOBLE_WARN("SpawnTest: TriggerPlaceAllSquads failed: ai_place not resolved");
    }
}

void TriggerCrowdToggle() {
    s_crowdOn = !s_crowdOn;
    if (s_crowdOn) {
        s_refTag = -1;
        s_crowdHandle[0] = -2;
    }
    NOBLE_INFO("SpawnTest: Numpad 8 Spartan crowd toggled {}", s_crowdOn ? "on" : "off (bodies will be cleaned up by the engine)");
}

void TriggerSpawnObjects(int count) {
    s_pending += count;
    NOBLE_INFO("SpawnTest: TriggerSpawnObjects({}), total queued {}", count, s_pending.load());
}

} // namespace noble::spawn
