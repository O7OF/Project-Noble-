#pragma once

#include <string_view>

namespace noble {

constexpr int VERSION_MAJOR = 0;
constexpr int VERSION_MINOR = 9;
constexpr int VERSION_PATCH = 16;
constexpr int VERSION_REVISION = 22;
#ifdef NOBLE_DEV_BUILD
constexpr std::string_view VERSION_STRING = "0.9.16.22-dev (16:9 Canvas Aspect Ratio Fitting, Heroic Armory Framing & Zoom, Window Settings Live Apply)";
#else
constexpr std::string_view VERSION_STRING = "0.9.16.22 (16:9 Canvas Aspect Ratio Fitting, Heroic Armory Framing & Zoom, Window Settings Live Apply)";
#endif
constexpr std::string_view APP_NAME = "Halo: Reach Community Edition";

} // namespace noble
