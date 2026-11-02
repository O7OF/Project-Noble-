#include "ProcessAudio.hpp"
#include "ReachAudio.hpp"
#include "engine/ReachHost.hpp"

#include <windows.h>
#include <mmdeviceapi.h>
#include <audiopolicy.h>
#include <endpointvolume.h>
#include <algorithm>
#include <atomic>
#include <cmath>

#pragma comment(lib, "ole32.lib")

namespace noble {
namespace ProcessAudio {

namespace {
std::atomic<float> s_lastVolume{-1.0f};
std::atomic<int> s_lastMute{-1};
}

void ApplyVolume(float gameVolume, bool masterMute) {
    const float clamped = std::clamp(gameVolume, 0.0f, 1.0f);
    const int muteInt = masterMute ? 1 : 0;
    if (std::fabs(s_lastVolume.load() - clamped) < 1e-4f && s_lastMute.load() == muteInt) {
        return;
    }
    s_lastVolume.store(clamped);
    s_lastMute.store(muteInt);

    HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    IMMDeviceEnumerator* enumerator = nullptr;
    hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_ALL,
                          __uuidof(IMMDeviceEnumerator), reinterpret_cast<void**>(&enumerator));
    if (SUCCEEDED(hr) && enumerator) {
        IMMDevice* device = nullptr;
        hr = enumerator->GetDefaultAudioEndpoint(eRender, eConsole, &device);
        if (SUCCEEDED(hr) && device) {
            IAudioSessionManager2* sessionManager = nullptr;
            hr = device->Activate(__uuidof(IAudioSessionManager2), CLSCTX_ALL, nullptr,
                                  reinterpret_cast<void**>(&sessionManager));
            if (SUCCEEDED(hr) && sessionManager) {
                ISimpleAudioVolume* simpleVolume = nullptr;
                hr = sessionManager->GetSimpleAudioVolume(nullptr, 0, &simpleVolume);
                if (SUCCEEDED(hr) && simpleVolume) {
                    simpleVolume->SetMasterVolume(clamped, nullptr);
                    simpleVolume->SetMute(masterMute ? TRUE : FALSE, nullptr);
                    simpleVolume->Release();
                }
                sessionManager->Release();
            }
            device->Release();
        }
        enumerator->Release();
    }
}

void ApplyAll(float gameVolume, float musicVolume, float otherVolume, bool masterMute) {
    ApplyVolume(gameVolume, masterMute);
    ReachAudio::SetMusicVolume(musicVolume);
    ReachAudio::SetMuted(masterMute);
    ReachHost::SetRuntimeVolumes(gameVolume, musicVolume, otherVolume);
}

} // namespace ProcessAudio
} // namespace noble

