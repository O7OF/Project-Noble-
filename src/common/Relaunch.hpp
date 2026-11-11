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

// Starts a match as a child of the container window without terminating the container.
// Returns the child process HANDLE so the container can monitor when the match ends.
bool StartChildMatch(const std::wstring& args, void** outProcessHandle = nullptr, unsigned long* outPid = nullptr);

// Starts noble-client.exe with these arguments and ends THIS process straight away (its windows are hidden
// first and the log is flushed), so there is never a second Noble client open next to the new one.
// Only returns (false) if the new process could not be started; the caller then keeps running.
// In container window mode (the arguments carry -container-hwnd) it does not start anything itself: the
// successor match has to live inside the launcher's window and only the launcher may start it, so the
// arguments are recorded for it (RequestContainerRelaunch) and this process ends. Returns false if that
// request could not be written, so the caller keeps running.
bool HandOff(const std::wstring& args);

// Container window mode only. Records the arguments of the match that should replace this one and returns
// true; the launcher window starts it as soon as this process is gone (see TakeContainerRelaunch). A
// hosted child cannot start its own successor: the launcher tracks one match process and would otherwise
// think the match had ended and bring its own menu back on top of the new game.
bool RequestContainerRelaunch(const std::wstring& args);

// Launcher side: the successor's arguments if a match process asked for one, then forgets the request.
// Empty when the match simply ended.
std::wstring TakeContainerRelaunch();

// Launcher side: drop a request left behind by a match that never used it (called when the menu starts).
void ClearContainerRelaunch();

} // namespace Relaunch
} // namespace noble
