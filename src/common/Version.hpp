#pragma once

#include <string_view>

namespace noble {

constexpr int VERSION_MAJOR = 0;
constexpr int VERSION_MINOR = 9;
constexpr int VERSION_PATCH = 16;
constexpr int VERSION_REVISION = 28;
#ifdef NOBLE_DEV_BUILD
constexpr std::string_view VERSION_STRING = "0.9.16.28-dev (Three-Region Server Browser Cluster, Master URL Normalization, Crash-Only Logs)";
#else
constexpr std::string_view VERSION_STRING = "0.9.16.28 (Three-Region Server Browser Cluster, Master URL Normalization, Crash-Only Logs)";
#endif
constexpr std::string_view APP_NAME = "Halo: Reach Community Edition";

} // namespace noble
