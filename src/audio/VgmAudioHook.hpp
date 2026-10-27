#pragma once

#include <string>
#include <filesystem>
#include <vector>

namespace noble {

class VgmAudioHook {
public:
    static VgmAudioHook& Get();

    bool Initialize(const std::filesystem::path& vgmstreamDir = "bin/vgmstream");
    bool ConvertToWav(const std::filesystem::path& inputPath, const std::filesystem::path& outputPath);
    bool ExtractFsbBank(const std::filesystem::path& fsbPath, const std::filesystem::path& outputDir);

    bool IsAvailable() const { return m_isAvailable; }
    const std::filesystem::path& GetToolPath() const { return m_vgmstreamCli; }

private:
    VgmAudioHook() = default;
    ~VgmAudioHook() = default;

    bool m_isAvailable = false;
    std::filesystem::path m_vgmstreamCli;
};

} // namespace noble
