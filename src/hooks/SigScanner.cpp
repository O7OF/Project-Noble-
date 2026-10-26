#include "SigScanner.hpp"
#include <sstream>
#include <iomanip>

namespace noble {

void SigScanner::ParseSignature(std::string_view signature, std::vector<int16_t>& bytes) {
    bytes.clear();
    std::string sig(signature);
    std::stringstream ss(sig);
    std::string token;

    while (ss >> token) {
        if (token == "?" || token == "??") {
            bytes.push_back(-1); // Wildcard
        } else {
            try {
                auto val = static_cast<int16_t>(std::stoul(token, nullptr, 16));
                bytes.push_back(val);
            } catch (...) {
                bytes.push_back(-1);
            }
        }
    }
}

uintptr_t SigScanner::FindPattern(uintptr_t baseAddress, size_t size, std::string_view signature) {
    if (baseAddress == 0 || size == 0) return 0;

    std::vector<int16_t> patternBytes;
    ParseSignature(signature, patternBytes);

    if (patternBytes.empty() || patternBytes.size() > size) {
        return 0;
    }

    const uint8_t* scanStart = reinterpret_cast<const uint8_t*>(baseAddress);
    const size_t scanLen = size - patternBytes.size();
    const size_t patternLen = patternBytes.size();

    for (size_t i = 0; i <= scanLen; ++i) {
        bool match = true;
        for (size_t j = 0; j < patternLen; ++j) {
            if (patternBytes[j] != -1 && patternBytes[j] != scanStart[i + j]) {
                match = false;
                break;
            }
        }
        if (match) {
            return reinterpret_cast<uintptr_t>(&scanStart[i]);
        }
    }

    return 0;
}

uintptr_t SigScanner::ResolveRelative(uintptr_t instructionAddress, int32_t offsetFromInstruction, int32_t instructionLength) {
    if (instructionAddress == 0) return 0;
    const int32_t* dispPtr = reinterpret_cast<const int32_t*>(instructionAddress + offsetFromInstruction);
    int32_t displacement = *dispPtr;
    return instructionAddress + instructionLength + displacement;
}

} // namespace noble
