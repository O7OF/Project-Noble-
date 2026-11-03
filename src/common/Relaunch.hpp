#pragma once

#include <string>

namespace noble {

// The engine can't change map once started (the engine also runs every match as its own process), so
// "return to menu", "play again" and "next mission" start a fresh noble-client.exe and close this one.
namespace Relaunch {

// Arguments that start the current game again (set by main when it launches a map), e.g.
// L"-map 20_sword_slayer -mode multiplayer -variant \"...\"".
void SetCurrentGameArgs(const std::wstring& args);
std::wstring CurrentGameArgs();

// Starts noble-client.exe with these arguments (empty = front-end menu). True if it started.
// `outPid` (optional) receives the new process id, so a caller can wait for its window to appear before hiding.
bool Start(const std::wstring& args, unsigned long* outPid = nullptr);

// Starts noble-client.exe with these arguments and ends THIS process straight away (its windows are hidden
// first and the log is flushed), so there is never a second Noble client open next to the new one.
// Only returns (false) if the new process could not be started; the caller then keeps running.
bool HandOff(const std::wstring& args);

} // namespace Relaunch
} // namespace noble
