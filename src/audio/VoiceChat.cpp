#include "VoiceChat.hpp"

#include "common/Logger.hpp"

#include <Windows.h>
#include <mmsystem.h>

#include <string>

#pragma comment(lib, "winmm.lib")

namespace noble {
namespace {

std::string Narrow(const wchar_t* text) {
    char buffer[128]{};
    WideCharToMultiByte(CP_UTF8, 0, text, -1, buffer, sizeof(buffer) - 1, nullptr, nullptr);
    return buffer;
}

} // namespace

void ProbeVoiceDevices() {
    const UINT inCount = waveInGetNumDevs();
    const UINT outCount = waveOutGetNumDevs();
    NOBLE_INFO("VoiceChat: {} capture device(s), {} playback device(s)", inCount, outCount);
    if (inCount > 0) {
        WAVEINCAPSW caps{};
        if (waveInGetDevCapsW(0, &caps, sizeof(caps)) == MMSYSERR_NOERROR) {
            NOBLE_INFO("VoiceChat: default capture: {}", Narrow(caps.szPname));
        }
    }
    if (outCount > 0) {
        WAVEOUTCAPSW caps{};
        if (waveOutGetDevCapsW(0, &caps, sizeof(caps)) == MMSYSERR_NOERROR) {
            NOBLE_INFO("VoiceChat: default playback: {}", Narrow(caps.szPname));
        }
    }
}

} // namespace noble
