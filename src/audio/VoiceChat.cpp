#include "network/NetworkTransport.hpp"
#include "VoiceChat.hpp"

#include "common/Logger.hpp"
#include "common/Settings.hpp"
#include "engine/KeyBindings.hpp"
#include "engine/PadLayouts.hpp"

#include <Windows.h>
#include <initguid.h>
#include <Audioclient.h>
#include <mmdeviceapi.h>
#include <functiondiscoverykeys_devpkey.h>
#include <shellapi.h>
#include <Xinput.h>
#include <wrl/client.h>

#include <imgui.h>
#include <opus.h>

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cmath>
#include <cstring>
#include <fstream>
#include <map>
#include <mutex>
#include <thread>

namespace noble {
namespace {

using Microsoft::WRL::ComPtr;

constexpr int kSampleRate = 48000;
constexpr int kFrameSamples = 960;              // 20 ms
constexpr int kMaxPayload = 400;                // Opus output cap; 24 kbps needs about 60 bytes a frame
constexpr int kJitterMinFrames = 3;             // 60 ms
constexpr int kJitterMaxFrames = 6;             // 120 ms
constexpr int kBufferedMaxFrames = 12;
constexpr int kMaxPeers = 64;
constexpr float kOpenMicThreshold = 0.01f;      // -40 dBFS, as RMS of a full-scale 1.0 signal
constexpr int kOpenMicHangFrames = 15;          // keeps the end of a word: 300 ms
constexpr REFERENCE_TIME kStreamBuffer = 2000000;  // 200 ms, in 100 ns units

// Wire layout, little-endian: "NVOX", sender id (the relay overwrites it), sequence, flags, channel, team, reserved, Opus.
// A game packet starts with the sender's machine id, and those are always odd, so 'N' (0x4E) can't start one.
constexpr size_t kHeaderSize = 18;
constexpr uint8_t kFlagEnd = 0x02;              // the talker stopped; no payload

struct Config {
    bool enabled = false;
    std::string input, output;
    float volume = 1.0f;
    bool openMic = false;
    int bitrateKbps = 20;
    int channel = 0;
    bool masterMute = false;
    int pttKey = 0;
    int pttPartyKey = 0;
    int pttTeamKey = 0;
    int pttGlobalKey = 0;
    int pttPad = 0;
    int pttPartyPad = 0;
    int pttTeamPad = 0;
    int pttGlobalPad = 0;
    std::array<int, 6> paddleKeys{};
};

struct BufferedFrame {
    uint16_t seq = 0;
    uint8_t flags = 0;
    std::vector<uint8_t> payload;
};

struct Peer {
    OpusDecoder* decoder = nullptr;
    std::vector<BufferedFrame> buffer;   // sorted by sequence
    uint16_t next = 0;
    bool playing = false;
    int target = kJitterMinFrames;
    int misses = 0;
    uint64_t lastUnderrunMs = 0;
    uint64_t lastHeardMs = 0;
    uint64_t lastAudioMs = 0;
};

struct Stats {
    std::atomic<uint64_t> sent{ 0 }, received{ 0 }, played{ 0 }, late{ 0 }, concealed{ 0 }, skipped{ 0 }, filtered{ 0 };
    std::atomic<uint64_t> captured{ 0 }, rendered{ 0 }, underruns{ 0 }, sentBytes{ 0 };
};

struct Test {
    bool active = false;
    bool tone = true;
    bool forceTransmit = true;
    bool silentOutput = true;
    int seconds = 10;
    std::string relayUrl;
    bool isHost = true;
    uint64_t machineId = 0;
    std::vector<int16_t>* wavTap = nullptr;
};

std::mutex s_mutex;   // peers, the per-player controls and the device lists
std::map<uint64_t, Peer> s_peers;
std::map<uint64_t, bool> s_muted;
std::map<uint64_t, float> s_peerVolume;
std::vector<VoiceChat::Device> s_inputs, s_outputs;
std::atomic<bool> s_wantDevices{ true };

std::thread s_thread;
std::atomic<bool> s_started{ false };
std::atomic<bool> s_stop{ false };
std::atomic<bool> s_transmitting{ false };
std::atomic<bool> s_pushToTalkBlocked{ false };
std::atomic<int> s_localTeam{ -1 };
std::atomic<uint64_t> s_mixPeak{ 0 };
std::atomic<int> s_testBitrate{ 16 };   // the relay test walks 16, 20, 24 to show the setting reaches a running encoder
Stats s_stats;
Test s_test;

uint64_t NowMs() { return GetTickCount64(); }

std::string Narrow(const wchar_t* text) {
    if (!text) return {};
    const int len = WideCharToMultiByte(CP_UTF8, 0, text, -1, nullptr, 0, nullptr, nullptr);
    if (len <= 1) return {};
    std::string out(static_cast<size_t>(len - 1), '\0');
    WideCharToMultiByte(CP_UTF8, 0, text, -1, out.data(), len, nullptr, nullptr);
    return out;
}

std::wstring Widen(const std::string& text) {
    if (text.empty()) return {};
    const int len = MultiByteToWideChar(CP_UTF8, 0, text.c_str(), -1, nullptr, 0);
    if (len <= 1) return {};
    std::wstring out(static_cast<size_t>(len - 1), L'\0');
    MultiByteToWideChar(CP_UTF8, 0, text.c_str(), -1, out.data(), len);
    return out;
}

std::string FriendlyName(IMMDevice* device) {
    ComPtr<IPropertyStore> props;
    if (FAILED(device->OpenPropertyStore(STGM_READ, &props))) return "unknown device";
    PROPVARIANT value;
    PropVariantInit(&value);
    std::string name = "unknown device";
    if (SUCCEEDED(props->GetValue(PKEY_Device_FriendlyName, &value)) && value.vt == VT_LPWSTR) name = Narrow(value.pwszVal);
    PropVariantClear(&value);
    return name;
}

ComPtr<IMMDeviceEnumerator> Enumerator() {
    ComPtr<IMMDeviceEnumerator> e;
    CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_ALL, IID_PPV_ARGS(&e));
    return e;
}

std::vector<VoiceChat::Device> ListDevices(EDataFlow flow) {
    std::vector<VoiceChat::Device> out;
    ComPtr<IMMDeviceEnumerator> e = Enumerator();
    ComPtr<IMMDeviceCollection> all;
    if (!e || FAILED(e->EnumAudioEndpoints(flow, DEVICE_STATE_ACTIVE, &all))) return out;
    UINT count = 0;
    all->GetCount(&count);
    for (UINT i = 0; i < count; ++i) {
        ComPtr<IMMDevice> device;
        LPWSTR id = nullptr;
        if (FAILED(all->Item(i, &device)) || FAILED(device->GetId(&id))) continue;
        out.push_back({ Narrow(id), FriendlyName(device.Get()) });
        CoTaskMemFree(id);
    }
    return out;
}

// An open WASAPI stream in Reach-independent shared mode. Windows converts to and from 48 kHz mono 16-bit.
struct Stream {
    ComPtr<IAudioClient> client;
    ComPtr<IAudioCaptureClient> capture;
    ComPtr<IAudioRenderClient> render;
    UINT32 bufferFrames = 0;
    std::string name;
    uint64_t retryAtMs = 0;

