#pragma once

#include <string_view>

namespace noble {

constexpr int VERSION_MAJOR = 0;
constexpr int VERSION_MINOR = 9;
constexpr int VERSION_PATCH = 16;
constexpr int VERSION_REVISION = 31;
#ifdef NOBLE_DEV_BUILD
constexpr std::string_view VERSION_STRING = "0.9.16.31-dev (Exclusive Fullscreen, Borderless Default, Visor Color Fix)";
#else
constexpr std::string_view VERSION_STRING = "0.9.16.31 (Exclusive Fullscreen, Borderless Default, Visor Color Fix)";
#endif
constexpr std::string_view APP_NAME = "Halo: Reach Community Edition";

} // namespace noble
