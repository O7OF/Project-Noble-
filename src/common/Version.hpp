#pragma once

#include <string_view>

namespace noble {

constexpr int VERSION_MAJOR = 0;
constexpr int VERSION_MINOR = 9;
constexpr int VERSION_PATCH = 16;
constexpr int VERSION_REVISION = 17;
#ifdef NOBLE_DEV_BUILD
constexpr std::string_view VERSION_STRING = "0.9.16.17-dev (Embedded UI Audio, Runtime Volume Scaling, Clean Release Packaging)";
#else
constexpr std::string_view VERSION_STRING = "0.9.16.17 (Embedded UI Audio, Runtime Volume Scaling, Clean Release Packaging)";
#endif
constexpr std::string_view APP_NAME = "Halo: Reach Community Edition";

} // namespace noble
