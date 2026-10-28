#pragma once

#include <dxgi.h>

namespace noble {

// Test helper: saves the frame the engine is about to present as a .bmp next to the exe.
// Enabled with  -capture-frames 600,1200,...  (frame numbers counted from the first slot-1 call).
// Files are named capture_<frame>.bmp. Also "-quit-after <seconds>" ends a test run by itself.
// Called from host slot 1, on the engine thread.
class FrameCapture {
public:
    static void OnFrame(IDXGISwapChain* swapChain, unsigned long long frame);
    // Saves the back buffer now as capture_<frame>.bmp (front-end menu tests: -menu-capture).
    static void SaveNow(IDXGISwapChain* swapChain, unsigned long long frame);
};

} // namespace noble
