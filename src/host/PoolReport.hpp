#pragma once
// Live engine pool report (LIMITS-HANDOFF.md, "how do we know the limits work").
//
// haloreach.dll keeps most game-state pools in "data arrays". Each has a 0x70-byte header created by the
// allocator at RVA 0x12FCC: name[32] at +0x00, record size at +0x20, signature 0x64407440 ("d@t@") at
// +0x28, capacity at +0x2C, data offset at +0x60 and in-use bitmap offset at +0x68 (both from the header).
// The report finds those headers in memory and prints capacity and slots in use, so the running engine
// itself shows whether the extended limits are in effect (object 2,048 on Stock, 32,767 with V16).
//
// Development builds only. The report is logged once the game state exists and again whenever a pool's
// capacity or the object/actor usage changes. "-pool-report-exit <seconds>" logs one report after that many
// seconds and ends the process; tools\limits\test-limits.ps1 uses it.
#include <cstdint>
#include <string>
#include <vector>

namespace noble::pools {

struct PoolInfo {
    std::string name;
    uint32_t capacity = 0;
    uint32_t used = 0;         // UINT32_MAX when the bitmap could not be read
    uint64_t recordSize = 0;
    uintptr_t address = 0;
    bool inEngineImage = false;  // header lies inside haloreach.dll (static storage / checkpoint copy)
};

// Scans readable committed memory for data-array headers. One entry per pool name: the live copy
// (outside the DLL image) is preferred over checkpoint copies stored in the image.
std::vector<PoolInfo> Scan(uintptr_t engineBase, size_t engineSize);

// Starts the background reporter (development builds; no-op otherwise). Call once after the DLL loads.
void StartReporter(uintptr_t engineBase, size_t engineSize, const std::string& profileName);

} // namespace noble::pools
