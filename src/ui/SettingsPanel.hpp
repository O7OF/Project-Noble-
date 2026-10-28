#pragma once

#include "common/Settings.hpp"

#include <imgui.h>
#include <string>

namespace noble {

// SETTINGS screen (front-end menu and pause menu), three columns as in docs/menu-redesign-spec.md
// §4.2: categories on the left (grouped, each with a live summary and an amber dot when it has
// unapplied changes, search box on top, unsaved-changes box or RESTORE DEFAULTS at the bottom),
// the category's rows in the middle, and a details panel on the right (what the focused or hovered
// row does, when it takes effect, its default, and a preview where one helps).
// Changes are edited on a copy and saved with APPLY CHANGES (DISCARD goes back). Settings keep the
// same JSON keys as before, so old settings.json files load unchanged.
// Keys / pad: Up/Down move between rows, Left/Right change a value, Enter activates (rebind, toggle,
// text), Q/E (LB/RB) switch category, Y applies and X discards.
class SettingsPanel {
public:
    void Render();
    bool IsCapturingKey() const { return m_rebindAction >= 0; }  // Esc cancels the rebind, not the screen
    void SetTab(int tab);                                          // test option -menu-tab (0..6 = category)

private:
    enum class Cat { Graphics, Audio, Game, Player, KeyboardMouse, Controller, Online, Count };
    enum class When { Apply, Restart, Instant, Info };
    enum class Preview { None, Fov, FrameLimit, Volume, Controller, Gpu };
    struct Details {
        bool set = false;
        std::string title, effect, text, facts;
        Preview preview = Preview::None;
    };

    // Columns.
    void RenderCategories(float width, float height);
    void RenderRows(float width, float height);
    void RenderDetails(float width, float height);
    void RenderCategory(Cat c);
    void RenderGraphics();
    void RenderAudio();
    void RenderGame();
    void RenderPlayer();
    void RenderKeyboardMouse();
    void RenderController();
    void RenderOnline();
    void RenderMusicPicker();
    void PollRebind();

    // Row kit. Row() lays out one row (returns false when the search hides it); the value widget
    // that follows draws into the row's value column and reads the shared key state.
    bool Row(const char* label, const char* description, When when, const std::string& defaultText,
             bool changed, bool disabled = false, Preview preview = Preview::None);
    void Section(const char* text);
    bool SliderValue(float& v, float lo, float hi, float step, float def, const char* text);
    int CycleValue(const char* value, int index, int count, bool wraps = true);
    bool ToggleValue(bool& v);
    void InfoValue(const char* text);

    // Changes.
    int ChangesIn(Cat c) const;
    static int CountDiff(const SettingsData& a, const SettingsData& b, Cat c);
    static SettingsData WithDefaults(SettingsData e, Cat c);
    int TotalChanges() const;
    void RestoreDefaults(Cat c);
    void Apply();
    void Discard();
    std::string Summary(Cat c) const;
    void SyncBuffers();

    Cat m_cat = Cat::Graphics;
    bool m_loaded = false;
    SettingsData m_edit;
    SettingsData m_saved;
    int m_rebindAction = -1;      // action waiting for a key press
    double m_rebindStarted = 0.0;
    char m_nameBuf[32] = {};
    char m_tagBuf[8] = {};
    char m_masterBuf[256] = {};
    char m_search[64] = {};
    std::string m_message;
    double m_messageTime = 0.0;
    int m_musicGame = -1;              // game shown in the music picker (-1: the saved track's game)
    char m_musicSearch[64] = {};
    bool m_musicScrollToCurrent = true;

    // Per-frame row state.
    int m_rowCount = 0;           // rows laid out this frame
    int m_lastRowCount = 0;
    int m_focus = 0;              // focused row (keys / pad)
    bool m_scrollToFocus = false;
    int m_curRow = -1;
    bool m_rowFocused = false, m_rowHovered = false, m_rowDisabled = false;
    ImVec2 m_valMin, m_valMax;
    bool m_keyLeft = false, m_keyRight = false, m_keyAccept = false;
    bool m_navAllowed = true;
    int m_textFocusRow = -1;      // Enter on a text row puts the caret there
    const char* m_pendingSection = nullptr;
    int m_pendingCat = -1;
    Details m_hover, m_focusDetails;
};

} // namespace noble
