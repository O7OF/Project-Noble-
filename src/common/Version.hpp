#pragma once

#include <string_view>

namespace noble {

constexpr int VERSION_MAJOR = 0;
constexpr int VERSION_MINOR = 9;
constexpr int VERSION_PATCH = 11;
constexpr int VERSION_REVISION = 0;
#ifdef NOBLE_DEV_BUILD
constexpr std::string_view VERSION_STRING = "0.9.11-dev (Game Type Picker)";
#else
constexpr std::string_view VERSION_STRING = "0.9.11 (Game Type Picker)";
#endif
constexpr std::string_view APP_NAME = "Halo: Reach Community Edition";

} // namespace noble
