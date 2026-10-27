#include "Window.hpp"
#include "common/Logger.hpp"

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

    RECT rc = { 0, 0, static_cast<LONG>(m_width), static_cast<LONG>(m_height) };
    AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);

    int windowWidth = rc.right - rc.left;
    int windowHeight = rc.bottom - rc.top;

    int posX = (GetSystemMetrics(SM_CXSCREEN) - windowWidth) / 2;
    int posY = (GetSystemMetrics(SM_CYSCREEN) - windowHeight) / 2;

    m_hwnd = CreateWindowExW(
        0,
        wc.lpszClassName,
        m_title.c_str(),
        WS_OVERLAPPEDWINDOW,
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

LRESULT Window::HandleMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
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
    }
    return DefWindowProcW(hwnd, uMsg, wParam, lParam);
}

} // namespace noble
