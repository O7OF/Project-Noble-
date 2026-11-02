#pragma once

#include <windows.h>
#include <string>
#include <functional>

namespace noble {

class Window {
public:
    using MessageCallback = std::function<LRESULT(HWND, UINT, WPARAM, LPARAM)>;

    Window(const std::wstring& title, uint32_t width = 1280, uint32_t height = 720);
    ~Window();

    bool Create();
    void Show(int cmdShow = SW_SHOWNORMAL);
    bool ProcessMessages();

    static Window* Active();

    // Applies a display-mode/window-size change to the live window only. The engine's swap chain is
    // created at the monitor's resolution, so switching windowed <-> borderless needs no restart and
    // no swap-chain resize; Windows scales the same crisp image to the new window.
    void ApplyWindowSettings(uint32_t width, uint32_t height, bool borderless);

    HWND GetHwnd() const { return m_hwnd; }
    uint32_t GetWidth() const { return m_width; }
    uint32_t GetHeight() const { return m_height; }
    bool ShouldClose() const { return m_shouldClose; }

    void SetCustomWndProc(MessageCallback callback) { m_customWndProc = callback; }

    // Borderless window covering the primary monitor (DISPLAY setting). Call before Create().
    void SetBorderless(bool borderless) { m_borderless = borderless; }

    // Mouse wheel notches (WM_MOUSEWHEEL, 120 per notch, up = positive) received since the last call;
    // the remainder of a partial notch is kept. Any thread.
    static int TakeWheelNotches();
    // True while the user is moving or resizing the window (title-bar drag, Alt+Space > Move). The
    // game releases the mouse meanwhile so the window can be moved. Any thread.
    static bool InSizeMove();
    // Left-button presses (WM_LBUTTONDOWN) since the last call. Overlay menus use it so that a click
    // shorter than one frame still counts. Any thread.
    static int TakeLeftClicks();
    // Characters typed while capture is on, for a text line drawn on a thread that can't read the window's messages.
    static void SetCharCapture(bool on);
    static std::wstring TakeTypedChars();

private:
    static LRESULT CALLBACK StaticWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    LRESULT HandleMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    std::wstring m_title;
    uint32_t m_width;
    uint32_t m_height;
    HWND m_hwnd = nullptr;
    HINSTANCE m_hInstance = nullptr;
    bool m_shouldClose = false;
    bool m_borderless = false;
    MessageCallback m_customWndProc;
};

} // namespace noble
