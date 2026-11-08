#pragma once

#include <string_view>

namespace noble {

// Closed-source line. The 0.19.x numbers deliberately never line up with the
// open-source 0.9.16.x builds (see BuildIdentity.hpp for the wire identity).
constexpr int VERSION_MAJOR = 0;
constexpr int VERSION_MINOR = 19;
constexpr int VERSION_PATCH = 22;
constexpr int VERSION_REVISION = 0;
#ifdef NOBLE_DEV_BUILD
constexpr std::string_view VERSION_STRING = "0.19.22-dev (Closed Source)";
#else
constexpr std::string_view VERSION_STRING = "0.19.22 (Closed Source)";
#endif
constexpr std::string_view APP_NAME = "Project Noble";

} // namespace noble
