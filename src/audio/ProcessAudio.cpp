#include "ProcessAudio.hpp"
#include "ReachAudio.hpp"
#include "engine/ReachHost.hpp"

#include <algorithm>

namespace noble {
namespace ProcessAudio {

// Game audio is mixed by haloreach.dll. Noble used to also set this process's WASAPI session volume to
// GAME, so every level was scaled twice (GAME 0.5 gave 0.25 to music/dialogue), a mute at launch wrote
// 0 into the engine profile and could leave the running mixer silent, and mid-match MUSIC/OTHER changes
// never reached the live mixer. Write the three real levels straight to the engine instead.
// masterMute is Noble's own menu audio only (see Settings.hpp), so it must not touch the game mix.
void ApplyAll(float gameVolume, float musicVolume, float otherVolume, bool masterMute) {
    const float game = std::clamp(gameVolume, 0.0f, 1.0f);
    const float music = std::clamp(musicVolume, 0.0f, 1.0f);
    const float other = std::clamp(otherVolume, 0.0f, 1.0f);
    ReachAudio::SetMusicVolume(music);
    ReachAudio::SetMuted(masterMute);
    ReachHost::SetRuntimeVolumes(game, music, other);
}

} // namespace ProcessAudio
} // namespace noble
