#pragma once

#include <filesystem>

namespace noble {

// Scans this process's memory for loaded map variants (BLF/"mvar" signature) and writes each to outDir.
// Returns how many were found. Used to work out where the engine keeps the Forge map it would save.
int DumpMapVariantCandidates(const std::filesystem::path& outDir);

} // namespace noble
