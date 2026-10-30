#include "FrameCapture.hpp"
#include "ui/PauseMenu.hpp"
#include "common/Logger.hpp"

#include <windows.h>
#include <shellapi.h>
#include <d3d11.h>
#include <wrl/client.h>

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <mutex>
#include <string>
#include <vector>

namespace noble {

namespace {

std::vector<unsigned long long> ParseCaptureFrames() {
    std::vector<unsigned long long> frames;
    int argc = 0;
    LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);
    if (!argv) return frames;
    for (int i = 1; i + 1 < argc; ++i) {
        if (_wcsicmp(argv[i], L"-capture-frames") != 0) continue;
        std::wstring list = argv[i + 1];
        size_t pos = 0;
        while (pos < list.size()) {
            size_t comma = list.find(L',', pos);
            if (comma == std::wstring::npos) comma = list.size();
            const unsigned long long value = wcstoull(list.substr(pos, comma - pos).c_str(), nullptr, 10);
            if (value > 0) frames.push_back(value);
            pos = comma + 1;
        }
    }
    LocalFree(argv);
    std::sort(frames.begin(), frames.end());
    return frames;
}

std::filesystem::path ExeDir() {
    wchar_t path[MAX_PATH] = {};
    GetModuleFileNameW(nullptr, path, MAX_PATH);
    return std::filesystem::path(path).parent_path();
}

bool WriteBmp(const std::filesystem::path& file, const uint8_t* bgra, uint32_t width, uint32_t height) {
    BITMAPFILEHEADER fh{};
    BITMAPINFOHEADER ih{};
    const uint32_t imageSize = width * height * 4;
    fh.bfType = 0x4D42;  // "BM"
    fh.bfOffBits = sizeof(fh) + sizeof(ih);
    fh.bfSize = fh.bfOffBits + imageSize;
    ih.biSize = sizeof(ih);
    ih.biWidth = static_cast<LONG>(width);
    ih.biHeight = -static_cast<LONG>(height);  // top-down
    ih.biPlanes = 1;
    ih.biBitCount = 32;
    ih.biCompression = BI_RGB;
    ih.biSizeImage = imageSize;
    std::ofstream out(file, std::ios::binary);
    if (!out) return false;
    out.write(reinterpret_cast<const char*>(&fh), sizeof(fh));
    out.write(reinterpret_cast<const char*>(&ih), sizeof(ih));
    out.write(reinterpret_cast<const char*>(bgra), imageSize);
    return static_cast<bool>(out);
}

void Capture(IDXGISwapChain* swapChain, unsigned long long frame) {
    using Microsoft::WRL::ComPtr;
    ComPtr<ID3D11Texture2D> backBuffer;
    if (FAILED(swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(backBuffer.GetAddressOf())))) return;
    ComPtr<ID3D11Device> device;
    backBuffer->GetDevice(device.GetAddressOf());
    ComPtr<ID3D11DeviceContext> context;
    device->GetImmediateContext(context.GetAddressOf());

    D3D11_TEXTURE2D_DESC desc{};
    backBuffer->GetDesc(&desc);
    const bool bgr = desc.Format == DXGI_FORMAT_B8G8R8A8_UNORM || desc.Format == DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
    const bool rgb = desc.Format == DXGI_FORMAT_R8G8B8A8_UNORM || desc.Format == DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
    if (!bgr && !rgb) {
        NOBLE_WARN("FrameCapture: unsupported back buffer format {}", static_cast<int>(desc.Format));
        return;
    }

