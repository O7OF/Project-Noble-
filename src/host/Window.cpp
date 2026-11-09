#include "Window.hpp"
#include "common/Logger.hpp"

#include <algorithm>
#include <atomic>
#include <mutex>
#include <string>

namespace noble {

namespace {
Window* s_activeWindow = nullptr;
std::atomic<HWND> s_activeHwnd{ nullptr };
std::atomic<bool> s_exclusiveFullscreenSetting{ false };
std::atomic<bool> s_exclusiveFullscreenActive{ false };
// Posted by the render thread after it leaves exclusive full screen: re-apply the window geometry.
constexpr UINT kMsgRefreshGeometry = WM_APP + 0x4E;
// Posted by RequestFocus(): the window's own thread takes the keyboard focus back from the container
// window it is hosted in (see EnsureContainerFocus).
constexpr UINT kMsgEnsureFocus = WM_APP + 0x4F;

// Container window mode: this process's window is a WS_CHILD of the launcher's top-level window instead
// of a top-level window of its own. Set once, in Create().
std::atomic<bool> s_containerMode{false};

// The top-level window at the root of hwnd's parent chain (hwnd itself for an ordinary window).
HWND RootWindowOf(HWND hwnd) { return hwnd ? GetAncestor(hwnd, GA_ROOT) : nullptr; }
}

Window* Window::Active() { return s_activeWindow; }

HWND Window::ActiveHwnd() { return s_activeHwnd.load(); }

bool Window::IsForeground(HWND hwnd) {
    if (!hwnd) return false;
    const HWND foreground = GetForegroundWindow();
    if (!foreground) return false;
    if (foreground == hwnd) return true;
    // Container window mode: the match renders into a WS_CHILD window inside the launcher's top-level
    // window, and Windows only ever reports a top-level window as the foreground one. That child is still
    // "in front" whenever the launcher is, so accept any foreground window it is a descendant of. IsChild
    // walks the window tree, which is shared by every process on the desktop (mouse clicks reach the child
    // by position, and the launcher forwards nothing), so this is exactly the container case.
    return IsChild(foreground, hwnd) != FALSE;
}

void Window::RequestFocus() {
    const HWND hwnd = s_activeHwnd.load();
    if (hwnd) PostMessageW(hwnd, kMsgEnsureFocus, 0, 0);
}

void Window::AttachContainerInput() {
    if (m_containerInputAttached || !m_hwnd || !m_containerParentHwnd) return;
    m_containerInputAttached = true;
    const DWORD ownThread = GetCurrentThreadId();
    const DWORD containerThread = GetWindowThreadProcessId(RootWindowOf(m_containerParentHwnd), nullptr);
    if (containerThread == 0 || containerThread == ownThread) return;
    // A thread can only give the focus to a window on its own input queue, and the window Windows makes
    // foreground - the launcher's - belongs to the launcher's thread. Joining the two queues lets this child
    // be the focused window of the shared queue, so keys, the mouse wheel and cursor confinement follow it
    // again, exactly as they did when a match owned a top-level window. The foreground window stays the
    // launcher, which is why focus tests use IsForeground() and not GetForegroundWindow().
    if (AttachThreadInput(ownThread, containerThread, TRUE)) {
        NOBLE_INFO("Win32 container child window: input queue attached to the launcher (threads {} and {})",
                   ownThread, containerThread);
    } else {
        m_containerInputAttached = false;   // retried by the next EnsureContainerFocus
        NOBLE_WARN("Win32 container child window: AttachThreadInput to the launcher failed (error {}); "
                   "keyboard focus stays with the launcher", GetLastError());
    }
}

void Window::EnsureContainerFocus() {
    if (!IsForeground(m_hwnd)) return;   // the user is in another app: never steal the focus
    AttachContainerInput();
    if (GetFocus() == m_hwnd) return;
    SetFocus(m_hwnd);
    static ULONGLONG s_lastLog = 0;   // the focus can be lost and retaken repeatedly: log at most every 5 s
    if (GetTickCount64() < s_lastLog + 5000) return;
    s_lastLog = GetTickCount64();
    if (GetFocus() == m_hwnd) {
        NOBLE_INFO("Win32 container child window: keyboard focus taken from the launcher");
    } else {
        NOBLE_WARN("Win32 container child window: SetFocus failed (error {}); input is still polled directly",
                   GetLastError());
    }
}

void Window::DetachContainerInput() {
    if (!m_containerInputAttached) return;
    m_containerInputAttached = false;
    if (!m_containerParentHwnd) return;
    const DWORD ownThread = GetCurrentThreadId();
    const DWORD containerThread = GetWindowThreadProcessId(RootWindowOf(m_containerParentHwnd), nullptr);
    if (containerThread != 0 && containerThread != ownThread) {
        AttachThreadInput(ownThread, containerThread, FALSE);
        NOBLE_INFO("Win32 container child window: input queue detached from the launcher");
    }
}

Window::Window(const std::wstring& title, uint32_t width, uint32_t height)
    : m_title(title), m_width(width), m_height(height), m_requestedWidth(width), m_requestedHeight(height) {
    s_activeWindow = this;
    m_hInstance = GetModuleHandleW(nullptr);
}

void Window::SetExclusiveFullscreen(bool on) {
    m_exclusiveFullscreen = on;
    if (on) m_borderless = true;
    s_exclusiveFullscreenSetting.store(on);
}

bool Window::WantsExclusiveFullscreen() {
    if (!s_exclusiveFullscreenSetting.load()) return false;
    // A hosted child window cannot own the display: container window mode is always borderless.
    if (s_containerMode.load()) return false;
    const HWND hwnd = s_activeHwnd.load();
    // Only while Noble is the foreground app: Alt-Tab, a click on another monitor or Win+D drops out,
    // and coming back re-enters. DXGI itself also leaves full screen on focus loss.
    return hwnd != nullptr && Window::IsForeground(hwnd) && !IsIconic(hwnd);
}

bool Window::IsExclusiveFullscreenActive() { return s_exclusiveFullscreenActive.load(); }
void Window::SetExclusiveFullscreenActive(bool on) { s_exclusiveFullscreenActive.store(on); }

void Window::RequestGeometryRefresh() {
    if (m_hwnd) PostMessageW(m_hwnd, kMsgRefreshGeometry, 0, 0);
}

Window::~Window() {
    if (s_activeWindow == this) s_activeWindow = nullptr;
    DetachContainerInput();
    if (m_hwnd) {
        DestroyWindow(m_hwnd);
        m_hwnd = nullptr;
    }
    UnregisterClassW(L"HaloReachCE_WindowClass", m_hInstance);
}

void Window::ApplyWindowSettings(uint32_t width, uint32_t height, bool borderless, bool exclusiveFullscreen) {
    if (!m_hwnd) return;
    m_exclusiveFullscreen = exclusiveFullscreen;
    m_borderless = borderless || exclusiveFullscreen;
    m_width = width;
    m_height = height;
    m_requestedWidth = width;
    m_requestedHeight = height;
    s_exclusiveFullscreenSetting.store(exclusiveFullscreen);

    // Container window mode: the launcher owns the window (and the display mode), and this window is a
    // child that fills its client area. Restyling it here would rip it out of the container.
    if (m_containerParentHwnd) return;

    // While DXGI owns the window in exclusive full screen, restyling it would fight the swap chain. The
    // render thread leaves full screen on its next Present (the setting is now off) and then posts
    // kMsgRefreshGeometry, which lands back here with the new geometry.
    if (IsExclusiveFullscreenActive()) return;

    MONITORINFO monitorInfo{};
    monitorInfo.cbSize = sizeof(monitorInfo);
    const HMONITOR monitor = MonitorFromWindow(m_hwnd, MONITOR_DEFAULTTOPRIMARY);
    GetMonitorInfoW(monitor, &monitorInfo);
    const RECT monitorRect = monitorInfo.rcMonitor;
    const RECT workRect = monitorInfo.rcWork;

    if (m_borderless) {
        SetWindowLongPtrW(m_hwnd, GWL_STYLE, WS_POPUP | WS_VISIBLE);
        const int w = monitorRect.right - monitorRect.left;
        const int h = monitorRect.bottom - monitorRect.top;
        SetWindowPos(m_hwnd, HWND_TOP, monitorRect.left, monitorRect.top, w, h,
                     SWP_FRAMECHANGED | SWP_SHOWWINDOW);
    } else {
        const DWORD style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
        RECT rect = { 0, 0, static_cast<LONG>(width), static_cast<LONG>(height) };
        AdjustWindowRect(&rect, style, FALSE);
        int winW = rect.right - rect.left;
        int winH = rect.bottom - rect.top;
        const int workW = workRect.right - workRect.left;
        const int workH = workRect.bottom - workRect.top;
        const int maxClientW = std::max(320, workW - (winW - static_cast<int>(width)));
        const int maxClientH = std::max(240, workH - (winH - static_cast<int>(height)));
        if (static_cast<int>(width) > maxClientW || static_cast<int>(height) > maxClientH) {
            const float scale = std::min(static_cast<float>(maxClientW) / static_cast<float>(width),
                                         static_cast<float>(maxClientH) / static_cast<float>(height));
            const int fittedW = std::max(320, static_cast<int>(static_cast<float>(width) * scale));
            const int fittedH = std::max(240, static_cast<int>(static_cast<float>(height) * scale));
            rect = { 0, 0, fittedW, fittedH };
            AdjustWindowRect(&rect, style, FALSE);
            winW = rect.right - rect.left;
            winH = rect.bottom - rect.top;
        }
        const int posX = workRect.left + (workW - winW) / 2;
        const int posY = workRect.top + (workH - winH) / 2;
        SetWindowLongPtrW(m_hwnd, GWL_STYLE, style);
        SetWindowPos(m_hwnd, HWND_NOTOPMOST, posX, posY, winW, winH, SWP_FRAMECHANGED | SWP_SHOWWINDOW);
    }
    SetForegroundWindow(m_hwnd);
}

bool Window::Create() {
    WNDCLASSEXW wc = { 0 };
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    wc.lpfnWndProc = StaticWndProc;
    wc.hInstance = m_hInstance;
    wc.hIcon = LoadIconW(m_hInstance, MAKEINTRESOURCEW(101));
    wc.hIconSm = LoadIconW(m_hInstance, MAKEINTRESOURCEW(101));
    wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    wc.hbrBackground = reinterpret_cast<HBRUSH>(GetStockObject(BLACK_BRUSH));
    wc.lpszClassName = L"HaloReachCE_WindowClass";

    if (!RegisterClassExW(&wc)) {
        NOBLE_ERROR("Failed to register Win32 window class.");
        return false;
    }

    // Container Window Mode: if hosted inside a container parent window, create a WS_CHILD window
    // that fits the container's client area completely, so the user never experiences the window closing
    // and opening a new one.
    if (m_containerParentHwnd && IsWindow(m_containerParentHwnd)) {
        // Ensure container parent window clips children so its rendering/swapchain cannot paint over child
        LONG_PTR parentStyle = GetWindowLongPtrW(m_containerParentHwnd, GWL_STYLE);
        if (!(parentStyle & WS_CLIPCHILDREN)) {
            SetWindowLongPtrW(m_containerParentHwnd, GWL_STYLE, parentStyle | WS_CLIPCHILDREN | WS_CLIPSIBLINGS);
        }

        RECT parentRc{};
        GetClientRect(m_containerParentHwnd, &parentRc);
        int parentW = parentRc.right - parentRc.left;
        int parentH = parentRc.bottom - parentRc.top;
        if (parentW > 0 && parentH > 0) {
            m_width = static_cast<uint32_t>(parentW);
            m_height = static_cast<uint32_t>(parentH);
        }
        m_hwnd = CreateWindowExW(
            WS_EX_NOPARENTNOTIFY,
            wc.lpszClassName,
            m_title.c_str(),
            WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
            0, 0,
            static_cast<int>(m_width), static_cast<int>(m_height),
            m_containerParentHwnd, nullptr,
            m_hInstance,
            this
        );
        if (!m_hwnd) {
            NOBLE_ERROR("Failed to create Win32 child window in container parent (error {}).", GetLastError());
            return false;
        }
        s_activeHwnd.store(m_hwnd);
        s_containerMode.store(true);
        SetWindowPos(m_hwnd, HWND_TOP, 0, 0, static_cast<int>(m_width), static_cast<int>(m_height), SWP_SHOWWINDOW | SWP_FRAMECHANGED);
        BringWindowToTop(m_hwnd);
        EnsureContainerFocus();
        NOBLE_INFO("Win32 container child window created: {}x{} [Parent: 0x{:X}]",
                   m_width, m_height, reinterpret_cast<uintptr_t>(m_containerParentHwnd));
        return true;
    }

    // Fixed-size window: the engine renders at the size chosen in Settings and cannot change that once
    // it has started, so a maximized or dragged-larger window can only stretch (and pixelate) that picture.
    // WINDOW MODE = BORDERLESS FULL SCREEN is the full-screen path: it creates the window at the monitor's
    // size. The menu process additionally supersamples 1.5x and the window downscales that cleanly.
    DWORD style = (WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_CLIPCHILDREN | WS_CLIPSIBLINGS);
    int windowWidth = 0, windowHeight = 0, posX = 0, posY = 0;
    if (m_borderless) {
        // Borderless: a popup window the size of the primary monitor.
        style = (WS_POPUP | WS_CLIPCHILDREN | WS_CLIPSIBLINGS);
        m_width = static_cast<uint32_t>(GetSystemMetrics(SM_CXSCREEN));
        m_height = static_cast<uint32_t>(GetSystemMetrics(SM_CYSCREEN));
        windowWidth = static_cast<int>(m_width);
        windowHeight = static_cast<int>(m_height);
    } else {
        RECT work{};
        if (!SystemParametersInfoW(SPI_GETWORKAREA, 0, &work, 0)) {
            work = { 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN) };
        }
        RECT rc = { 0, 0, static_cast<LONG>(m_width), static_cast<LONG>(m_height) };
        AdjustWindowRect(&rc, style, FALSE);
        windowWidth = rc.right - rc.left;
        windowHeight = rc.bottom - rc.top;

        // A 1920x1080 client plus its frame is wider/taller than a 1920x1080 desktop. Shrink a window
        // that would not fit the work area (keeping its aspect), so normal windowed mode always opens
        // fully on screen and the engine renders at exactly the client size.
        const int workW = work.right - work.left;
        const int workH = work.bottom - work.top;
        const int borderW = windowWidth - static_cast<int>(m_width);
        const int borderH = windowHeight - static_cast<int>(m_height);
        const int maxClientW = std::max(320, workW - borderW);
        const int maxClientH = std::max(240, workH - borderH);
        if (static_cast<int>(m_width) > maxClientW || static_cast<int>(m_height) > maxClientH) {
            const float scale = std::min(static_cast<float>(maxClientW) / static_cast<float>(m_width),
                                         static_cast<float>(maxClientH) / static_cast<float>(m_height));
            const int fittedW = std::max(320, static_cast<int>(static_cast<float>(m_width) * scale));
            const int fittedH = std::max(240, static_cast<int>(static_cast<float>(m_height) * scale));
            NOBLE_INFO("Win32 window: {}x{} does not fit the {}x{} work area; opening at {}x{}",
                       m_width, m_height, workW, workH, fittedW, fittedH);
            m_width = static_cast<uint32_t>(fittedW);
            m_height = static_cast<uint32_t>(fittedH);
            RECT fitted = { 0, 0, static_cast<LONG>(m_width), static_cast<LONG>(m_height) };
            AdjustWindowRect(&fitted, style, FALSE);
            windowWidth = fitted.right - fitted.left;
            windowHeight = fitted.bottom - fitted.top;
        }
        posX = work.left + (workW - windowWidth) / 2;
        posY = work.top + (workH - windowHeight) / 2;
        if (posX < work.left) posX = work.left;
        if (posY < work.top) posY = work.top;
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

    s_activeHwnd.store(m_hwnd);

    SendMessageW(m_hwnd, WM_SETICON, ICON_BIG, reinterpret_cast<LPARAM>(wc.hIcon));
    SendMessageW(m_hwnd, WM_SETICON, ICON_SMALL, reinterpret_cast<LPARAM>(wc.hIconSm));

    NOBLE_INFO("Win32 host window created successfully ({}x{}). HWND: 0x{:X}", 
        m_width, m_height, reinterpret_cast<uintptr_t>(m_hwnd));

    return true;
}

void Window::Show(int cmdShow) {
    if (m_hwnd) {
        ShowWindow(m_hwnd, m_containerParentHwnd ? SW_SHOW : cmdShow);
        UpdateWindow(m_hwnd);
        if (m_containerParentHwnd) EnsureContainerFocus();
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
std::atomic<int> s_leftClicks{0};
} // namespace

int Window::TakeLeftClicks() { return s_leftClicks.exchange(0); }

namespace {
std::atomic<bool> s_charCapture{ false };
std::mutex s_typedMutex;
std::wstring s_typedChars;
}

void Window::SetCharCapture(bool on) {
    std::lock_guard<std::mutex> lock(s_typedMutex);
    s_typedChars.clear();
    s_charCapture.store(on);
}

std::wstring Window::TakeTypedChars() {
    std::lock_guard<std::mutex> lock(s_typedMutex);
    std::wstring out;
    out.swap(s_typedChars);
    return out;
}

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
    // Test-only: keep a capture run from reacting to the mouse/keyboard (see "-menu-ignore-input").
    static const bool s_ignoreInput = wcsstr(GetCommandLineW(), L"-menu-ignore-input") != nullptr;
    if (s_ignoreInput) {
        const bool mouse = (uMsg >= WM_MOUSEFIRST && uMsg <= WM_MOUSELAST) || uMsg == WM_MOUSEWHEEL ||
                           uMsg == WM_MOUSELEAVE || uMsg == WM_SETCURSOR;
        const bool key = (uMsg >= WM_KEYFIRST && uMsg <= WM_KEYLAST);
        if (mouse || key) return 0;
    }
    switch (uMsg) {
        case WM_MOUSEWHEEL: s_wheelDelta.fetch_add(GET_WHEEL_DELTA_WPARAM(wParam)); break;
        case WM_LBUTTONDOWN: s_leftClicks.fetch_add(1); break;
        case WM_CHAR:
            if (s_charCapture.load()) {
                std::lock_guard<std::mutex> lock(s_typedMutex);
                if (s_typedChars.size() < 256) s_typedChars.push_back(static_cast<wchar_t>(wParam));
            }
            break;
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
        case kMsgRefreshGeometry:
            if (!IsExclusiveFullscreenActive()) {
                ApplyWindowSettings(m_requestedWidth, m_requestedHeight, m_borderless, m_exclusiveFullscreen);
            }
            return 0;
        case kMsgEnsureFocus:
            // Handled on the window's own thread, which is the only one that can set the focus.
            if (m_containerParentHwnd) {
                EnsureContainerFocus();
            } else {
                SetForegroundWindow(m_hwnd);
                SetFocus(m_hwnd);
            }
            return 0;
        case WM_DESTROY:
            DetachContainerInput();
            s_activeHwnd.store(nullptr);
            m_shouldClose = true;
            PostQuitMessage(0);
            return 0;
        case WM_SIZE:
            m_width = LOWORD(lParam);
            m_height = HIWORD(lParam);
            break;
            // DefWindowProcW handles sizing finish
            return 0;
        case WM_SETCURSOR:
            // Hide the arrow over the game area while the window is focused (the engine thread
            // can't do this itself: SetCursor/ShowCursor only work from the window's own thread).
            // ReachHost clears the class cursor while it owns the mouse (mouse look) and restores the
            // arrow while a menu needs the pointer, so hide it only when the class cursor is cleared.
            if (LOWORD(lParam) == HTCLIENT && IsForeground(hwnd) &&
                GetClassLongPtrW(hwnd, GCLP_HCURSOR) == 0) {
                SetCursor(nullptr);
                return TRUE;
            }
            break;
    }
    return DefWindowProcW(hwnd, uMsg, wParam, lParam);
}

} // namespace noble
