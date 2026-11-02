#pragma once

#include <string_view>

namespace noble {

constexpr int VERSION_MAJOR = 0;
constexpr int VERSION_MINOR = 9;
constexpr int VERSION_PATCH = 16;
constexpr int VERSION_REVISION = 19;
#ifdef NOBLE_DEV_BUILD
constexpr std::string_view VERSION_STRING = "0.9.16.19-dev (Elite Crash Fix, Multi-Region Server Browser, NAT/UPnP Diagnostics, 16p Firefight & 128p Custom/Forge Expansion)";
#else
constexpr std::string_view VERSION_STRING = "0.9.16.19 (Elite Crash Fix, Multi-Region Server Browser, NAT/UPnP Diagnostics, 16p Firefight & 128p Custom/Forge Expansion)";
#endif
constexpr std::string_view APP_NAME = "Halo: Reach Community Edition";

} // namespace noble