    D3D11_TEXTURE2D_DESC stagingDesc = desc;
    stagingDesc.Usage = D3D11_USAGE_STAGING;
    stagingDesc.BindFlags = 0;
    stagingDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
    stagingDesc.MiscFlags = 0;
    stagingDesc.SampleDesc.Count = 1;
    stagingDesc.SampleDesc.Quality = 0;
    ComPtr<ID3D11Texture2D> staging;
    if (FAILED(device->CreateTexture2D(&stagingDesc, nullptr, staging.GetAddressOf()))) return;
    if (desc.SampleDesc.Count > 1) {
        context->ResolveSubresource(staging.Get(), 0, backBuffer.Get(), 0, desc.Format);
    } else {
        context->CopyResource(staging.Get(), backBuffer.Get());
    }

    D3D11_MAPPED_SUBRESOURCE mapped{};
    if (FAILED(context->Map(staging.Get(), 0, D3D11_MAP_READ, 0, &mapped))) return;
    std::vector<uint8_t> pixels(static_cast<size_t>(desc.Width) * desc.Height * 4);
    for (uint32_t y = 0; y < desc.Height; ++y) {
        const uint8_t* src = static_cast<const uint8_t*>(mapped.pData) + static_cast<size_t>(y) * mapped.RowPitch;
        uint8_t* dst = pixels.data() + static_cast<size_t>(y) * desc.Width * 4;
        for (uint32_t x = 0; x < desc.Width; ++x) {
            const uint8_t* p = src + x * 4;
            uint8_t* q = dst + x * 4;
            q[0] = rgb ? p[2] : p[0];
            q[1] = p[1];
            q[2] = rgb ? p[0] : p[2];
            q[3] = 255;
        }
    }
    context->Unmap(staging.Get(), 0);

    const auto file = ExeDir() / (L"capture_" + std::to_wstring(frame) + L".bmp");
    if (WriteBmp(file, pixels.data(), desc.Width, desc.Height)) {
        NOBLE_INFO("FrameCapture: saved frame {} ({}x{}) to {}", frame, desc.Width, desc.Height, file.string());
    }
}

} // namespace

// "-quit-after <seconds>": test runs end themselves (counted from the first frame).
static double ParseQuitAfter() {
    double seconds = 0.0;
    int argc = 0;
    LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);
    if (!argv) return 0.0;
    for (int i = 1; i + 1 < argc; ++i) {
        if (_wcsicmp(argv[i], L"-quit-after") == 0) seconds = _wtof(argv[i + 1]);
    }
    LocalFree(argv);
    return seconds;
}

void FrameCapture::SaveNow(IDXGISwapChain* swapChain, unsigned long long frame) {
    if (swapChain) Capture(swapChain, frame);
}

void FrameCapture::OnFrame(IDXGISwapChain* swapChain, unsigned long long frame) {
    static std::once_flag s_once;
    static std::vector<unsigned long long> s_frames;
    static double s_quitAfter = 0.0;
    static ULONGLONG s_firstFrameTick = 0;
    std::call_once(s_once, [] {
        s_frames = ParseCaptureFrames();
        s_quitAfter = ParseQuitAfter();
        s_firstFrameTick = GetTickCount64();
        if (s_quitAfter > 0.0) NOBLE_INFO("FrameCapture: -quit-after {:.0f} s armed", s_quitAfter);
    });
    if (s_quitAfter > 0.0 && static_cast<double>(GetTickCount64() - s_firstFrameTick) / 1000.0 > s_quitAfter) {
        NOBLE_INFO("FrameCapture: -quit-after {} s reached, exiting test run", s_quitAfter);
        spdlog::default_logger()->flush();
        TerminateProcess(GetCurrentProcess(), 0);
    }
    if (s_frames.empty() || !swapChain) return;
    if (std::binary_search(s_frames.begin(), s_frames.end(), frame)) {
        Capture(swapChain, frame);
    }
    // Also one shot of the post-game panel, two seconds or so after the match ends.
    static unsigned long long s_gameOverFrame = 0;
    if (PauseMenu::IsGameOver()) {
        if (s_gameOverFrame == 0) s_gameOverFrame = frame;
        if (frame == s_gameOverFrame + 120) Capture(swapChain, frame);
    }
}

} // namespace noble
