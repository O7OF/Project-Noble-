#pragma once

#include <string>

namespace noble {

// The engine can't change map once started (RR also runs every match as its own process), so
// "return to menu", "play again" and "next mission" start a fresh noble-client.exe and close this one.
namespace Relaunch {

// Arguments that start the current game again (set by main when it launches a map), e.g.
// L"-map 20_sword_slayer -mode multiplayer -variant \"...\"".
void SetCurrentGameArgs(const std::wstring& args);
std::wstring CurrentGameArgs();

// Starts noble-client.exe with these arguments (empty = front-end menu). True if it started.
bool Start(const std::wstring& args);

} // namespace Relaunch
} // namespace noble
