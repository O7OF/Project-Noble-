#pragma once

#include <string_view>

namespace noble {

constexpr int VERSION_MAJOR = 0;
constexpr int VERSION_MINOR = 9;
constexpr int VERSION_PATCH = 5;
#ifdef NOBLE_DEV_BUILD
constexpr std::string_view VERSION_STRING = "0.9.5-dev (Push-to-Talk Bindings & Music Names)";
#else
constexpr std::string_view VERSION_STRING = "0.9.5 (Push-to-Talk Bindings & Music Names)";
#endif
constexpr std::string_view APP_NAME = "Halo: Reach Community Edition";

} // namespace noble
