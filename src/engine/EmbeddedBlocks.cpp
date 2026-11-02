#include "EmbeddedBlocks.hpp"
#include <windows.h>
#include <algorithm>

namespace noble::EmbeddedBlocks {

static std::string NormalizeResourceName(const std::string& filename) {
    std::string s = filename;
    const size_t lastSlash = s.find_last_of("/\\");
    if (lastSlash != std::string::npos) s = s.substr(lastSlash + 1);
    for (char& c : s) {
        if (c == '.') c = '_';
        else c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
    }
    return s;
}

const uint8_t* GetBlock(const std::string& filename, size_t* outSize) {
    const std::string resName = NormalizeResourceName(filename);
    HMODULE hMod = GetModuleHandleW(nullptr);
    HRSRC hRes = FindResourceA(hMod, resName.c_str(), MAKEINTRESOURCEA(10));
    if (!hRes) return nullptr;
    HGLOBAL hGlobal = LoadResource(hMod, hRes);
    if (!hGlobal) return nullptr;
    const void* ptr = LockResource(hGlobal);
    if (!ptr) return nullptr;
    if (outSize) *outSize = static_cast<size_t>(SizeofResource(hMod, hRes));
    return reinterpret_cast<const uint8_t*>(ptr);
}

bool HasBlock(const std::string& filename) {
    const std::string resName = NormalizeResourceName(filename);
    HMODULE hMod = GetModuleHandleW(nullptr);
    return FindResourceA(hMod, resName.c_str(), MAKEINTRESOURCEA(10)) != nullptr;
}

} // namespace noble::EmbeddedBlocks
