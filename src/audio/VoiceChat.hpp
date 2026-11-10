#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace noble {

// Voice chat: WASAPI capture and playback on their own thread, Opus at 48 kHz mono in 20 ms frames, carried by the session's
// relay WebSocket. It never goes through Reach's XAudio, so it keeps running through map loads and menus.
//
// The microphone and speaker streams are only open while the player is in a relay session with voice enabled. Nothing is
// sent unless push-to-talk is held, or open mic is on and the input is above -40 dBFS. Settings come from
// SettingsData (voice*), polled twice a second, so applying them in the Settings screen is enough.
//
// Endpoints and keys belong to NetworkTransport. Nothing here takes an address from the UI.
namespace VoiceChat {

struct Device {
    std::string id;     // Windows endpoint id, stored in settings; empty means the system default
    std::string name;
};

void Start();           // Starts the voice thread once. Idle (no streams, no mic) until a relay session exists.
void Shutdown();

// Per-player controls, keyed by the sender id the relay stamps (the player's machine id). They last for this run of the game.
void SetPeerMuted(uint64_t peer, bool muted);
bool IsPeerMuted(uint64_t peer);
void SetPeerVolume(uint64_t peer, float volume);   // 0..2
float PeerVolume(uint64_t peer);
bool IsPeerTalking(uint64_t peer);
bool IsTransmitting();
int NoSessionHint();                            // 1 or 2 for a few seconds after a talk key is held with no voice session (1: no session, 2: voice off in Settings); else 0
std::vector<uint64_t> ActiveTalkers();          // talking now and not muted by you
void SetPushToTalkBlocked(bool blocked);        // while the player types in chat, so typing the key doesn't open the mic

// Team channel: the local player's team, -1 while unknown. Filtering by team happens on the receiving client.
void SetLocalTeam(int team);

// The system's active devices, refreshed by the voice thread. Empty until the first refresh.
std::vector<Device> Devices(bool capture);

} // namespace VoiceChat

// Logs the devices found (`-voice-probe`).
void ProbeVoiceDevices();

// Headless test modes that run without MCC. Returns true when the command line asked for one, with its exit code.
//   -voice-test loopback [-voice-test-wav file] [-voice-test-source tone|mic]
//   -voice-test host|guest -voice-test-relay <url> [-voice-test-seconds n] [-voice-test-wav file] [-voice-test-source tone|mic]
bool RunVoiceTestIfRequested(int& exitCode);

} // namespace noble
