#pragma once
// Scheduler guard for the extended (V16-layout) engine (LIMITS-TODO C10).
//
// Each frame the engine's object update reserves one scheduler job per object in one go
// (prepare, haloreach+0x68ADD0) and never checks that enough jobs are free. V16 has 16,384 jobs; with
// about 6,500 moving objects the free-job queue runs dry and the engine crashes popping it
// (haloreach+0x68B21F). The guard runs before prepare: if fewer jobs are free than requested, it runs
// queued jobs on this thread through the engine's own helper (haloreach+0x68B254, the call the engine's
// job-wait loop makes) until enough have finished and been returned, then lets prepare continue.
// Finished jobs return their slots immediately, so this turns the crash into a short stall.
//
// Installed only when the loaded DLL has the V16 scheduler layout (code bytes checked); no effect on Stock.
#include <windows.h>

namespace noble::scheduler {

void InstallGuard(HMODULE engine);

} // namespace noble::scheduler
