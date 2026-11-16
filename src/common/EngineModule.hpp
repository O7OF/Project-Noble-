#pragma once
// Resolves the Reach engine module by name without keeping the literal "haloreach.dll" in the shipped
// binary: the name is decrypted on the stack for the lookup. Every game-module lookup in the client goes
// through here so the string exists in exactly one place (and in no plain-text form).

#include <windows.h>

#include <string>

#include "ObfuscatedString.hpp"

namespace noble {

// The engine's window class name, built on first use from an obfuscated literal so neither the launcher nor
// a dump of its .rdata exposes the string a tool would search for to find the game window.
inline const std::wstring& ReachWindowClass() {
    static const std::wstring name = NOBLE_OBFW(L"HaloReachCE_WindowClass");
    return name;
}

// File name of the engine module, for path building and file-name comparisons.
inline const std::wstring& ReachModuleFileName() {
    static const std::wstring name = NOBLE_OBFW(L"haloreach.dll");
    return name;
}

inline HMODULE ReachModule() {
    // Resolved lazily: the launcher itself starts before the engine DLL is mapped, so a static initialiser
    // here would cache null. Re-resolving while null is cheap and handles the first successful load.
    static HMODULE handle = nullptr;
    if (!handle) {
        handle = ::GetModuleHandleW(NOBLE_OBFW(L"haloreach.dll").c_str());
    }
    return handle;
}

inline HMODULE ReachModuleFromAddress(const void* address) {
    HMODULE module = nullptr;
    if (::GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
                                 GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                             reinterpret_cast<LPCWSTR>(address), &module)) {
        return module;
    }
    return nullptr;
}

} // namespace noble
