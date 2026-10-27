#include "Window.hpp"
#include "common/Logger.hpp"

#include <atomic>

namespace noble {

Window::Window(const std::wstring& title, uint32_t width, uint32_t height)
    : m_title(title), m_width(width), m_height(height) {
    m_hInstance = GetModuleHandleW(nullptr);
}

Window::~Window() {
    if (m_hwnd) {
        DestroyWindow(m_hwnd);
        m_hwnd = nullptr;
    }
    UnregisterClassW(L"HaloReachCE_WindowClass", m_hInstance);
}

bool Window::Create() {
    WNDCLASSEXW wc = { 0 };
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    wc.lpfnWndProc = StaticWndProc;
    wc.hInstance = m_hInstance;
    wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    wc.hbrBackground = reinterpret_cast<HBRUSH>(GetStockObject(BLACK_BRUSH));
    wc.lpszClassName = L"HaloReachCE_WindowClass";

    if (!RegisterClassExW(&wc)) {
        NOBLE_ERROR("Failed to register Win32 window class.");
        return false;
    }

    // Fixed-size window: the engine renders at the size chosen in Settings and can't change it once
    // started, so a maximized or dragged-larger window only stretched that picture. For a full-screen
    // picture use WINDOW MODE = BORDERLESS FULL SCREEN, which renders at the monitor's size.
    DWORD style = WS_OVERLAPPEDWINDOW & ~(WS_THICKFRAME | WS_MAXIMIZEBOX);
    int windowWidth = 0, windowHeight = 0, posX = 0, posY = 0;
    if (m_borderless) {
        // Borderless: a popup window the size of the primary monitor.
        style = WS_POPUP;
        m_width = static_cast<uint32_t>(GetSystemMetrics(SM_CXSCREEN));
        m_height = static_cast<uint32_t>(GetSystemMetrics(SM_CYSCREEN));
        windowWidth = static_cast<int>(m_width);
        windowHeight = static_cast<int>(m_height);
    } else {
        RECT rc = { 0, 0, static_cast<LONG>(m_width), static_cast<LONG>(m_height) };
        AdjustWindowRect(&rc, style, FALSE);
        windowWidth = rc.right - rc.left;
        windowHeight = rc.bottom - rc.top;
        posX = (GetSystemMetrics(SM_CXSCREEN) - windowWidth) / 2;
        posY = (GetSystemMetrics(SM_CYSCREEN) - windowHeight) / 2;
        if (posX < 0) posX = 0;
        if (posY < 0) posY = 0;
    }

    m_hwnd = CreateWindowExW(
        0,
        wc.lpszClassName,
        m_title.c_str(),
        style,
        posX, posY,
        windowWidth, windowHeight,
        nullptr, nullptr,
        m_hInstance,
        this
    );

    if (!m_hwnd) {
        NOBLE_ERROR("Failed to create Win32 host window. Error: {}", GetLastError());
        return false;
    }

    NOBLE_INFO("Win32 host window created successfully ({}x{}). HWND: 0x{:X}", 
        m_width, m_height, reinterpret_cast<uintptr_t>(m_hwnd));

    return true;
}

void Window::Show(int cmdShow) {
    if (m_hwnd) {
        ShowWindow(m_hwnd, cmdShow);
        UpdateWindow(m_hwnd);
    }
}

bool Window::ProcessMessages() {
    MSG msg = { 0 };
    while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)) {
        if (msg.message == WM_QUIT) {
            m_shouldClose = true;
            return false;
        }
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    return !m_shouldClose;
}

LRESULT CALLBACK Window::StaticWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    Window* pThis = nullptr;
    if (uMsg == WM_NCCREATE) {
        CREATESTRUCTW* pCreate = reinterpret_cast<CREATESTRUCTW*>(lParam);
        pThis = reinterpret_cast<Window*>(pCreate->lpCreateParams);
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pThis));
    } else {
        pThis = reinterpret_cast<Window*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    }

    if (pThis) {
        return pThis->HandleMessage(hwnd, uMsg, wParam, lParam);
    }
    return DefWindowProcW(hwnd, uMsg, wParam, lParam);
}

namespace {
std::atomic<int> s_wheelDelta{0};
std::atomic<bool> s_sizeMove{false};
} // namespace

int Window::TakeWheelNotches() {
    int delta = s_wheelDelta.load();
    for (;;) {
        const int notches = delta / WHEEL_DELTA;
        if (notches == 0) return 0;
        if (s_wheelDelta.compare_exchange_weak(delta, delta - notches * WHEEL_DELTA)) return notches;
    }
}

bool Window::InSizeMove() { return s_sizeMove.load(); }

LRESULT Window::HandleMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_MOUSEWHEEL: s_wheelDelta.fetch_add(GET_WHEEL_DELTA_WPARAM(wParam)); break;
        case WM_ENTERSIZEMOVE: s_sizeMove.store(true); ClipCursor(nullptr); break;
        case WM_EXITSIZEMOVE: s_sizeMove.store(false); break;
        case WM_KILLFOCUS: ClipCursor(nullptr); break;
        default: break;
    }
    if (m_customWndProc) {
        LRESULT res = m_customWndProc(hwnd, uMsg, wParam, lParam);
        if (res != 0) return res;
    }

    switch (uMsg) {
        case WM_ERASEBKGND:
            return 1; // Prevent GDI from clearing background over DirectX 11 presentation
        case WM_CLOSE:
            m_shouldClose = true;
            DestroyWindow(hwnd);
            return 0;
        case WM_DESTROY:
            m_shouldClose = true;
            PostQuitMessage(0);
            return 0;
        case WM_SIZE:
            m_width = LOWORD(lParam);
            m_height = HIWORD(lParam);
            return 0;
        case WM_SETCURSOR:
            // Hide the arrow over the game area while the window is focused (the engine thread
            // can't do this itself: SetCursor/ShowCursor only work from the window's own thread).
            // ReachHost clears the class cursor while it owns the mouse (mouse look) and restores the
            // arrow while a menu needs the pointer, so hide it only when the class cursor is cleared.
            if (LOWORD(lParam) == HTCLIENT && GetForegroundWindow() == hwnd &&
                GetClassLongPtrW(hwnd, GCLP_HCURSOR) == 0) {
                SetCursor(nullptr);
                return TRUE;
            }
            break;
    }
    return DefWindowProcW(hwnd, uMsg, wParam, lParam);
}

} // namespace noble
