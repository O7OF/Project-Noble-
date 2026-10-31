#pragma once

#include <string_view>

namespace noble {

constexpr int VERSION_MAJOR = 0;
constexpr int VERSION_MINOR = 9;
constexpr int VERSION_PATCH = 7;
#ifdef NOBLE_DEV_BUILD
constexpr std::string_view VERSION_STRING = "0.9.7-dev (Scoreboard Voice & Controller)";
#else
constexpr std::string_view VERSION_STRING = "0.9.7 (Scoreboard Voice & Controller)";
#endif
constexpr std::string_view APP_NAME = "Halo: Reach Community Edition";

} // namespace noble