    bool Open() const { return client != nullptr; }
    void Close() {
        if (client) client->Stop();
        capture.Reset();
        render.Reset();
        client.Reset();
        bufferFrames = 0;
    }
    HRESULT OpenOn(EDataFlow flow, const std::string& deviceId) {
        Close();
        ComPtr<IMMDeviceEnumerator> e = Enumerator();
        if (!e) return E_FAIL;
        ComPtr<IMMDevice> device;
        if (!deviceId.empty()) e->GetDevice(Widen(deviceId).c_str(), &device);   // a device that has gone falls back to the default
        if (!device) {
            const HRESULT hr = e->GetDefaultAudioEndpoint(flow, eConsole, &device);
            if (FAILED(hr)) return hr;
        }
        name = FriendlyName(device.Get());
        ComPtr<IAudioClient> c;
        HRESULT hr = device->Activate(__uuidof(IAudioClient), CLSCTX_ALL, nullptr, reinterpret_cast<void**>(c.GetAddressOf()));
        if (FAILED(hr)) return hr;
        WAVEFORMATEX format{};
        format.wFormatTag = WAVE_FORMAT_PCM;
        format.nChannels = 1;
        format.nSamplesPerSec = kSampleRate;
        format.wBitsPerSample = 16;
        format.nBlockAlign = 2;
        format.nAvgBytesPerSec = kSampleRate * 2;
        hr = c->Initialize(AUDCLNT_SHAREMODE_SHARED, AUDCLNT_STREAMFLAGS_AUTOCONVERTPCM | AUDCLNT_STREAMFLAGS_SRC_DEFAULT_QUALITY,
                           kStreamBuffer, 0, &format, nullptr);
        if (FAILED(hr)) return hr;
        c->GetBufferSize(&bufferFrames);
        hr = flow == eCapture ? c->GetService(IID_PPV_ARGS(&capture)) : c->GetService(IID_PPV_ARGS(&render));
        if (FAILED(hr)) return hr;
        if (flow == eRender) {   // start from silence so the first padding reading is meaningful
            BYTE* data = nullptr;
            if (SUCCEEDED(render->GetBuffer(kFrameSamples, &data))) render->ReleaseBuffer(kFrameSamples, AUDCLNT_BUFFERFLAGS_SILENT);
        }
        hr = c->Start();
        if (FAILED(hr)) { capture.Reset(); render.Reset(); return hr; }
        client = c;
        return S_OK;
    }
};

// The capture side's small ring: 200 ms is far more than one poll can leave behind.
class Ring {
public:
    void Push(const int16_t* samples, size_t count) {
        for (size_t i = 0; i < count; ++i) {
            m_data[m_end] = samples ? samples[i] : 0;
            m_end = (m_end + 1) % m_data.size();
            if (m_size == m_data.size()) m_start = (m_start + 1) % m_data.size();
            else ++m_size;
        }
    }
    bool PopFrame(int16_t* out) {
        if (m_size < kFrameSamples) return false;
        for (int i = 0; i < kFrameSamples; ++i) {
            out[i] = m_data[m_start];
            m_start = (m_start + 1) % m_data.size();
        }
        m_size -= kFrameSamples;
        return true;
    }
    void Clear() { m_start = m_end = m_size = 0; }
private:
    std::array<int16_t, kFrameSamples * 10> m_data{};
    size_t m_start = 0, m_end = 0, m_size = 0;
};

void WriteWav(const std::string& path, const std::vector<int16_t>& samples) {
    std::ofstream f(path, std::ios::binary);
    if (!f) { NOBLE_WARN("VoiceChat: can't write {}", path); return; }
    const uint32_t dataBytes = static_cast<uint32_t>(samples.size() * 2);
    const auto u32 = [&f](uint32_t v) { f.write(reinterpret_cast<const char*>(&v), 4); };
    const auto u16 = [&f](uint16_t v) { f.write(reinterpret_cast<const char*>(&v), 2); };
    f.write("RIFF", 4); u32(36 + dataBytes); f.write("WAVEfmt ", 8); u32(16); u16(1); u16(1);
    u32(kSampleRate); u32(kSampleRate * 2); u16(2); u16(16);
    f.write("data", 4); u32(dataBytes);
    f.write(reinterpret_cast<const char*>(samples.data()), dataBytes);
}

// The test signal: two harmonics with a 4 Hz syllable-like envelope. Nobody's voice ever ends up in a committed file.
void FillTone(int16_t* out, int64_t& position) {
    constexpr double kPi = 3.14159265358979323846;
    for (int i = 0; i < kFrameSamples; ++i, ++position) {
        const double t = static_cast<double>(position) / kSampleRate;
        const double envelope = 0.6 + 0.4 * std::sin(2.0 * kPi * 4.0 * t);
        const double wave = 0.7 * std::sin(2.0 * kPi * 440.0 * t) + 0.3 * std::sin(2.0 * kPi * 880.0 * t);
        out[i] = static_cast<int16_t>(9000.0 * envelope * wave);
    }
}

float FrameLevel(const int16_t* samples) {
    double sum = 0.0;
    for (int i = 0; i < kFrameSamples; ++i) sum += static_cast<double>(samples[i]) * samples[i];
    return static_cast<float>(std::sqrt(sum / kFrameSamples) / 32768.0);
}

// Push-to-talk ----------------------------------------------------------------------------------------------------------

using XInputGetStateFn = DWORD(WINAPI*)(DWORD, XINPUT_STATE*);

bool PadInputHeld(const Config& cfg, int input) {
    if (input == PadLayouts::InputNone) return false;
    if (input >= PadLayouts::InputPaddle1 && input < PadLayouts::InputPaddle1 + PadLayouts::kPaddleCount) {
        const int vk = cfg.paddleKeys[static_cast<size_t>(input - PadLayouts::InputPaddle1)];
        return vk > 0 && (GetAsyncKeyState(vk) & 0x8000) != 0;
    }
    static XInputGetStateFn s_get = [] {
        HMODULE m = LoadLibraryW(L"xinput1_4.dll");
        return m ? reinterpret_cast<XInputGetStateFn>(GetProcAddress(m, "XInputGetState")) : nullptr;
    }();
    if (!s_get) return false;
    // A missing pad makes XInputGetState slow: while none is connected, look again only every 2 s.
    static uint64_t s_nextLook = 0;
    const uint64_t now = NowMs();
    if (now < s_nextLook) return false;
    constexpr int kStickOn = 16000;
    for (DWORD slot = 0; slot < 4; ++slot) {
        XINPUT_STATE st{};
        if (s_get(slot, &st) != ERROR_SUCCESS) continue;
        const XINPUT_GAMEPAD& g = st.Gamepad;
        bool held = false;
        switch (input) {
        case PadLayouts::InputLT: held = g.bLeftTrigger > 128; break;
        case PadLayouts::InputRT: held = g.bRightTrigger > 128; break;
        case PadLayouts::InputLStickUp: held = g.sThumbLY > kStickOn; break;
        case PadLayouts::InputLStickDown: held = g.sThumbLY < -kStickOn; break;
        case PadLayouts::InputLStickLeft: held = g.sThumbLX < -kStickOn; break;
        case PadLayouts::InputLStickRight: held = g.sThumbLX > kStickOn; break;
        case PadLayouts::InputRStickUp: held = g.sThumbRY > kStickOn; break;
        case PadLayouts::InputRStickDown: held = g.sThumbRY < -kStickOn; break;
        case PadLayouts::InputRStickLeft: held = g.sThumbRX < -kStickOn; break;
        case PadLayouts::InputRStickRight: held = g.sThumbRX > kStickOn; break;
        default: held = input > 0 && (g.wButtons & input) != 0; break;
        }
        return held;
    }
    s_nextLook = now + 2000;
    return false;
}

// Returns the voice channel the player is holding a key for, or -1. Separate keys win over the generic PTT; when
// several are held, the narrower channel wins (party > team > everyone).
int ChannelFromHeld(bool party, bool team, bool global, bool generic, int fallbackChannel) {
    if (party) return 2;
    if (team) return 1;
    if (global) return 0;
    if (generic) return fallbackChannel;
    return -1;
}

int PushToTalkChannel(const Config& cfg, int fallbackChannel) {
    // Only for our own window: the same key typed in another program, or in one of our text boxes, must not open the mic.
    HWND foreground = GetForegroundWindow();
    DWORD process = 0;
    if (foreground) GetWindowThreadProcessId(foreground, &process);
    if (process != GetCurrentProcessId()) return -1;
    if (s_pushToTalkBlocked.load() || (ImGui::GetCurrentContext() && ImGui::GetIO().WantTextInput)) return -1;
    auto held = [](int vk) { return vk > 0 && vk < 256 && (GetAsyncKeyState(vk) & 0x8000) != 0; };
    const bool party = held(cfg.pttPartyKey) || PadInputHeld(cfg, cfg.pttPartyPad);
    const bool team = held(cfg.pttTeamKey) || PadInputHeld(cfg, cfg.pttTeamPad);
    const bool global = held(cfg.pttGlobalKey) || PadInputHeld(cfg, cfg.pttGlobalPad);
    const bool generic = held(cfg.pttKey) || PadInputHeld(cfg, cfg.pttPad);
    return ChannelFromHeld(party, team, global, generic, fallbackChannel);
}

Config ReadConfig() {
    const SettingsData d = Settings::Get().Snapshot();
    Config c;
    c.enabled = d.voiceEnabled;
    c.input = d.voiceInputDevice;
    c.output = d.voiceOutputDevice;
    c.volume = d.voiceVolume;
    c.openMic = d.voiceOpenMic;
    c.bitrateKbps = d.voiceBitrateKbps;
    c.channel = d.voiceChannel;
    c.masterMute = d.masterMute;
    c.pttKey = KeyBindings::EffectiveKey(KeyBindings::kActionPushToTalk);
    c.pttPartyKey = KeyBindings::EffectiveKey(KeyBindings::kActionVoiceParty);
    c.pttTeamKey = KeyBindings::EffectiveKey(KeyBindings::kActionVoiceTeam);
    c.pttGlobalKey = KeyBindings::EffectiveKey(KeyBindings::kActionVoiceGlobal);
    c.pttPad = PadLayouts::EffectiveInput(d.padLayout, PadLayouts::PushToTalk, &d.padBindings);
    c.pttPartyPad = PadLayouts::EffectiveInput(d.padLayout, PadLayouts::VoiceParty, &d.padBindings);
    c.pttTeamPad = PadLayouts::EffectiveInput(d.padLayout, PadLayouts::VoiceTeam, &d.padBindings);
    c.pttGlobalPad = PadLayouts::EffectiveInput(d.padLayout, PadLayouts::VoiceGlobal, &d.padBindings);
    c.paddleKeys = d.padPaddleKeys;
    return c;
}

// Receiving ---------------------------------------------------------------------------------------------------------------

void DestroyPeer(Peer& peer) {
    if (peer.decoder) opus_decoder_destroy(peer.decoder);
    peer.decoder = nullptr;
}

void ClearPeers() {
    std::lock_guard<std::mutex> lock(s_mutex);
    for (auto& [id, peer] : s_peers) DestroyPeer(peer);
    s_peers.clear();
}

// Called on the relay thread. The bytes come from other players, so nothing is trusted but the size checks.
void OnRelayFrame(const uint8_t* data, size_t size) {
    if (size < kHeaderSize || size > kHeaderSize + kMaxPayload) return;
    uint64_t sender = 0;
    uint16_t seq = 0;
    std::memcpy(&sender, data + 4, 8);
    std::memcpy(&seq, data + 12, 2);
    const uint8_t flags = data[14];
    const uint8_t channel = data[15];
    const int team = static_cast<int8_t>(data[16]);
    s_stats.received.fetch_add(1);

    const int localTeam = s_localTeam.load();
    if (channel == 1 && team >= 0 && localTeam >= 0 && team != localTeam) {
        s_stats.filtered.fetch_add(1);
        return;
    }

    std::lock_guard<std::mutex> lock(s_mutex);
    auto it = s_peers.find(sender);
    if (it == s_peers.end()) {
        if (s_peers.size() >= kMaxPeers) return;
        int error = 0;
        OpusDecoder* decoder = opus_decoder_create(kSampleRate, 1, &error);
        if (!decoder || error != OPUS_OK) return;
        it = s_peers.emplace(sender, Peer{}).first;
        it->second.decoder = decoder;
        NOBLE_INFO("VoiceChat: first frame from peer 0x{:X}", sender);
    }
    Peer& peer = it->second;
    peer.lastHeardMs = NowMs();

    if (peer.playing && static_cast<int16_t>(seq - peer.next) < 0) {   // its turn has passed: late packets are dropped
        s_stats.late.fetch_add(1);
        return;
    }
    auto where = peer.buffer.begin();
    while (where != peer.buffer.end() && static_cast<int16_t>(seq - where->seq) > 0) ++where;
    if (where != peer.buffer.end() && where->seq == seq) return;
    BufferedFrame frame;
    frame.seq = seq;
    frame.flags = flags;
    frame.payload.assign(data + kHeaderSize, data + size);
    peer.buffer.insert(where, std::move(frame));
    if (static_cast<int>(peer.buffer.size()) > kBufferedMaxFrames) {
        peer.buffer.erase(peer.buffer.begin());
        s_stats.skipped.fetch_add(1);
        if (peer.playing) peer.next = peer.buffer.front().seq;
    }
}

// One 20 ms frame of a peer's voice, or false when it has nothing to play. Holds s_mutex.
bool PullPeer(Peer& peer, int16_t* pcm, uint64_t now) {
    if (!peer.playing) {
        if (peer.buffer.empty()) return false;
        const bool shortSpurt = std::any_of(peer.buffer.begin(), peer.buffer.end(), [](const BufferedFrame& f) { return (f.flags & kFlagEnd) != 0; });
        if (static_cast<int>(peer.buffer.size()) < peer.target && !shortSpurt) return false;
        peer.playing = true;
        peer.misses = 0;
        peer.next = peer.buffer.front().seq;
    }
    if (peer.target > kJitterMinFrames && now - peer.lastUnderrunMs > 10000) {   // a clean 10 s: back towards 60 ms
        --peer.target;
        peer.lastUnderrunMs = now;
    }
    while (static_cast<int>(peer.buffer.size()) > peer.target + 4) {   // fell behind: catch up
        peer.buffer.erase(peer.buffer.begin());
        peer.next = peer.buffer.front().seq;
        s_stats.skipped.fetch_add(1);
    }
    if (!peer.buffer.empty() && peer.buffer.front().seq == peer.next) {
        BufferedFrame frame = std::move(peer.buffer.front());
        peer.buffer.erase(peer.buffer.begin());
        ++peer.next;
        peer.misses = 0;
        if ((frame.flags & kFlagEnd) != 0 || frame.payload.empty()) {
            peer.playing = false;
            opus_decoder_ctl(peer.decoder, OPUS_RESET_STATE);
            return false;
        }
        const int got = opus_decode(peer.decoder, frame.payload.data(), static_cast<opus_int32>(frame.payload.size()), pcm, kFrameSamples, 0);
        if (got == kFrameSamples) {
            s_stats.played.fetch_add(1);
            peer.lastAudioMs = now;
            return true;
        }
    } else {
        ++peer.next;
        if (peer.buffer.empty()) {
            peer.target = (std::min)(kJitterMaxFrames, peer.target + 1);
            peer.lastUnderrunMs = now;
        }
        if (++peer.misses >= 5) {   // the talker is gone without an end marker
            peer.playing = false;
            peer.buffer.clear();
            opus_decoder_ctl(peer.decoder, OPUS_RESET_STATE);
            return false;
        }
    }
    const int got = opus_decode(peer.decoder, nullptr, 0, pcm, kFrameSamples, 0);   // Opus fills the gap
    if (got != kFrameSamples) return false;
    s_stats.concealed.fetch_add(1);
    return true;
}

void MixFrame(int16_t* out, float masterGain) {
    int32_t mix[kFrameSamples]{};
    int16_t pcm[kFrameSamples];
    const uint64_t now = NowMs();
    {
        std::lock_guard<std::mutex> lock(s_mutex);
        for (auto it = s_peers.begin(); it != s_peers.end();) {
            Peer& peer = it->second;
            const uint64_t id = it->first;
            if (PullPeer(peer, pcm, now)) {
                const auto muted = s_muted.find(id);
                if (muted == s_muted.end() || !muted->second) {
                    const auto volume = s_peerVolume.find(id);
                    const float gain = volume == s_peerVolume.end() ? 1.0f : volume->second;
                    for (int i = 0; i < kFrameSamples; ++i) mix[i] += static_cast<int32_t>(static_cast<float>(pcm[i]) * gain);
                }
            }
            if (now - peer.lastHeardMs > 30000) {
                DestroyPeer(peer);
                it = s_peers.erase(it);
            } else {
                ++it;
            }
        }
    }
    int peak = 0;
    for (int i = 0; i < kFrameSamples; ++i) {
        const int32_t clipped = std::clamp(mix[i], -32768, 32767);
        peak = (std::max)(peak, std::abs(clipped));
        out[i] = static_cast<int16_t>(clipped);
    }
    if (static_cast<uint64_t>(peak) > s_mixPeak.load()) s_mixPeak.store(static_cast<uint64_t>(peak));
    if (s_test.wavTap) s_test.wavTap->insert(s_test.wavTap->end(), out, out + kFrameSamples);
    if (masterGain != 1.0f) {
        for (int i = 0; i < kFrameSamples; ++i) {
            out[i] = static_cast<int16_t>(std::clamp(static_cast<int32_t>(static_cast<float>(out[i]) * masterGain), -32768, 32767));
        }
    }
}

// The voice thread ------------------------------------------------------------------------------------------------------

void VoiceThread() {
    CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    Stream captureStream, renderStream;
    Ring ring;
    OpusEncoder* encoder = nullptr;
    int encoderBitrate = 0;
    uint16_t seq = 0;
    int64_t tonePosition = 0;
    bool wasTransmitting = false;
    int hang = 0;
    bool sessionOpen = false;
    uint64_t lastConnectedMs = 0, nextConfigMs = 0, nextDevicesMs = 0, nextStatsMs = 0;
    uint64_t toneClockStart = 0, toneFrames = 0, mixClockStart = 0, mixFrames = 0;
    Config cfg, opened;
    if (s_test.active) cfg.enabled = true;   // tests never read the player's settings
    std::vector<uint8_t> packet(kHeaderSize + kMaxPayload);
    uint64_t lastSent = 0, lastRecv = 0, lastPlayed = 0, lastCaptured = 0, lastRendered = 0, lastBytes = 0;
    int lastChannel = 0;

    const auto sendFrame = [&](const uint8_t* payload, int size, uint8_t flags, uint8_t channel) {
        std::memcpy(packet.data(), "NVOX", 4);
        const uint64_t id = NetworkTransport::Get().GetRelayMachineId();
        std::memcpy(packet.data() + 4, &id, 8);
        std::memcpy(packet.data() + 12, &seq, 2);
        packet[14] = flags;
        packet[15] = channel;
        packet[16] = static_cast<uint8_t>(static_cast<int8_t>(s_localTeam.load()));
        packet[17] = 0;
        if (size > 0) std::memcpy(packet.data() + kHeaderSize, payload, static_cast<size_t>(size));
        ++seq;
        NetworkTransport::Get().SendVoiceFrame(packet.data(), kHeaderSize + static_cast<size_t>(size));
        s_stats.sent.fetch_add(1);
        s_stats.sentBytes.fetch_add(kHeaderSize + static_cast<size_t>(size));
    };

    const auto processFrame = [&](const int16_t* pcm) {
        int channel = cfg.channel;
        bool transmit = false;
        if (s_test.active) {
            transmit = s_test.forceTransmit;
        } else {
            channel = PushToTalkChannel(cfg, cfg.channel);
            transmit = channel >= 0;
            if (!transmit && cfg.openMic) {
                if (FrameLevel(pcm) >= kOpenMicThreshold) hang = kOpenMicHangFrames;
                else if (hang > 0) --hang;
                transmit = hang > 0;
                channel = cfg.channel;
            }
        }
        if (transmit) lastChannel = channel;
        if (transmit && encoder) {
            uint8_t payload[kMaxPayload];
            const int bytes = opus_encode(encoder, pcm, kFrameSamples, payload, kMaxPayload);
            if (bytes > 0) sendFrame(payload, bytes, 0, static_cast<uint8_t>(channel));
        } else if (wasTransmitting) {
            sendFrame(nullptr, 0, kFlagEnd, static_cast<uint8_t>(lastChannel));
            opus_encoder_ctl(encoder, OPUS_RESET_STATE);
        }
        wasTransmitting = transmit;
        s_transmitting.store(transmit);
    };

    while (!s_stop.load()) {
        Sleep(5);
        const uint64_t now = NowMs();

        if (now >= nextConfigMs) {
            nextConfigMs = now + 500;
            if (!s_test.active) cfg = ReadConfig();
            else cfg.bitrateKbps = s_testBitrate.load();
        }
        if (s_wantDevices.load() || now >= nextDevicesMs) {
            nextDevicesMs = now + 10000;
            s_wantDevices.store(false);
            auto inputs = ListDevices(eCapture);
            auto outputs = ListDevices(eRender);
            std::lock_guard<std::mutex> lock(s_mutex);
            s_inputs = std::move(inputs);
            s_outputs = std::move(outputs);
        }

        // A relay reconnect (5 s grace) keeps the streams open, so a blip doesn't cut anyone off.
        if (NetworkTransport::Get().IsRelayConnected()) lastConnectedMs = now;
        const bool wanted = cfg.enabled && lastConnectedMs != 0 && now - lastConnectedMs < 5000;
        if (wanted != sessionOpen) {
            sessionOpen = wanted;
            NOBLE_INFO("VoiceChat: session {}", wanted ? "open" : "closed");
            if (!wanted) {
                captureStream.Close();
                renderStream.Close();
                ring.Clear();
                ClearPeers();
                s_transmitting.store(false);
                wasTransmitting = false;
            }
            captureStream.retryAtMs = renderStream.retryAtMs = 0;
            toneClockStart = mixClockStart = now;
            toneFrames = mixFrames = 0;
        }
        if (!sessionOpen) continue;

        if (cfg.input != opened.input && captureStream.Open()) captureStream.Close();
        if (cfg.output != opened.output && renderStream.Open()) renderStream.Close();
        if (!captureStream.Open() && now >= captureStream.retryAtMs) {
            const HRESULT hr = captureStream.OpenOn(eCapture, cfg.input);
            if (SUCCEEDED(hr)) {
                opened.input = cfg.input;
                NOBLE_INFO("VoiceChat: capture open on '{}' (48 kHz mono, {} frame buffer)", captureStream.name, captureStream.bufferFrames);
            } else {
                captureStream.retryAtMs = now + 5000;
                NOBLE_WARN("VoiceChat: no microphone stream (0x{:08X}); listening still works", static_cast<uint32_t>(hr));
            }
        }
        if (!renderStream.Open() && now >= renderStream.retryAtMs) {
            const HRESULT hr = renderStream.OpenOn(eRender, cfg.output);
            if (SUCCEEDED(hr)) {
                opened.output = cfg.output;
                NOBLE_INFO("VoiceChat: playback open on '{}' (48 kHz mono, {} frame buffer)", renderStream.name, renderStream.bufferFrames);
            } else {
                renderStream.retryAtMs = now + 5000;
                NOBLE_WARN("VoiceChat: no playback stream (0x{:08X})", static_cast<uint32_t>(hr));
            }
        }
        if (encoderBitrate != cfg.bitrateKbps) {
            if (!encoder) {
                int error = 0;
                encoder = opus_encoder_create(kSampleRate, 1, OPUS_APPLICATION_VOIP, &error);
                if (error != OPUS_OK) encoder = nullptr;
            }
            if (encoder) {
                opus_encoder_ctl(encoder, OPUS_SET_BITRATE(cfg.bitrateKbps * 1000));
                opus_encoder_ctl(encoder, OPUS_SET_COMPLEXITY(5));
                encoderBitrate = cfg.bitrateKbps;
                NOBLE_INFO("VoiceChat: Opus encoder at {} kbps", cfg.bitrateKbps);
            }
        }

        // Capture: drain the device into the ring, then encode whole frames.
        if (captureStream.Open()) {
            UINT32 packetFrames = 0;
            HRESULT hr = captureStream.capture->GetNextPacketSize(&packetFrames);
            while (SUCCEEDED(hr) && packetFrames > 0) {
                BYTE* data = nullptr;
                UINT32 frames = 0;
                DWORD flags = 0;
                hr = captureStream.capture->GetBuffer(&data, &frames, &flags, nullptr, nullptr);
                if (FAILED(hr)) break;
                ring.Push((flags & AUDCLNT_BUFFERFLAGS_SILENT) != 0 ? nullptr : reinterpret_cast<const int16_t*>(data), frames);
                captureStream.capture->ReleaseBuffer(frames);
                hr = captureStream.capture->GetNextPacketSize(&packetFrames);
            }
            if (FAILED(hr)) {   // unplugged or the format changed: try again shortly
                NOBLE_WARN("VoiceChat: microphone stream lost (0x{:08X})", static_cast<uint32_t>(hr));
                captureStream.Close();
                captureStream.retryAtMs = now + 2000;
            }
        }
        int16_t frame[kFrameSamples];
        while (ring.PopFrame(frame)) {
            s_stats.captured.fetch_add(1);
            if (!(s_test.active && s_test.tone)) processFrame(frame);
        }
        if (s_test.active && s_test.tone) {   // the synthetic source keeps its own clock so it doesn't depend on a microphone
            while ((now - toneClockStart) / 20 > toneFrames) {
                ++toneFrames;
                FillTone(frame, tonePosition);
                processFrame(frame);
            }
        }

        // Playback: the device's clock drives the mixer, which keeps it about 60 ms ahead.
        const float gain = (cfg.masterMute || (s_test.active && s_test.silentOutput)) ? 0.0f : cfg.volume;
        if (renderStream.Open()) {
            UINT32 padding = 0;
            HRESULT hr = renderStream.client->GetCurrentPadding(&padding);
            if (SUCCEEDED(hr) && padding == 0 && s_stats.rendered.load() > 0) s_stats.underruns.fetch_add(1);
            while (SUCCEEDED(hr) && padding < kFrameSamples * 3 && padding + kFrameSamples <= renderStream.bufferFrames) {
                BYTE* data = nullptr;
                hr = renderStream.render->GetBuffer(kFrameSamples, &data);
                if (FAILED(hr)) break;
                MixFrame(reinterpret_cast<int16_t*>(data), gain);
                hr = renderStream.render->ReleaseBuffer(kFrameSamples, 0);
                s_stats.rendered.fetch_add(1);
                padding += kFrameSamples;
            }
            if (FAILED(hr)) {
                NOBLE_WARN("VoiceChat: playback stream lost (0x{:08X})", static_cast<uint32_t>(hr));
                renderStream.Close();
                renderStream.retryAtMs = now + 2000;
            }
            mixClockStart = now;
            mixFrames = 0;
        } else {   // no speaker: keep the jitter buffers moving so nothing piles up
            while ((now - mixClockStart) / 20 > mixFrames) {
                ++mixFrames;
                MixFrame(frame, 0.0f);
            }
        }

        // One line a second in tests. In a real session one every 10 s, and only while voice is flowing.
        const uint64_t statsEvery = s_test.active ? 1000 : 10000;
        if (now >= nextStatsMs) {
            const uint64_t sent = s_stats.sent.load(), recv = s_stats.received.load(), played = s_stats.played.load();
            const uint64_t captured = s_stats.captured.load(), rendered = s_stats.rendered.load();
            if (nextStatsMs != 0 && (s_test.active || sent != lastSent || recv != lastRecv)) {
                const double secs = static_cast<double>(statsEvery) / 1000.0;
                NOBLE_INFO("VoiceChat: frames/s sent={:.0f} bytes/frame={} recv={:.0f} played={:.0f} captured={:.0f} rendered={:.0f} | totals late={} concealed={} skipped={} filtered={} underruns={} peak={}",
                           static_cast<double>(sent - lastSent) / secs, sent != lastSent ? (s_stats.sentBytes.load() - lastBytes) / (sent - lastSent) : 0, static_cast<double>(recv - lastRecv) / secs, static_cast<double>(played - lastPlayed) / secs,
                           static_cast<double>(captured - lastCaptured) / secs, static_cast<double>(rendered - lastRendered) / secs,
                           s_stats.late.load(), s_stats.concealed.load(), s_stats.skipped.load(), s_stats.filtered.load(), s_stats.underruns.load(), s_mixPeak.exchange(0));
            }
            lastBytes = s_stats.sentBytes.load(); lastSent = sent; lastRecv = recv; lastPlayed = played; lastCaptured = captured; lastRendered = rendered;
            nextStatsMs = now + statsEvery;
        }
    }

    captureStream.Close();
    renderStream.Close();
    ClearPeers();
    if (encoder) opus_encoder_destroy(encoder);
    CoUninitialize();
}

// Test modes --------------------------------------------------------------------------------------------------------------

// Best signal-to-noise over the codec's delay, between what went in and what came out.
double RoundTripSnrDb(const std::vector<int16_t>& in, const std::vector<int16_t>& out) {
    double best = -100.0;
    const size_t skip = kSampleRate / 2, count = kSampleRate * 2;
    if (in.size() < skip + count + 2000 || out.size() < skip + count + 2000) return best;
    for (size_t delay = 0; delay < 1000; ++delay) {
        double signal = 0.0, noise = 0.0;
        for (size_t i = 0; i < count; ++i) {
            const double a = in[skip + i], b = out[skip + i + delay];
            signal += a * a;
            noise += (a - b) * (a - b);
        }
        if (noise > 0.0 && signal > 0.0) best = (std::max)(best, 10.0 * std::log10(signal / noise));
    }
    return best;
}

// Network conditions a local relay never produces, driven straight through the receive and mix code.
int RunJitterSelfTest() {
    int failures = 0;
    const auto check = [&failures](const char* what, bool ok) {
        NOBLE_INFO("VoiceChat jitter test: {}: {}", what, ok ? "ok" : "FAILED");
        if (!ok) ++failures;
    };
    // Channel priority for the separate PTT binds: party > team > everyone, then the TALK TO fallback, then silence.
    check("voice channel: party wins when several are held", ChannelFromHeld(true, true, true, true, 0) == 2);
    check("voice channel: team beats everyone", ChannelFromHeld(false, true, true, true, 0) == 1);
    check("voice channel: everyone beats generic PTT", ChannelFromHeld(false, false, true, true, 1) == 0);
    check("voice channel: generic PTT uses TALK TO", ChannelFromHeld(false, false, false, true, 1) == 1);
    check("voice channel: nothing held is silent", ChannelFromHeld(false, false, false, false, 1) == -1);
    // The Settings list is generated from these tables; a missing name would hide a row.
    auto named = [](const char* s) { return s != nullptr && s[0] != '\0'; };
    check("voice bind: keyboard party/team/global rows exist",
          named(KeyBindings::ActionName(KeyBindings::kActionVoiceParty)) &&
          named(KeyBindings::ActionName(KeyBindings::kActionVoiceTeam)) &&
          named(KeyBindings::ActionName(KeyBindings::kActionVoiceGlobal)));
    check("voice bind: controller party/team/global rows exist",
          named(PadLayouts::ActionName(PadLayouts::VoiceParty)) &&
          named(PadLayouts::ActionName(PadLayouts::VoiceTeam)) &&
          named(PadLayouts::ActionName(PadLayouts::VoiceGlobal)));
    check("voice bind: keyboard defaults avoid the F1-F6 paddle keys",
          KeyBindings::DefaultKey(KeyBindings::kActionVoiceParty) == VK_F9 &&
          KeyBindings::DefaultKey(KeyBindings::kActionVoiceTeam) == VK_F10 &&
          KeyBindings::DefaultKey(KeyBindings::kActionVoiceGlobal) == VK_F11);
    int error = 0;
    OpusEncoder* encoder = opus_encoder_create(kSampleRate, 1, OPUS_APPLICATION_VOIP, &error);
    if (!encoder) return 1;
    int64_t position = 0;
    constexpr uint64_t kPeer = 0x77;
    const auto feed = [&](uint16_t seq, uint8_t flags = 0, uint8_t channel = 0, int8_t team = -1) {
        uint8_t packet[kHeaderSize + kMaxPayload]{};
        std::memcpy(packet, "NVOX", 4);
        std::memcpy(packet + 4, &kPeer, 8);
        std::memcpy(packet + 12, &seq, 2);
        packet[14] = flags;
        packet[15] = channel;
        packet[16] = static_cast<uint8_t>(team);
        int bytes = 0;
        if ((flags & kFlagEnd) == 0) {
            int16_t pcm[kFrameSamples];
            FillTone(pcm, position);
            bytes = opus_encode(encoder, pcm, kFrameSamples, packet + kHeaderSize, kMaxPayload);
        }
        OnRelayFrame(packet, kHeaderSize + static_cast<size_t>((std::max)(bytes, 0)));
    };
    int16_t out[kFrameSamples];
    const auto tick = [&out] { MixFrame(out, 1.0f); };
    const auto target = [kPeer] { std::lock_guard<std::mutex> lock(s_mutex); return s_peers.count(kPeer) ? s_peers[kPeer].target : 0; };
    const auto playing = [kPeer] { std::lock_guard<std::mutex> lock(s_mutex); return s_peers.count(kPeer) && s_peers[kPeer].playing; };

    // Waits for 60 ms of audio, and crosses the sequence number's wrap on the way.
    uint16_t seq = 65530;
    feed(seq++); feed(seq++);
    tick();
    check("two frames aren't enough to start", s_stats.played.load() == 0 && !playing());
    feed(seq++);
    tick();
    check("three frames (60 ms) start playback", s_stats.played.load() == 1 && playing());
    for (int i = 0; i < 20; ++i) { feed(seq++); tick(); }
    check("steady stream across the wrap has no gaps", s_stats.played.load() == 21 && s_stats.concealed.load() == 0 && seq < 100);

    // Out of order inside the buffer: both still play.
    const uint64_t playedBefore = s_stats.played.load();
    const uint16_t first = seq++, second = seq++;
    feed(second); feed(first);
    tick(); tick();
    check("reordered frames play in order", s_stats.played.load() == playedBefore + 2 && s_stats.concealed.load() == 0);

    // Behind the playhead: dropped, not played.
    feed(static_cast<uint16_t>(seq - 30));
    check("a late frame is dropped", s_stats.late.load() == 1);

    // A dropout: Opus conceals it and the buffer grows, but never past 120 ms.
    while (playing()) {
        std::lock_guard<std::mutex> lock(s_mutex);
        if (s_peers[kPeer].buffer.empty()) break;
        s_peers[kPeer].buffer.clear();
    }
    const uint64_t concealedBefore = s_stats.concealed.load();
    tick();
    check("a gap is concealed and the buffer grows to 80 ms", s_stats.concealed.load() == concealedBefore + 1 && target() == 4);
    for (int i = 0; i < 8; ++i) tick();
    check("silence ends playback after five misses", !playing());
    check("the buffer has not grown past 120 ms", target() <= kJitterMaxFrames);
    { std::lock_guard<std::mutex> lock(s_mutex); s_peers[kPeer].target = kJitterMaxFrames; s_peers[kPeer].buffer.clear(); }
    for (int i = 0; i < 9; ++i) tick();
    check("120 ms is the ceiling", target() == kJitterMaxFrames);

    // The end marker stops playback at once, even for a spurt shorter than the buffer.
    const uint64_t concealedEnd = s_stats.concealed.load();
    seq = 500;
    feed(seq++); feed(seq++, kFlagEnd);
    tick();
    check("a short spurt plays without waiting", playing());
    tick();
    check("the end marker stops it cleanly", !playing() && s_stats.concealed.load() == concealedEnd);

    // Team channel, and what a hostile client could send.
    s_localTeam.store(1);
    const uint64_t filteredBefore = s_stats.filtered.load();
    feed(seq++, 0, 1, 2);
    check("another team's team-channel frame is filtered", s_stats.filtered.load() == filteredBefore + 1);
    feed(seq++, 0, 1, 1);
    check("your own team's frame is not", s_stats.filtered.load() == filteredBefore + 1);
    s_localTeam.store(-1);
    const uint64_t receivedBefore = s_stats.received.load();
    std::vector<uint8_t> junk(kHeaderSize + kMaxPayload + 1, 0x41);
    OnRelayFrame(junk.data(), 5);
    OnRelayFrame(junk.data(), junk.size());
    check("short and oversized frames are ignored", s_stats.received.load() == receivedBefore);
    uint8_t garbage[64];
    std::memset(garbage, 0xFF, sizeof(garbage));
    std::memcpy(garbage + 4, &kPeer, 8);
    garbage[12] = 0x10; garbage[13] = 0x27; garbage[14] = 0; garbage[15] = 0; garbage[16] = 0xFF;
    { std::lock_guard<std::mutex> lock(s_mutex); s_peers[kPeer].playing = false; s_peers[kPeer].buffer.clear(); s_peers[kPeer].target = kJitterMinFrames; }
    for (uint16_t i = 0; i < 4; ++i) { std::memcpy(garbage + 12, &i, 2); OnRelayFrame(garbage, sizeof(garbage)); }
    for (int i = 0; i < 6; ++i) tick();
    check("garbage where Opus should be doesn't crash", true);

    // Open mic's -40 dB gate.
    int16_t pcm[kFrameSamples]{};
    const bool silentClosed = FrameLevel(pcm) < kOpenMicThreshold;
    FillTone(pcm, position);
    check("the open mic gate is closed on silence and open on speech", silentClosed && FrameLevel(pcm) >= kOpenMicThreshold);

    opus_encoder_destroy(encoder);
    ClearPeers();
    NOBLE_INFO("VoiceChat jitter test: {}", failures == 0 ? "PASS" : "FAIL");
    return failures;
}
int RunLoopback(const std::string& wavPath, bool tone) {
    CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    constexpr int kFrames = 250;   // 5 s
    std::vector<int16_t> source;
    if (tone) {
        source.resize(static_cast<size_t>(kFrames) * kFrameSamples);
        int64_t position = 0;
        for (int f = 0; f < kFrames; ++f) FillTone(source.data() + static_cast<size_t>(f) * kFrameSamples, position);
    } else {
        Stream mic;
        const HRESULT hr = mic.OpenOn(eCapture, "");
        if (FAILED(hr)) { NOBLE_ERROR("VoiceChat loopback: microphone failed (0x{:08X})", static_cast<uint32_t>(hr)); return 1; }
        NOBLE_INFO("VoiceChat loopback: capturing 5 s from '{}'", mic.name);
        while (source.size() < static_cast<size_t>(kFrames) * kFrameSamples) {
            Sleep(10);
            UINT32 frames = 0;
            mic.capture->GetNextPacketSize(&frames);
            while (frames > 0) {
                BYTE* data = nullptr;
                DWORD flags = 0;
                if (FAILED(mic.capture->GetBuffer(&data, &frames, &flags, nullptr, nullptr))) break;
                const int16_t* s = reinterpret_cast<const int16_t*>(data);
                for (UINT32 i = 0; i < frames; ++i) source.push_back((flags & AUDCLNT_BUFFERFLAGS_SILENT) != 0 ? int16_t(0) : s[i]);
                mic.capture->ReleaseBuffer(frames);
                mic.capture->GetNextPacketSize(&frames);
            }
        }
        source.resize(static_cast<size_t>(kFrames) * kFrameSamples);
    }

    int failures = 0;
    for (const int kbps : { 16, 20, 24 }) {
        int error = 0;
        OpusEncoder* encoder = opus_encoder_create(kSampleRate, 1, OPUS_APPLICATION_VOIP, &error);
        OpusDecoder* decoder = opus_decoder_create(kSampleRate, 1, &error);
        if (!encoder || !decoder) { ++failures; continue; }
        opus_encoder_ctl(encoder, OPUS_SET_BITRATE(kbps * 1000));
        opus_encoder_ctl(encoder, OPUS_SET_COMPLEXITY(5));
        std::vector<int16_t> decoded(source.size());
        size_t totalBytes = 0;
        uint8_t payload[kMaxPayload];
        for (int f = 0; f < kFrames; ++f) {
            const size_t at = static_cast<size_t>(f) * kFrameSamples;
            const int bytes = opus_encode(encoder, source.data() + at, kFrameSamples, payload, kMaxPayload);
            if (bytes <= 0 || opus_decode(decoder, payload, bytes, decoded.data() + at, kFrameSamples, 0) != kFrameSamples) { ++failures; break; }
            totalBytes += static_cast<size_t>(bytes);
        }
        opus_encoder_destroy(encoder);
        opus_decoder_destroy(decoder);
        const double snr = tone ? RoundTripSnrDb(source, decoded) : 0.0;
        NOBLE_INFO("VoiceChat loopback: {} kbps, {} frames, {} bytes a frame, in level {:.4f}, out level {:.4f}, SNR {:.1f} dB",
                   kbps, kFrames, totalBytes / kFrames, FrameLevel(source.data() + kSampleRate), FrameLevel(decoded.data() + kSampleRate), snr);
        if (tone && snr < 3.0) ++failures;
        if (kbps == 20 && !wavPath.empty()) {
            WriteWav(wavPath, decoded);
            NOBLE_INFO("VoiceChat loopback: wrote {}", wavPath);
        }
    }
    CoUninitialize();
    NOBLE_INFO("VoiceChat loopback: {}", failures == 0 ? "PASS" : "FAIL");
    return failures == 0 ? 0 : 1;
}

int RunRelayTest(const std::string& wavPath) {
    std::vector<int16_t> tap;
    if (!wavPath.empty()) s_test.wavTap = &tap;
    NetworkTransport::Get().StartRelay(s_test.relayUrl, "voice-test", s_test.isHost, s_test.machineId);
    VoiceChat::Start();

    // A third of the way in a peer drops to half volume, two thirds in it's muted, so the peak column shows the controls work.
    const int total = s_test.seconds;
    for (int second = 0; second < total; ++second) {
        Sleep(1000);
        std::vector<uint64_t> ids;
        {
            std::lock_guard<std::mutex> lock(s_mutex);
            for (const auto& [id, peer] : s_peers) ids.push_back(id);
        }
        for (const uint64_t id : ids) {
            if (second == total / 3) s_testBitrate.store(20);
            if (second == total * 2 / 3) s_testBitrate.store(24);
            if (second == total / 3) { VoiceChat::SetPeerVolume(id, 0.5f); NOBLE_INFO("VoiceChat test: peer 0x{:X} to half volume", id); }
            if (second == total * 2 / 3) { VoiceChat::SetPeerMuted(id, true); NOBLE_INFO("VoiceChat test: peer 0x{:X} muted", id); }
        }
    }
    VoiceChat::Shutdown();
    s_test.wavTap = nullptr;
    NetworkTransport::Get().StopRelay();
    if (!wavPath.empty()) {
        WriteWav(wavPath, tap);
        NOBLE_INFO("VoiceChat test: wrote {} ({} samples)", wavPath, tap.size());
    }
    const uint64_t sent = s_stats.sent.load(), received = s_stats.received.load(), played = s_stats.played.load();
    const bool pass = sent > 0 && received > 0 && played > 0;
    NOBLE_INFO("VoiceChat test: total sent={} ({} bytes) received={} played={} captured={} rendered={} late={} concealed={} skipped={} underruns={}: {}",
               sent, s_stats.sentBytes.load(), received, played, s_stats.captured.load(), s_stats.rendered.load(), s_stats.late.load(),
               s_stats.concealed.load(), s_stats.skipped.load(), s_stats.underruns.load(), pass ? "PASS" : "FAIL");
    return pass ? 0 : 1;
}

} // namespace

