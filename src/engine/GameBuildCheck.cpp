#include "engine/GameBuildCheck.hpp"

#include "common/EngineModule.hpp"
#include "common/Logger.hpp"

#include <windows.h>

#include <atomic>
#include <cstdint>

namespace noble::GameBuildCheck {
namespace {

// The MCC Halo: Reach build Noble's addresses belong to (PE header link time and image size).
// Update both together with the addresses when Noble moves to a new MCC build (tools/check-addresses.py).
constexpr uint32_t kKnownTimeDateStamp = 0x68A0EFE1;
constexpr uint32_t kKnownSizeOfImage = 0x4EDA000;   // file value, for reference (tools/check-addresses.py)

std::atomic<bool> s_done{false};
std::atomic<bool> s_known{true};

bool ReadHeaders(const uint8_t* base, uint32_t& stamp, uint32_t& size) {
    __try {
        const auto* dos = reinterpret_cast<const IMAGE_DOS_HEADER*>(base);
        if (dos->e_magic != IMAGE_DOS_SIGNATURE) return false;
        const auto* nt = reinterpret_cast<const IMAGE_NT_HEADERS64*>(base + dos->e_lfanew);
        if (nt->Signature != IMAGE_NT_SIGNATURE) return false;
        stamp = nt->FileHeader.TimeDateStamp;
        size = nt->OptionalHeader.SizeOfImage;
        return true;
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        return false;
    }
}

} // namespace

void CheckOnce() {
    if (s_done.load(std::memory_order_relaxed)) return;
    HMODULE module = ReachModule();
    if (!module) return;
    if (s_done.exchange(true)) return;
    uint32_t stamp = 0, size = 0;
    if (!ReadHeaders(reinterpret_cast<const uint8_t*>(module), stamp, size)) {
        NOBLE_WARN("Game version check: could not read the game's version information");
        return;
    }
    // Only the link time is compared: Noble's own loader maps the engine with a larger image than the file
    // says (seen 2026-11-19), so the in-memory image size never matches kKnownSizeOfImage.
    const bool known = stamp == kKnownTimeDateStamp;
    s_known.store(known);
#ifdef NOBLE_DEV_BUILD
    NOBLE_INFO("Game version check (dev): link stamp {:08X}, image size {:X}", stamp, size);
#endif
    if (known) {
        NOBLE_INFO("Game version check: this is the MCC Halo: Reach version Noble was made for");
    } else {
        NOBLE_WARN("Game version check: MCC Halo: Reach has been updated since this Noble version was made. "
                   "Features that change the game may switch themselves off until Noble is updated.");
    }
}

bool IsKnownBuild() { return s_known.load(std::memory_order_relaxed); }

} // namespace noble::GameBuildCheck
