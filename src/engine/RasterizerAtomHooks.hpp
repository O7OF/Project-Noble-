#pragma once

#include <cstdint>
#include <windows.h>

namespace noble {

class RasterizerAtomHooks {
public:
    static bool Install(HMODULE hReachModule);
    static void Uninstall();
};

} // namespace noble
