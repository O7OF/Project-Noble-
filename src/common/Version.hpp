#pragma once

#include <string_view>

namespace noble {

constexpr int VERSION_MAJOR = 0;
constexpr int VERSION_MINOR = 9;
constexpr int VERSION_PATCH = 16;
constexpr int VERSION_REVISION = 25;
#ifdef NOBLE_DEV_BUILD
constexpr std::string_view VERSION_STRING = "0.9.16.25-dev (Live Borderless, Native-Resolution Rendering, Crisp Menu Backdrops, Responsive Buttons)";
#else
constexpr std::string_view VERSION_STRING = "0.9.16.25 (Live Borderless, Native-Resolution Rendering, Crisp Menu Backdrops, Responsive Buttons)";
#endif
constexpr std::string_view APP_NAME = "Halo: Reach Community Edition";

} // namespace noble
