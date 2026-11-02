#pragma once

#include <string_view>

namespace noble {

constexpr int VERSION_MAJOR = 0;
constexpr int VERSION_MINOR = 9;
constexpr int VERSION_PATCH = 16;
constexpr int VERSION_REVISION = 33;
#ifdef NOBLE_DEV_BUILD
constexpr std::string_view VERSION_STRING = "0.9.16.33-dev (Armor Fix, 4K UI Scaling, Exclusive Fullscreen)";
#else
constexpr std::string_view VERSION_STRING = "0.9.16.33 (Armor Fix, 4K UI Scaling, Exclusive Fullscreen)";
#endif
constexpr std::string_view APP_NAME = "Halo: Reach Community Edition";

} // namespace noble
