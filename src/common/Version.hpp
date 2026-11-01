#pragma once

#include <string_view>

namespace noble {

constexpr int VERSION_MAJOR = 0;
constexpr int VERSION_MINOR = 9;
constexpr int VERSION_PATCH = 16;
constexpr int VERSION_REVISION = 15;
#ifdef NOBLE_DEV_BUILD
constexpr std::string_view VERSION_STRING = "0.9.16.15-dev (Elite Illumination Isolation, Stable ID Persistence & Scoreboard Names)";
#else
constexpr std::string_view VERSION_STRING = "0.9.16.15 (Elite Illumination Isolation, Stable ID Persistence & Scoreboard Names)";
#endif
constexpr std::string_view APP_NAME = "Halo: Reach Community Edition";

} // namespace noble
