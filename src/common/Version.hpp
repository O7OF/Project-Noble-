#pragma once

#include <string_view>

namespace noble {

constexpr int VERSION_MAJOR = 0;
constexpr int VERSION_MINOR = 9;
constexpr int VERSION_PATCH = 12;
constexpr int VERSION_REVISION = 1;
#ifdef NOBLE_DEV_BUILD
constexpr std::string_view VERSION_STRING = "0.9.12.1-dev (Lights & Extended Visors)";
#else
constexpr std::string_view VERSION_STRING = "0.9.12.1 (Lights & Extended Visors)";
#endif
constexpr std::string_view APP_NAME = "Halo: Reach Community Edition";

} // namespace noble