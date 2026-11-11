#pragma once

#include <filesystem>

namespace noble {

// Watches every writable committed page on a timer and writes what changed between scans. Placing a Forge object
// changes very little memory, so two scans that bracket the placement point straight at the editor's structures.
// Runs on its own thread; pages are read only, and only committed, non-guard, writable ones are touched.
void StartPageScan(int seconds, std::filesystem::path outDir);
void StopPageScan();

} // namespace noble
