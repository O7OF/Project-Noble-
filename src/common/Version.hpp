#pragma once

#include <string_view>

namespace noble {

constexpr int VERSION_MAJOR = 0;
constexpr int VERSION_MINOR = 9;
constexpr int VERSION_PATCH = 16;
constexpr int VERSION_REVISION = 16;
#ifdef NOBLE_DEV_BUILD
constexpr std::string_view VERSION_STRING = "0.9.16.16-dev (Live Career & Armory Stats Display, Stable Device ID Persistence)";
#else
constexpr std::string_view VERSION_STRING = "0.9.16.16 (Live Career & Armory Stats Display, Stable Device ID Persistence)";
#endif
constexpr std::string_view APP_NAME = "Halo: Reach Community Edition";

} // namespace noble
