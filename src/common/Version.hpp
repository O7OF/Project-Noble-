#pragma once

#include <string_view>

namespace noble {

// Closed-source line. The 0.19.x numbers deliberately never line up with the
// open-source 0.9.16.x builds (see BuildIdentity.hpp for the wire identity).
//
// VERSION_STRING is the single source of truth. The numeric components below are
// parsed from it at compile time so the auto-updater's comparison can never drift
// from the version shown on the main menu (0.19.45 shipped with VERSION_PATCH = 44,
// which made every 0.19.45 install offer v0.19.45 as an update forever).
#ifdef NOBLE_DEV_BUILD
constexpr std::string_view VERSION_STRING = "0.19.67-dev (Closed Source)";
#else
constexpr std::string_view VERSION_STRING = "0.19.67 (Closed Source)";
#endif
constexpr std::string_view APP_NAME = "Project Noble";

namespace version_detail {
// Returns the index-th dot-separated numeric component of a "a.b.c[.d][-suffix ...]" string (0 if absent).
constexpr int Component(std::string_view v, int index) {
    int current = 0;
    int value = 0;
    bool any = false;
    for (const char c : v) {
        if (c >= '0' && c <= '9') {
            if (current == index) { value = value * 10 + (c - '0'); any = true; }
        } else if (c == '.') {
            if (current == index) break;
            ++current;
        } else {
            break;  // '-', ' ', '(' end the numeric part
        }
    }
    return (current == index && any) ? value : 0;
}
} // namespace version_detail

constexpr int VERSION_MAJOR = version_detail::Component(VERSION_STRING, 0);
constexpr int VERSION_MINOR = version_detail::Component(VERSION_STRING, 1);
constexpr int VERSION_PATCH = version_detail::Component(VERSION_STRING, 2);
constexpr int VERSION_REVISION = version_detail::Component(VERSION_STRING, 3);

static_assert(VERSION_MAJOR == 0 && VERSION_MINOR == 19 && VERSION_PATCH > 0,
              "VERSION_STRING must start with a 0.19.x version");

} // namespace noble
