#pragma once

#include <string_view>

namespace noble {

constexpr int VERSION_MAJOR = 0;
constexpr int VERSION_MINOR = 9;
constexpr int VERSION_PATCH = 10;
constexpr int VERSION_REVISION = 7;
#ifdef NOBLE_DEV_BUILD
constexpr std::string_view VERSION_STRING = "0.9.10.7-dev (Save Migration & Elite Sync)";
#else
constexpr std::string_view VERSION_STRING = "0.9.10.7 (Save Migration & Elite Sync)";
#endif
constexpr std::string_view APP_NAME = "Halo: Reach Community Edition";

} // namespace noble
