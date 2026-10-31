#pragma once

#include <string_view>

namespace noble {

constexpr int VERSION_MAJOR = 0;
constexpr int VERSION_MINOR = 8;
constexpr int VERSION_PATCH = 8;
#ifdef NOBLE_DEV_BUILD
constexpr std::string_view VERSION_STRING = "0.8.8-dev (Lobby, Roster & Scoreboard)";
#else
constexpr std::string_view VERSION_STRING = "0.8.8 (Lobby, Roster & Scoreboard)";
#endif
constexpr std::string_view APP_NAME = "Halo: Reach Community Edition";

} // namespace noble
