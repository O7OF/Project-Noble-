#pragma once

#include <string_view>

namespace noble {

constexpr int VERSION_MAJOR = 0;
constexpr int VERSION_MINOR = 9;
constexpr int VERSION_PATCH = 13;
constexpr int VERSION_REVISION = 2;
#ifdef NOBLE_DEV_BUILD
constexpr std::string_view VERSION_STRING = "0.9.13.2-dev (Host Mode Transfer)";
#else
constexpr std::string_view VERSION_STRING = "0.9.13.2 (Host Mode Transfer)";
#endif
constexpr std::string_view APP_NAME = "Halo: Reach Community Edition";

} // namespace noble
