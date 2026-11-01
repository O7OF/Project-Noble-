#pragma once

#include <string_view>

namespace noble {

constexpr int VERSION_MAJOR = 0;
constexpr int VERSION_MINOR = 9;
constexpr int VERSION_PATCH = 10;
constexpr int VERSION_REVISION = 5;
#ifdef NOBLE_DEV_BUILD
constexpr std::string_view VERSION_STRING = "0.9.10.5-dev (Headless Tests)";
#else
constexpr std::string_view VERSION_STRING = "0.9.10.5 (Headless Tests)";
#endif
constexpr std::string_view APP_NAME = "Halo: Reach Community Edition";

} // namespace noble
