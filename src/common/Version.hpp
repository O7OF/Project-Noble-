#pragma once

#include <string_view>

namespace noble {

constexpr int VERSION_MAJOR = 0;
constexpr int VERSION_MINOR = 9;
constexpr int VERSION_PATCH = 9;
constexpr int VERSION_REVISION = 2;
#ifdef NOBLE_DEV_BUILD
constexpr std::string_view VERSION_STRING = "0.9.9.2-dev (Voice Channel Binds)";
#else
constexpr std::string_view VERSION_STRING = "0.9.9.2 (Voice Channel Binds)";
#endif
constexpr std::string_view APP_NAME = "Halo: Reach Community Edition";

} // namespace noble
