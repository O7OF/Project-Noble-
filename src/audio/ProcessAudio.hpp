#pragma once

namespace noble {
namespace ProcessAudio {

// Writes the three real levels to the engine (profile + live mixer) and updates Noble's own menu audio.
// There is deliberately no process/session volume stage: the engine mixes game audio itself, and adding a
// second WASAPI gain multiplied every level twice and made MASTER MUTE silence the game.
// masterMute only affects Noble's own audio (menu music and menu sounds), matching Settings.hpp.
void ApplyAll(float gameVolume, float musicVolume, float otherVolume, bool masterMute);

} // namespace ProcessAudio
} // namespace noble
