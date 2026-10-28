#pragma once

namespace noble {

// Menu feedback sounds. Halo: Reach's own UI sounds (cursor, A/B buttons, bumpers) come from the
// player's MCC install through ReachAudio; until they are decoded, or when MCC's bank or vgmstream is
// missing, short tones generated in code play instead (PlaySound, nothing shipped). Hover has a 40 ms
// cooldown, after the partner's Outbreak-Z audio_manager.gd. Turned off with the "Menu sounds" setting.
namespace MenuSounds {

void Click();              // accept (A / Enter)
void Back();               // back (B / Esc)
void Scroll();             // moving up/down a list with keys or the d-pad
void Change();             // changing a value left/right
void Tab(int direction = 1); // switching tabs (bumpers); -1 = left
void Error();
// Plays when the hovered item changes (pass ImGui's item id).
void Hover(unsigned int itemId);

} // namespace MenuSounds
} // namespace noble
