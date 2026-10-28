#pragma once

namespace noble {

// Menu feedback sounds, after the partner's Outbreak-Z audio_manager.gd: a click, a softer hover
// tick with a 40 ms cooldown, and a tab sound. The sounds are short tones generated in code (no
// sound files are shipped or read) and played asynchronously with PlaySound. Turned off with the
// "Menu sounds" setting.
namespace MenuSounds {

void Click();
void Tab();
// Plays when the hovered item changes (pass ImGui's item id).
void Hover(unsigned int itemId);

} // namespace MenuSounds
} // namespace noble
