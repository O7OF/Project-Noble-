#pragma once

#include "common/Settings.hpp"

namespace noble {

// SETTINGS screen (front-end menu), laid out like MCC's: CONTROLS, KEY BINDINGS, VIDEO, AUDIO,
// GAMEPLAY and NETWORK tabs. Based on the
// partner's settings_manager.gd / build_settings_menu.gd. Changes are edited on a copy and saved
// with APPLY. Mouse, volume and key settings reach the engine through the profile (host slot 34)
// and Noble's input code; display size and player name take effect the next time a game starts.
class SettingsPanel {
public:
    void Render();
    bool IsCapturingKey() const { return m_rebindAction >= 0; }  // Esc cancels the rebind, not the screen
    void SetTab(int tab) { m_tab = tab % 6; }                     // test option -menu-tab

private:
    void RenderControls();
    void RenderKeyBindings();
    void RenderAudio();
    void RenderDisplay();
    void RenderGameplay();
    void RenderNetwork();
    void PollRebind();

    int m_tab = 0;

    bool m_loaded = false;
    SettingsData m_edit;
    bool m_dirty = false;
    int m_rebindAction = -1;      // action waiting for a key press
    double m_rebindStarted = 0.0;
    char m_nameBuf[32] = {};
    char m_tagBuf[8] = {};
    char m_masterBuf[256] = {};
    const char* m_message = nullptr;
};

} // namespace noble
