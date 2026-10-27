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

    HWND GetHwnd() const { return m_hwnd; }
    uint32_t GetWidth() const { return m_width; }
    uint32_t GetHeight() const { return m_height; }
    bool ShouldClose() const { return m_shouldClose; }

    void SetCustomWndProc(MessageCallback callback) { m_customWndProc = callback; }

    // Borderless window covering the primary monitor (DISPLAY setting). Call before Create().
    void SetBorderless(bool borderless) { m_borderless = borderless; }

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
