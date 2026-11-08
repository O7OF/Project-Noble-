#include "SchedulerGuard.hpp"

#include "common/Logger.hpp"
#include "hooks/HookManager.hpp"

#include <atomic>
#include <cstdint>
#include <cstring>

namespace noble::scheduler {

namespace {

constexpr uintptr_t kPrepareRva = 0x68ADD0;     // prepare(object-job context, job count)
constexpr uintptr_t kRunJobRva = 0x68B254;      // runs a queued job on this thread (engine job-wait helper)

// V16 layout (SCHEDULER.md): context +0x44008 = manager, manager +0x08 = scheduler owner,
// owner +0x88 = body, body +0x260038 = free-job queue, queue +0x30000 = free count,
// owner +0x2900F0 = worker context passed to the run-job helper.
constexpr size_t kCtxManager = 0x44008;
constexpr size_t kBody = 0x88;
constexpr size_t kFreeCount = 0x260038 + 0x30000;
constexpr size_t kWorkerContext = 0x2900F0;
constexpr uint32_t kMargin = 64;                 // other threads may take jobs between the check and prepare
constexpr DWORD kTimeoutMs = 3000;
constexpr uint32_t kSchedulerJobs = 16384;      // V16 job capacity (and per-call scratch capacity)

// First 0x30 bytes of prepare in the V16 build (the +0x44008 / +0x20000 operands identify the layout).
const uint8_t kPrepareV16[] = {
    0x48, 0x89, 0x5C, 0x24, 0x10, 0x48, 0x89, 0x74, 0x24, 0x18, 0x48, 0x89, 0x7C, 0x24, 0x20, 0x41,
    0x56, 0x48, 0x83, 0xEC, 0x20, 0x48, 0x8B, 0x81, 0x08, 0x40, 0x04, 0x00, 0x4C, 0x8D, 0xB1, 0x00,
    0x00, 0x02, 0x00, 0x8B, 0xFA, 0x48, 0x8B, 0xF1, 0x33, 0xD2, 0x48, 0x8B, 0x58, 0x08, 0x48, 0x81,
};
const uint8_t kRunJobHead[] = {
    0x48, 0x89, 0x5C, 0x24, 0x18, 0x55, 0x56, 0x57, 0x41, 0x54, 0x41, 0x55, 0x41, 0x56, 0x41, 0x57,
};

using PrepareFn = void(__fastcall*)(void* context, uint32_t count);
using RunJobFn = uint64_t(__fastcall*)(void* workerContext, int32_t, int32_t, void* scratch);

PrepareFn s_originalPrepare = nullptr;
RunJobFn s_runJob = nullptr;
std::atomic<uint64_t> s_waits{ 0 };
ULONGLONG s_lastLog = 0;

void __fastcall Hook_Prepare(void* context, uint32_t count) {
    if (count > 0 && context) {
        auto* manager = *reinterpret_cast<uint8_t**>(static_cast<uint8_t*>(context) + kCtxManager);
        auto* owner = manager ? *reinterpret_cast<uint8_t**>(manager + 0x08) : nullptr;
        if (owner) {
            auto* freeCount = reinterpret_cast<volatile int32_t*>(owner + kBody + kFreeCount);
            if (count + kMargin > kSchedulerJobs) {
                // Bigger than the whole scheduler: waiting cannot help, so do not stall the frame for it.
                const ULONGLONG now = GetTickCount64();
                if (now - s_lastLog > 5000) {
                    s_lastLog = now;
                    NOBLE_ERROR("Scheduler guard: object update wants {} jobs but the scheduler only has {}; too many objects "
                                "are moving at once (V16 ceiling is about 8,000-8,500)", count, kSchedulerJobs);
                }
            } else if (*freeCount < static_cast<int32_t>(count + kMargin)) {
                const int32_t freeBefore = *freeCount;
                const ULONGLONG start = GetTickCount64();
                uint64_t runs = 0;
                while (*freeCount < static_cast<int32_t>(count + kMargin) && GetTickCount64() - start < kTimeoutMs) {
                    alignas(16) uint8_t scratch[32] = {};
                    s_runJob(owner + kWorkerContext, 0, 1, scratch);   // same call as the engine's job-wait loop
                    ++runs;
                    if ((runs & 15) == 0) SwitchToThread();             // let the worker threads finish theirs
                }
                ++s_waits;
                const ULONGLONG now = GetTickCount64();
                const bool timedOut = *freeCount < static_cast<int32_t>(count);
                if (timedOut || now - s_lastLog > 5000) {
                    s_lastLog = now;
                    if (timedOut) {
                        NOBLE_ERROR("Scheduler guard: {} jobs needed but only {} free after {} ms; more objects are moving than "
                                    "the scheduler can hold", count, static_cast<int32_t>(*freeCount), kTimeoutMs);
                    } else {
                        NOBLE_WARN("Scheduler guard: {} jobs needed, {} were free; waited {} ms for finished jobs ({} waits so far)",
                            count, freeBefore, now - start, s_waits.load());
                    }
                }
            }
        }
    }
    s_originalPrepare(context, count);
}

} // namespace

void InstallGuard(HMODULE engine) {
    const auto base = reinterpret_cast<uintptr_t>(engine);
    if (!base) return;
    if (std::memcmp(reinterpret_cast<void*>(base + kPrepareRva), kPrepareV16, sizeof(kPrepareV16)) != 0 ||
        std::memcmp(reinterpret_cast<void*>(base + kRunJobRva), kRunJobHead, sizeof(kRunJobHead)) != 0) {
        return;   // not the V16 scheduler layout (Stock, or another build): nothing to guard
    }
    s_runJob = reinterpret_cast<RunJobFn>(base + kRunJobRva);
    if (!HookManager::InstallHook(reinterpret_cast<void*>(base + kPrepareRva), reinterpret_cast<void*>(&Hook_Prepare),
            reinterpret_cast<void**>(&s_originalPrepare), "scheduler_guard_prepare")) {
        NOBLE_WARN("Scheduler guard: could not hook the object-job prepare; running without it");
        return;
    }
    NOBLE_INFO("Scheduler guard: active (object updates wait for free scheduler jobs instead of crashing)");
}

} // namespace noble::scheduler
