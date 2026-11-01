#pragma once

#include <string_view>

namespace noble {

constexpr int VERSION_MAJOR = 0;
constexpr int VERSION_MINOR = 9;
constexpr int VERSION_PATCH = 10;
constexpr int VERSION_REVISION = 4;
#ifdef NOBLE_DEV_BUILD
constexpr std::string_view VERSION_STRING = "0.9.10.4-dev (Voice Bind Coverage)";
#else
constexpr std::string_view VERSION_STRING = "0.9.10.4 (Voice Bind Coverage)";
#endif
constexpr std::string_view APP_NAME = "Halo: Reach Community Edition";

} // namespace noble
