#pragma once

#include <string_view>

namespace noble {

constexpr int VERSION_MAJOR = 0;
constexpr int VERSION_MINOR = 9;
constexpr int VERSION_PATCH = 9;
constexpr int VERSION_REVISION = 1;
#ifdef NOBLE_DEV_BUILD
constexpr std::string_view VERSION_STRING = "0.9.9.1-dev (Armory Visor Unlocks)";
#else
constexpr std::string_view VERSION_STRING = "0.9.9.1 (Armory Visor Unlocks)";
#endif
constexpr std::string_view APP_NAME = "Halo: Reach Community Edition";

} // namespace noble
