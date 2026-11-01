#pragma once

#include <string_view>

namespace noble {

constexpr int VERSION_MAJOR = 0;
constexpr int VERSION_MINOR = 9;
constexpr int VERSION_PATCH = 16;
constexpr int VERSION_REVISION = 14;
#ifdef NOBLE_DEV_BUILD
constexpr std::string_view VERSION_STRING = "0.9.16.14-dev (Elite Lights Dual-Pipeline & 16-Player Campaign Co-op)";
#else
constexpr std::string_view VERSION_STRING = "0.9.16.14 (Elite Lights Dual-Pipeline & 16-Player Campaign Co-op)";
#endif
constexpr std::string_view APP_NAME = "Halo: Reach Community Edition";

} // namespace noble
