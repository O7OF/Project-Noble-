#pragma once

#include <cstdint>
#include <string_view>
#include <vector>
#include <optional>

namespace noble {

class SigScanner {
public:
    // Scans a memory range for an IDA-style signature (e.g. "48 8B 05 ? ? ? ? 48 85 C0")
    static uintptr_t FindPattern(uintptr_t baseAddress, size_t size, std::string_view signature);

    // Resolves a 32-bit RIP-relative displacement at offset (e.g. for CALL, JMP, or MOV reg, [RIP + disp32])
    static uintptr_t ResolveRelative(uintptr_t instructionAddress, int32_t offsetFromInstruction, int32_t instructionLength);

private:
    static void ParseSignature(std::string_view signature, std::vector<int16_t>& bytes);
};

} // namespace noble
