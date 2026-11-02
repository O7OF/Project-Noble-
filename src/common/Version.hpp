#pragma once

#include <string_view>

namespace noble {

constexpr int VERSION_MAJOR = 0;
constexpr int VERSION_MINOR = 9;
constexpr int VERSION_PATCH = 16;
constexpr int VERSION_REVISION = 34;
#ifdef NOBLE_DEV_BUILD
constexpr std::string_view VERSION_STRING = "0.9.16.34-dev (New RR Mainmenu, Remapped Armor GLOBALS, 4K UI, Exclusive Fullscreen)";
#else
constexpr std::string_view VERSION_STRING = "0.9.16.34 (New RR Mainmenu, Remapped Armor GLOBALS, 4K UI, Exclusive Fullscreen)";
#endif
constexpr std::string_view APP_NAME = "Halo: Reach Community Edition";

} // namespace noble
