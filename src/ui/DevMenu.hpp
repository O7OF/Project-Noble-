#pragma once

#include <windows.h>
#include <string>

#if defined(NOBLE_DEV_BUILD)

namespace noble {

class DevMenu {
public:
    static bool IsOpen();
    static void Toggle();
    static void Open();
    static void Close();
    static bool WantsPointer();

    // Renders the Developer Mod / Cheat Menu overlay
    static void Draw();

private:
    static bool s_open;
    static int s_categoryFilter;
    static char s_searchQuery[64];
    static std::string s_statusMessage;
    static unsigned long long s_statusTime;
};

} // namespace noble

#else // !defined(NOBLE_DEV_BUILD) - Release build stubs (zero dev menu code compiled in)

namespace noble {

class DevMenu {
public:
    static constexpr bool IsOpen() { return false; }
    static void Toggle() {}
    static void Open() {}
    static void Close() {}
    static constexpr bool WantsPointer() { return false; }
    static void Draw() {}
};

} // namespace noble

#endif
