#include "MenuSounds.hpp"
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

// Builds a 16-bit mono PCM WAV in memory: a short sine "blip" sliding from f0 to f1 with a fast
// attack and exponential decay.
std::vector<uint8_t> MakeBlip(float f0, float f1, float seconds, float volume) {
    const uint32_t rate = 22050;
    const uint32_t samples = static_cast<uint32_t>(seconds * rate);
    const uint32_t dataBytes = samples * 2;
    std::vector<uint8_t> wav(44 + dataBytes);
    auto put32 = [&](size_t off, uint32_t v) { memcpy(&wav[off], &v, 4); };
    auto put16 = [&](size_t off, uint16_t v) { memcpy(&wav[off], &v, 2); };
    memcpy(&wav[0], "RIFF", 4);
    put32(4, 36 + dataBytes);
    memcpy(&wav[8], "WAVEfmt ", 8);
    put32(16, 16);
    put16(20, 1);       // PCM
    put16(22, 1);       // mono
    put32(24, rate);
    put32(28, rate * 2);
    put16(32, 2);
    put16(34, 16);
    memcpy(&wav[36], "data", 4);
    put32(40, dataBytes);
    double phase = 0.0;
    for (uint32_t i = 0; i < samples; ++i) {
        const float t = static_cast<float>(i) / static_cast<float>(samples);
        const float freq = f0 + (f1 - f0) * t;
        phase += 2.0 * 3.14159265358979 * freq / rate;
        const float attack = std::min(1.0f, static_cast<float>(i) / (rate * 0.002f));
        const float env = attack * std::exp(-5.0f * t);
        const int16_t v = static_cast<int16_t>(std::sin(phase) * env * volume * 32767.0f);
        put16(44 + i * 2, static_cast<uint16_t>(v));
    }
    return wav;
}

const std::vector<uint8_t>& ClickWav() {
    static const std::vector<uint8_t> wav = MakeBlip(1400.0f, 900.0f, 0.06f, 0.35f);
    return wav;
}
const std::vector<uint8_t>& HoverWav() {
    static const std::vector<uint8_t> wav = MakeBlip(2200.0f, 2000.0f, 0.025f, 0.12f);
    return wav;
}
const std::vector<uint8_t>& TabWav() {
    static const std::vector<uint8_t> wav = MakeBlip(900.0f, 1300.0f, 0.07f, 0.30f);
    return wav;
}

bool Enabled() {
    return Settings::Get().Snapshot().menuSounds;
}

void Play(const std::vector<uint8_t>& wav) {
    if (!Enabled()) return;
    PlaySoundW(reinterpret_cast<LPCWSTR>(wav.data()), nullptr, SND_MEMORY | SND_ASYNC | SND_NODEFAULT);
}

std::atomic<unsigned int> s_lastHovered{0};
std::atomic<ULONGLONG> s_lastHoverTime{0};

} // namespace

void Click() { Play(ClickWav()); }
void Tab() { Play(TabWav()); }

void Hover(unsigned int itemId) {
    if (itemId == 0 || s_lastHovered.exchange(itemId) == itemId) return;
    const ULONGLONG now = GetTickCount64();
    if (now - s_lastHoverTime.load() < 40) return;  // 40 ms cooldown, as in audio_manager.gd
    s_lastHoverTime = now;
    Play(HoverWav());
}

} // namespace MenuSounds
} // namespace noble