namespace VoiceChat {

void Start() {
    if (s_started.exchange(true)) return;
    s_stop.store(false);
    NetworkTransport::Get().SetVoiceFrameCallback(&OnRelayFrame);
    s_thread = std::thread(&VoiceThread);
}

void Shutdown() {
    if (!s_started.exchange(false)) return;
    s_stop.store(true);
    if (s_thread.joinable()) s_thread.join();
    NetworkTransport::Get().SetVoiceFrameCallback(nullptr);
}

void SetPeerMuted(uint64_t peer, bool muted) {
    std::lock_guard<std::mutex> lock(s_mutex);
    s_muted[peer] = muted;
}

bool IsPeerMuted(uint64_t peer) {
    std::lock_guard<std::mutex> lock(s_mutex);
    const auto it = s_muted.find(peer);
    return it != s_muted.end() && it->second;
}

void SetPeerVolume(uint64_t peer, float volume) {
    std::lock_guard<std::mutex> lock(s_mutex);
    s_peerVolume[peer] = std::clamp(volume, 0.0f, 2.0f);
}

float PeerVolume(uint64_t peer) {
    std::lock_guard<std::mutex> lock(s_mutex);
    const auto it = s_peerVolume.find(peer);
    return it == s_peerVolume.end() ? 1.0f : it->second;
}

bool IsPeerTalking(uint64_t peer) {
    std::lock_guard<std::mutex> lock(s_mutex);
    const auto it = s_peers.find(peer);
    return it != s_peers.end() && NowMs() - it->second.lastAudioMs < 250;
}

bool IsTransmitting() { return s_transmitting.load(); }

std::vector<uint64_t> ActiveTalkers() {
    std::vector<uint64_t> out;
    const uint64_t now = NowMs();
    std::lock_guard<std::mutex> lock(s_mutex);
    for (const auto& [id, peer] : s_peers) {
        const auto muted = s_muted.find(id);
        if (now - peer.lastAudioMs < 250 && (muted == s_muted.end() || !muted->second)) out.push_back(id);
    }
    return out;
}

void SetPushToTalkBlocked(bool blocked) { s_pushToTalkBlocked.store(blocked); }

void SetLocalTeam(int team) { s_localTeam.store(team); }

std::vector<Device> Devices(bool capture) {
    std::lock_guard<std::mutex> lock(s_mutex);
    return capture ? s_inputs : s_outputs;
}

} // namespace VoiceChat

