#pragma once

namespace noble {
namespace ProcessAudio {

// Controls the entire process audio endpoint in Windows (WASAPI / SimpleAudioVolume).
// This directly changes in-game audio volume and muting in real time mid-match.
void ApplyVolume(float gameVolume, bool masterMute);

// Updates ReachHost profile buffer (+0xA5C, +0xA60, +0xA64), ReachAudio music/sfx, and Windows process audio.
void ApplyAll(float gameVolume, float musicVolume, float otherVolume, bool masterMute);

} // namespace ProcessAudio
} // namespace noble

