#include "MenuSounds.hpp"
#include "common/Settings.hpp"
#include "audio/ReachAudio.hpp"
#include "MenuSoundData.inc"

#include <windows.h>
#include <mmsystem.h>

#include <algorithm>
#include <atomic>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <vector>

#pragma comment(lib, "winmm.lib")

namespace noble {
namespace MenuSounds {

namespace {

bool Enabled() {
    const auto& s = Settings::Get().Snapshot();
    return s.menuSounds && !s.masterMute && s.volumeOther > 0.001f;
}

void PlayEmbedded(const uint8_t* wavBytes, size_t size, ReachAudio::Sfx sfx) {
    if (!Enabled()) return;
    const float volume = Settings::Get().Snapshot().volumeOther;
    // Play through ReachAudio's XAudio2 source voice for volume-scaled mixing
    if (ReachAudio::PlaySfx(sfx, volume)) return;
    // Fallback: direct in-memory play via WinMM without touching disk
    if (wavBytes && size > 0) {
        PlaySoundW(reinterpret_cast<LPCWSTR>(wavBytes), nullptr, SND_MEMORY | SND_ASYNC | SND_NODEFAULT);
    }
}

std::atomic<unsigned int> s_lastHovered{0};
std::atomic<ULONGLONG> s_lastHoverTime{0};

} // namespace

void Click() { PlayEmbedded(kSoundClick, kSoundClickSize, ReachAudio::Sfx::Accept); }
void Back() { PlayEmbedded(kSoundBack, kSoundBackSize, ReachAudio::Sfx::Back); }
void Scroll() { PlayEmbedded(kSoundScroll, kSoundScrollSize, ReachAudio::Sfx::CursorVertical); }
void Change() { PlayEmbedded(kSoundChange, kSoundChangeSize, ReachAudio::Sfx::CursorHorizontal); }
void Tab(int direction) {
    PlayEmbedded(kSoundTab, kSoundTabSize, direction < 0 ? ReachAudio::Sfx::BumperLeft : ReachAudio::Sfx::BumperRight);
}
void Error() { PlayEmbedded(kSoundError, kSoundErrorSize, ReachAudio::Sfx::Error); }

void Hover(unsigned int itemId) {
    if (itemId == 0 || s_lastHovered.exchange(itemId) == itemId) return;
    const ULONGLONG now = GetTickCount64();
    if (now - s_lastHoverTime.load() < 40) return;  // 40 ms cooldown
    s_lastHoverTime = now;
    PlayEmbedded(kSoundScroll, kSoundScrollSize, ReachAudio::Sfx::CursorVertical);
}

} // namespace MenuSounds
} // namespace noble
