#pragma once

#include <imgui.h>

#include <cstdint>
#include <string>

namespace noble {

// Text chat and the "who's talking" panel, drawn over the pre-game lobby and over a match.
//
// Voice: a panel at the top left lists who is talking right now, with your own row first while you transmit. Muted players
// never appear. Chat: T, Y and P (rebindable) open a line for team, everyone or party. It shows who the line goes to, what
// you're typing and the log. New messages also show for a few seconds with the line closed.
//
// While the line is open the game and the menus get no keyboard. Controllers can read the log but can't type yet.
namespace SocialOverlay {

void Start();    // once at startup: subscribes to the relay's chat frames

// Every frame, before the UI is built. allowOpen: nothing else owns the keyboard, so the chat keys may open the line.
void Poll(bool allowOpen);
bool WantsFrame();       // something is on screen, so a match that otherwise draws no UI must draw this frame
bool IsTyping();         // the line is open: the game and menus must take no keys
bool OwnsEscape();       // Esc is closing the line (or just did), so it must not also open the pause menu
void Draw(float voiceTop, float chatX, float chatBottom);   // into the current ImGui frame; positions are in 1080p units

// The icon on a player's row: nothing when quiet, a pulsing mic while talking, a crossed-out mic when you've muted them.
void DrawPlayerVoiceIcon(ImDrawList* dl, ImVec2 center, float scale, uint64_t machineId, bool local);

// Names for the voice panel, from the session roster. The id is the player's machine id.
void SetPlayerName(uint64_t machineId, const std::string& name);

} // namespace SocialOverlay
} // namespace noble
