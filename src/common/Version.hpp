#pragma once

#include <string_view>

namespace noble {

constexpr int VERSION_MAJOR = 0;
constexpr int VERSION_MINOR = 9;
constexpr int VERSION_PATCH = 16;
constexpr int VERSION_REVISION = 20;
#ifdef NOBLE_DEV_BUILD
constexpr std::string_view VERSION_STRING = "0.9.16.20-dev (Window Maximize, Armory Slot Cameras, Spartan Undersuit & Color Fix, 30 Visor Ranks, Embedded Map & CSV)";
#else
constexpr std::string_view VERSION_STRING = "0.9.16.20 (Window Maximize, Armory Slot Cameras, Spartan Undersuit & Color Fix, 30 Visor Ranks, Embedded Map & CSV)";
#endif
constexpr std::string_view APP_NAME = "Halo: Reach Community Edition";

} // namespace noble
