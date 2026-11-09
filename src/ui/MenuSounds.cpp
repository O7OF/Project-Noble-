#include "MenuSounds.hpp"
#include "MenuSoundData.inc"
#include "audio/ReachAudio.hpp"
#include "common/Settings.hpp"


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
  const auto &s = Settings::Get().Snapshot();
  return s.menuSounds && !s.masterMute && s.volumeOther > 0.001f;
}

void PlayEmbedded(const uint8_t *wavBytes, size_t size, ReachAudio::Sfx sfx) {
  if (!Enabled())
    return;
  const float volume = Settings::Get().Snapshot().volumeOther;
  // Play through ReachAudio's XAudio2 source voice for volume-scaled mixing
  if (ReachAudio::PlaySfx(sfx, volume))
    return;
  // Fallback: direct in-memory play via WinMM without touching disk
  if (wavBytes && size > 0) {
    PlaySoundW(reinterpret_cast<LPCWSTR>(wavBytes), nullptr,
               SND_MEMORY | SND_ASYNC | SND_NODEFAULT);
  }
}

std::atomic<unsigned int> s_lastHovered{0};
std::atomic<ULONGLONG> s_lastHoverTime{0};
std::atomic<ULONGLONG> s_lastClickTime{0};
std::atomic<ULONGLONG> s_lastBackTime{0};
std::atomic<ULONGLONG> s_lastScrollTime{0};
std::atomic<ULONGLONG> s_lastChangeTime{0};
std::atomic<ULONGLONG> s_lastTabTime{0};
std::atomic<ULONGLONG> s_lastErrorTime{0};

bool Debounce(std::atomic<ULONGLONG> &lastTime, ULONGLONG minIntervalMs = 75) {
  const ULONGLONG now = GetTickCount64();
  ULONGLONG prev = lastTime.load(std::memory_order_relaxed);
  if (now - prev < minIntervalMs)
    return false;
  return lastTime.compare_exchange_strong(prev, now, std::memory_order_relaxed);
}

} // namespace

void Click() {
  if (!Debounce(s_lastClickTime, 80))
    return;
  PlayEmbedded(kSoundClick, kSoundClickSize, ReachAudio::Sfx::Accept);
}

void Back() {
  if (!Debounce(s_lastBackTime, 80))
    return;
  PlayEmbedded(kSoundBack, kSoundBackSize, ReachAudio::Sfx::Back);
}

void Scroll() {
  if (!Debounce(s_lastScrollTime, 60))
    return;
  PlayEmbedded(kSoundScroll, kSoundScrollSize, ReachAudio::Sfx::CursorVertical);
}

void Change() {
  if (!Debounce(s_lastChangeTime, 60))
    return;
  PlayEmbedded(kSoundChange, kSoundChangeSize,
               ReachAudio::Sfx::CursorHorizontal);
}

void Tab(int direction) {
  if (!Debounce(s_lastTabTime, 80))
    return;
  PlayEmbedded(kSoundTab, kSoundTabSize,
               direction < 0 ? ReachAudio::Sfx::BumperLeft
                             : ReachAudio::Sfx::BumperRight);
}

void Error() {
  if (!Debounce(s_lastErrorTime, 100))
    return;
  PlayEmbedded(kSoundError, kSoundErrorSize, ReachAudio::Sfx::Error);
}

void Hover(unsigned int itemId) {
  if (itemId == 0 || s_lastHovered.exchange(itemId) == itemId)
    return;
  const ULONGLONG now = GetTickCount64();
  if (now - s_lastHoverTime.load() < 60)
    return; // 60 ms cooldown
  s_lastHoverTime = now;
  PlayEmbedded(kSoundScroll, kSoundScrollSize, ReachAudio::Sfx::CursorVertical);
}

} // namespace MenuSounds
} // namespace noble
