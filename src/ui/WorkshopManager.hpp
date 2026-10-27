#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <filesystem>

namespace noble {

struct ModItem {
    std::string id;            // Workshop PublishedFileId or local folder name
    std::string title;         // from ModInfo.json "Title"
    std::string description;   // from ModInfo.json "Description"
    std::string author;
    uintmax_t sizeBytes = 0;
    std::vector<std::string> mapFiles;
    bool isEnabled = true;
    bool isFromWorkshop = true;
    std::filesystem::path folderPath;
};

class WorkshopManager {
public:
    WorkshopManager();
    ~WorkshopManager() = default;

    void RefreshMods(const std::filesystem::path& workshopPath, const std::filesystem::path& localModsPath);
    void Render();

    const std::vector<ModItem>& GetMods() const { return m_mods; }

private:
    std::vector<ModItem> m_mods;
    int m_selectedModIndex = -1;
    int m_skippedOtherGames = 0;   // Workshop items for Halo 3, CE, etc.
    std::filesystem::path m_workshopPath;
    std::filesystem::path m_localModsPath;
};

} // namespace noble
