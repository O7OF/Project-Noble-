#pragma once

#include <string_view>

namespace noble {

constexpr int VERSION_MAJOR = 0;
constexpr int VERSION_MINOR = 9;
constexpr int VERSION_PATCH = 16;
constexpr int VERSION_REVISION = 18;
#ifdef NOBLE_DEV_BUILD
constexpr std::string_view VERSION_STRING = "0.9.16.18-dev (Embedded Launch Blocks, App Icon, Clean Unified Save Storage, Armory Purchase & Career Stat Tracking)";
#else
constexpr std::string_view VERSION_STRING = "0.9.16.18 (Embedded Launch Blocks, App Icon, Clean Unified Save Storage, Armory Purchase & Career Stat Tracking)";
#endif
constexpr std::string_view APP_NAME = "Halo: Reach Community Edition";

} // namespace noble
