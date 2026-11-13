#include "CollisionWalkGuard.hpp"

#include "common/Logger.hpp"
#include "hooks/HookManager.hpp"

#include <atomic>
#include <cstdint>
#include <cstring>
#include <intrin.h>

namespace noble::guards {

namespace {

// haloreach.dll 1.3528.0.0: recursive half of the "objects near this object" query. It walks one
// collision-hierarchy node: its element list (node+0x08, elements are 0x30 bytes, linked through
// element+0x28) and then both children (node+0x10, node+0x14) through this same function.
constexpr uintptr_t kWalkRva = 0x150A30;
const uint8_t kWalkHead[] = {
    0x48, 0x8B, 0xC4,                   // mov  rax, rsp
    0x48, 0x89, 0x58, 0x08,             // mov  [rax+08h], rbx
    0x48, 0x89, 0x68, 0x10,             // mov  [rax+10h], rbp
    0x48, 0x89, 0x70, 0x18,             // mov  [rax+18h], rsi
    0x57,                               // push rdi
    0x41, 0x56,                         // push r14
    0x41, 0x57,                         // push r15
    0x48, 0x83, 0xEC, 0x50,             // sub  rsp, 50h
};

// Real object hierarchies are tens of nodes deep (the whole node pool holds 6,144 records, shared by
// every object in the level), so 512 levels cannot truncate a healthy walk. 512 frames are 56 KB of
// stack, well inside any of the engine's threads.
constexpr int kMaxDepth = 512;
// Whatever the depth, never get closer than this to the thread's stack limit.
constexpr uintptr_t kStackReserve = 0x10000;   // 64 KB
constexpr uint32_t kMaxLogs = 8;

// int64 walk(uint32_t filter /*rcx*/, const float* position /*rdx*/, uint64_t unused /*r8*/,
//            uint32_t node /*r9*/, void** out /*[rsp+28h]*/, int32_t max /*[rsp+30h]*/)
using WalkFn = int64_t(__fastcall*)(uint32_t, const float*, uint64_t, uint32_t, void**, int32_t);

WalkFn s_walk = nullptr;
thread_local int t_depth = 0;
thread_local uintptr_t t_stackLimit = 0;
std::atomic<uint64_t> s_bails{ 0 };
std::atomic<uint32_t> s_logs{ 0 };
std::atomic<uint32_t> s_seen{ 0 };

int64_t __fastcall Hook_Walk(uint32_t filter, const float* position, uint64_t arg3, uint32_t node, void** out, int32_t max) {
    // One-shot diagnostic: proves the detour is on the query path, and that the argument layout matched what
    // the walk expects (node = a slot handle in the 6,144-record pool, max = the query's result cap, out = stack).
    // The load keeps the hot path free of the locked read-modify-write after the first call.
    if (s_seen.load(std::memory_order_relaxed) == 0 && s_seen.fetch_add(1, std::memory_order_relaxed) == 0) {
        NOBLE_INFO("Collision walk guard: intercepting the query (filter {}, node {:X}, max {}, out {:p})",
            filter, node, max, static_cast<void*>(out));
    }
    if (t_stackLimit == 0) t_stackLimit = __readgsqword(0x10);   // TEB.StackLimit
    const uintptr_t here = reinterpret_cast<uintptr_t>(&filter);
    if (t_depth >= kMaxDepth || (t_stackLimit != 0 && here < t_stackLimit + kStackReserve)) {
        ++s_bails;
        if (s_logs.fetch_add(1) < kMaxLogs) {
            NOBLE_WARN("Collision walk guard: collision hierarchy is cyclic or stale (node {:X}, {} levels deep); stopping "
                       "this walk instead of overflowing the stack", node, t_depth);
        }
        return 0;   // no colliding objects from this subtree; the caller keeps the ones it already found
    }
    ++t_depth;
    const int64_t found = s_walk(filter, position, arg3, node, out, max);
    --t_depth;
    return found;
}

} // namespace

void InstallCollisionWalkGuard(HMODULE engine) {
    const auto base = reinterpret_cast<uintptr_t>(engine);
    if (!base) return;
    if (std::memcmp(reinterpret_cast<void*>(base + kWalkRva), kWalkHead, sizeof(kWalkHead)) != 0) {
        NOBLE_WARN("Collision walk guard: haloreach.dll code does not match the expected build; crash guard skipped");
        return;
    }
    if (!HookManager::InstallHook(reinterpret_cast<void*>(base + kWalkRva), reinterpret_cast<void*>(&Hook_Walk),
            reinterpret_cast<void**>(&s_walk), "collision_walk_guard")) {
        NOBLE_WARN("Collision walk guard: could not hook the hierarchy walk; a cyclic hierarchy can still overflow the stack");
        return;
    }
    NOBLE_INFO("Collision walk guard: active (recursive collision-hierarchy walk is bounded to {} levels)", kMaxDepth);
}

} // namespace noble::guards
