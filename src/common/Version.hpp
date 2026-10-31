#pragma once

#include <string_view>

namespace noble {

constexpr int VERSION_MAJOR = 0;
constexpr int VERSION_MINOR = 8;
constexpr int VERSION_PATCH = 9;
#ifdef NOBLE_DEV_BUILD
constexpr std::string_view VERSION_STRING = "0.8.9-dev (Workshop Maps & Prompts)";
#else
constexpr std::string_view VERSION_STRING = "0.8.9 (Workshop Maps & Prompts)";
#endif
constexpr std::string_view APP_NAME = "Halo: Reach Community Edition";

} // namespace noble
