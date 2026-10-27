#include "VgmAudioHook.hpp"
#include "common/Logger.hpp"
#include <windows.h>
#include <sstream>

namespace noble {

VgmAudioHook& VgmAudioHook::Get() {
    static VgmAudioHook s_instance;
    return s_instance;
}

bool VgmAudioHook::Initialize(const std::filesystem::path& vgmstreamDir) {
    std::filesystem::path candidate1 = vgmstreamDir / "vgmstream-cli.exe";
    std::filesystem::path candidate2 = std::filesystem::current_path() / "bin" / "vgmstream" / "vgmstream-cli.exe";
    std::filesystem::path candidate3 = "bin/vgmstream/vgmstream-cli.exe";

    if (std::filesystem::exists(candidate1)) {
        m_vgmstreamCli = candidate1;
    } else if (std::filesystem::exists(candidate2)) {
        m_vgmstreamCli = candidate2;
    } else if (std::filesystem::exists(candidate3)) {
        m_vgmstreamCli = candidate3;
    }

    if (!m_vgmstreamCli.empty() && std::filesystem::exists(m_vgmstreamCli)) {
        m_isAvailable = true;
        NOBLE_INFO("VgmAudioHook: Found vgmstream CLI decoder at '{}'", m_vgmstreamCli.string());
        return true;
    }

    NOBLE_WARN("VgmAudioHook: vgmstream-cli.exe not found. Audio extraction/decoding will be disabled.");
    m_isAvailable = false;
    return false;
}

bool VgmAudioHook::ConvertToWav(const std::filesystem::path& inputPath, const std::filesystem::path& outputPath) {
    if (!m_isAvailable) {
        if (!Initialize()) return false;
    }

    if (!std::filesystem::exists(inputPath)) {
        NOBLE_ERROR("VgmAudioHook: Input audio file does not exist: '{}'", inputPath.string());
        return false;
    }

    std::filesystem::create_directories(outputPath.parent_path());

    std::wstring cmd = L"\"" + m_vgmstreamCli.wstring() + L"\" -o \"" + outputPath.wstring() + L"\" \"" + inputPath.wstring() + L"\"";

    STARTUPINFOW si{};
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;

    PROCESS_INFORMATION pi{};

    std::vector<wchar_t> cmdBuffer(cmd.begin(), cmd.end());
    cmdBuffer.push_back(L'\0');

    if (!CreateProcessW(nullptr, cmdBuffer.data(), nullptr, nullptr, FALSE, CREATE_NO_WINDOW, nullptr, nullptr, &si, &pi)) {
        NOBLE_ERROR("VgmAudioHook: Failed to spawn vgmstream-cli process.");
        return false;
    }

    WaitForSingleObject(pi.hProcess, 15000); // 15 sec timeout
    DWORD exitCode = 1;
    GetExitCodeProcess(pi.hProcess, &exitCode);

    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);

    if (exitCode == 0 && std::filesystem::exists(outputPath)) {
        NOBLE_INFO("VgmAudioHook: Successfully converted '{}' -> '{}'", inputPath.string(), outputPath.string());
        return true;
    }

    NOBLE_ERROR("VgmAudioHook: Conversion failed with exit code: {}", exitCode);
    return false;
}

bool VgmAudioHook::ExtractFsbBank(const std::filesystem::path& fsbPath, const std::filesystem::path& outputDir) {
    if (!m_isAvailable) {
        if (!Initialize()) return false;
    }

    if (!std::filesystem::exists(fsbPath)) {
        NOBLE_ERROR("VgmAudioHook: FSB soundbank does not exist: '{}'", fsbPath.string());
        return false;
    }

    std::filesystem::create_directories(outputDir);

    // Batch extract all subsongs using -S option: vgmstream-cli -S 0 -o outputDir/?n.wav input.fsb
    std::wstring outPattern = (outputDir / L"?n.wav").wstring();
    std::wstring cmd = L"\"" + m_vgmstreamCli.wstring() + L"\" -S 0 -o \"" + outPattern + L"\" \"" + fsbPath.wstring() + L"\"";

    STARTUPINFOW si{};
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;

    PROCESS_INFORMATION pi{};

    std::vector<wchar_t> cmdBuffer(cmd.begin(), cmd.end());
    cmdBuffer.push_back(L'\0');

    if (!CreateProcessW(nullptr, cmdBuffer.data(), nullptr, nullptr, FALSE, CREATE_NO_WINDOW, nullptr, nullptr, &si, &pi)) {
        NOBLE_ERROR("VgmAudioHook: Failed to spawn vgmstream-cli for soundbank extraction.");
        return false;
    }

    WaitForSingleObject(pi.hProcess, 30000);
    DWORD exitCode = 1;
    GetExitCodeProcess(pi.hProcess, &exitCode);

    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);

    NOBLE_INFO("VgmAudioHook: Soundbank extraction for '{}' completed (ExitCode: {})", fsbPath.string(), exitCode);
    return (exitCode == 0);
}

} // namespace noble
