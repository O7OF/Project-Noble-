#pragma once

#include "host/SteamWorkshop.hpp"

#include <filesystem>
#include <functional>
#include <memory>
#include <string>

namespace noble {

// Steam Web API details (title, download size, preview picture) fetched off the render thread.
struct WorkshopDetails;

// The "this lobby needs a Workshop mod you do not have" dialog. It is a modal over the menu, so the
// Server browser never leaves the server list until the player downloads the mod or cancels. DOWNLOAD &
// JOIN asks Steam for the item and continues the pending lobby join once the item is in the Workshop cache.
class ModDownloadPrompt {
public:
    static ModDownloadPrompt& Get();

    // Starts the prompt for one lobby. onReady runs once the item is installed (empty for the screenshot
    // demo) and receives the installed mod folder.
    // demo: draws the dialog for -mod-prompt-demo / screenshots and never starts a real download.
    void Show(const std::string& publishedFileId, const std::string& title,
              std::function<void(const std::filesystem::path&)> onReady, bool demo = false);

    // Draws the modal (and polls an in-flight download). Call once per overlay frame.
    void Render();

    bool IsActive() const { return m_state != State::Hidden; }

    // True when the item is already in Steam's Workshop cache or Noble's data/mods.
    static bool IsInstalled(const std::string& publishedFileId, std::filesystem::path* folder = nullptr);

private:
    enum class State { Hidden, Prompt, Downloading, Finishing, Failed };

    // Closes the popup from inside BeginPopupModal and clears the pending callback.
    void Cancel();
    void BeginDownload();
    // Starts the background Steam Web API request for the item's title, size and preview picture.
    void StartDetailsFetch();

    State m_state = State::Hidden;
    bool m_popupOpened = false;
    bool m_demo = false;
    std::string m_modId;
    std::string m_modTitle;
    std::string m_message;
    std::filesystem::path m_installedFolder;
    std::function<void(const std::filesystem::path&)> m_onReady;
    SteamWorkshop::Download m_download;
    std::shared_ptr<WorkshopDetails> m_details;
};

} // namespace noble