void ProbeVoiceDevices() {
    const HRESULT init = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    const auto report = [](EDataFlow flow, const char* label) {
        const auto devices = ListDevices(flow);
        NOBLE_INFO("VoiceChat: {} {} device(s)", devices.size(), label);
        for (const auto& d : devices) NOBLE_INFO("VoiceChat:   {}", d.name);
        ComPtr<IMMDeviceEnumerator> e = Enumerator();
        ComPtr<IMMDevice> device;
        if (e && SUCCEEDED(e->GetDefaultAudioEndpoint(flow, eConsole, &device))) NOBLE_INFO("VoiceChat: default {}: {}", label, FriendlyName(device.Get()));
    };
    report(eCapture, "capture");
    report(eRender, "playback");
    if (SUCCEEDED(init)) CoUninitialize();
}

bool RunVoiceTestIfRequested(int& exitCode) {
    int argc = 0;
    LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);
    std::string mode, wav, source = "tone";
    for (int i = 1; argv && i + 1 < argc; ++i) {
        const std::wstring arg(argv[i]);
        const std::string value = Narrow(argv[i + 1]);
        if (arg == L"-voice-test") mode = value;
        else if (arg == L"-voice-test-wav") wav = value;
        else if (arg == L"-voice-test-source") source = value;
        else if (arg == L"-voice-test-relay") s_test.relayUrl = value;
        else if (arg == L"-voice-test-seconds") s_test.seconds = std::clamp(std::atoi(value.c_str()), 3, 600);
    }
    if (argv) LocalFree(argv);
    if (mode.empty()) return false;

    if (mode == "loopback") {
        exitCode = RunLoopback(wav, source != "mic");
        if (RunJitterSelfTest() != 0) exitCode = 1;
        return true;
    }
    if ((mode != "host" && mode != "guest") || s_test.relayUrl.empty()) {
        NOBLE_ERROR("VoiceChat test: use -voice-test loopback, or -voice-test host|guest with -voice-test-relay");
        exitCode = 2;
        return true;
    }
    s_test.active = true;
    s_test.tone = source != "mic";
    s_test.isHost = mode == "host";
    s_test.machineId = s_test.isHost ? NetworkTransport::kDefaultHostMachineId : (static_cast<uint64_t>(GetCurrentProcessId()) << 8) | 0x1001ULL;
    exitCode = RunRelayTest(wav);
    return true;
}

} // namespace noble
