#pragma once

#include <string_view>

namespace noble {

constexpr int VERSION_MAJOR = 0;
constexpr int VERSION_MINOR = 9;
constexpr int VERSION_PATCH = 6;
#ifdef NOBLE_DEV_BUILD
constexpr std::string_view VERSION_STRING = "0.9.6-dev (Vehicle 1st-Person Camera & FOV Modifiers)";
#else
constexpr std::string_view VERSION_STRING = "0.9.6 (Vehicle 1st-Person Camera & FOV Modifiers)";
#endif
constexpr std::string_view APP_NAME = "Halo: Reach Community Edition";

} // namespace noble
