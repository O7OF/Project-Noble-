#pragma once

namespace noble {

// Voice chat, phase 1: enumerate the default capture/playback devices so the transport has real device handles to open.
// Called once at startup when -voice-probe is passed (and later by the settings page).
void ProbeVoiceDevices();

} // namespace noble
