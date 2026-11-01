#pragma once

#include <string_view>

namespace noble {

constexpr int VERSION_MAJOR = 0;
constexpr int VERSION_MINOR = 9;
constexpr int VERSION_PATCH = 14;
constexpr int VERSION_REVISION = 2;
#ifdef NOBLE_DEV_BUILD
constexpr std::string_view VERSION_STRING = "0.9.14.2-dev (Armor Illumination & Replication)";
#else
constexpr std::string_view VERSION_STRING = "0.9.14.2 (Armor Illumination & Replication)";
#endif
constexpr std::string_view APP_NAME = "Halo: Reach Community Edition";

} // namespace noble
