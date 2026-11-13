#pragma once

#include <windows.h>
#include <string>

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
