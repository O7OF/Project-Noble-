#include "ModDownloadPrompt.hpp"
#include "NobleStyle.hpp"
#include "common/Logger.hpp"

#include <imgui.h>

#include <utility>

namespace noble {

namespace {

constexpr const char* kPopupId = "WORKSHOP MOD REQUIRED";
constexpr const char* kMissingMessage =
    "The lobby you are trying to join currently has a mod installed from the Workshop that you do not have.";

} // namespace

ModDownloadPrompt& ModDownloadPrompt::Get() {
    static ModDownloadPrompt prompt;
    return prompt;
}

bool ModDownloadPrompt::IsInstalled(const std::string& publishedFileId, std::filesystem::path* folder) {
    const std::filesystem::path found = SteamWorkshop::FindInstalledFolder(publishedFileId);
    if (folder) *folder = found;
    return !found.empty();
}

void ModDownloadPrompt::Show(const std::string& publishedFileId, const std::string& title,
                             std::function<void(const std::filesystem::path&)> onReady, bool demo) {
    m_download.Cancel();
    m_demo = demo;
    m_modId = publishedFileId;
    m_modTitle = title;
    m_message.clear();
    m_installedFolder.clear();
    m_onReady = std::move(onReady);
    m_state = State::Prompt;
    m_popupOpened = false;
    NOBLE_INFO("ModDownloadPrompt: lobby requires Workshop item {} ('{}')", publishedFileId, title);
}

void ModDownloadPrompt::Cancel() {
    m_download.Cancel();
    m_state = State::Hidden;
    m_popupOpened = false;
    m_onReady = nullptr;
}

void ModDownloadPrompt::BeginDownload() {
    if (m_demo) return;
    m_message.clear();
    if (m_download.Start(m_modId, m_modTitle, m_message)) {
        m_state = State::Downloading;
        NOBLE_INFO("ModDownloadPrompt: download of {} started: {}", m_modId, m_message);
    } else {
        m_state = State::Failed;
        NOBLE_WARN("ModDownloadPrompt: download of {} could not start: {}", m_modId, m_message);
    }
}

void ModDownloadPrompt::Render() {
    if (m_state == State::Hidden) return;

    if (m_state == State::Downloading) {
        m_download.Tick();
        if (m_download.IsDone()) {
            m_installedFolder = m_download.Folder();
            m_state = State::Finishing;
            NOBLE_INFO("ModDownloadPrompt: Workshop item {} installed at '{}'", m_modId, m_installedFolder.string());
        } else if (m_download.IsFailed()) {
            m_message = m_download.Status();
            m_state = State::Failed;
        }
    }

    if (!m_popupOpened) {
        ImGui::OpenPopup(kPopupId);
        m_popupOpened = true;
    }

    ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(640.0f, 0.0f));
    bool joinAfterPopup = false;
    if (ImGui::BeginPopupModal(kPopupId, nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Spacing();
        const bool canAct = m_state == State::Prompt || m_state == State::Failed;
        if (m_state == State::Prompt) {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.92f, 0.95f, 0.98f, 1.0f));
            ImGui::TextWrapped("%s", kMissingMessage);
            ImGui::PopStyleColor();
            ImGui::Spacing();
        }

        if (!m_modTitle.empty()) {
            ImGui::TextColored(NobleStyle::Accent(), "%s", m_modTitle.c_str());
            ImGui::TextDisabled("Workshop item %s", m_modId.c_str());
            ImGui::Spacing();
        }

        if (m_state == State::Prompt) {
            ImGui::TextDisabled("Noble can ask Steam to download it, then join the lobby once it is installed.");
            NobleStyle::SectionGap();
            const bool downloadPressed = NobleStyle::PrimaryButton("DOWNLOAD & JOIN", ImVec2(230.0f, 38.0f));
            if (!m_demo && (downloadPressed ||
                (canAct && ImGui::IsKeyPressed(ImGuiKey_Enter, false)) ||
                (canAct && ImGui::IsKeyPressed(ImGuiKey_GamepadFaceDown, false)))) {
                BeginDownload();
            }
            ImGui::SameLine();
            if (ImGui::Button("CANCEL", ImVec2(130.0f, 38.0f)) ||
                ImGui::IsKeyPressed(ImGuiKey_Escape, false) ||
                ImGui::IsKeyPressed(ImGuiKey_GamepadFaceRight, false)) {
                Cancel();
                ImGui::CloseCurrentPopup();
            }
        } else if (m_state == State::Downloading) {
            ImGui::TextWrapped("%s", m_download.Status().c_str());
            ImGui::Spacing();
            ImGui::ProgressBar(-1.0f * static_cast<float>(ImGui::GetTime()), ImVec2(-1.0f, 8.0f), "");
            ImGui::Spacing();
            if (m_download.NeedsManualSubscribe()) {
                ImGui::TextDisabled("The Steam window may be behind the game. Nothing else is needed here; Noble "
                                    "joins the lobby as soon as Steam finishes the download.");
            }
            NobleStyle::SectionGap();
            if (m_download.NeedsManualSubscribe() && ImGui::Button("OPEN WORKSHOP PAGE", ImVec2(200.0f, 34.0f))) {
                SteamWorkshop::OpenWorkshopPage(m_modId);
            }
            if (m_download.NeedsManualSubscribe()) ImGui::SameLine();
            if (ImGui::Button("CANCEL", ImVec2(130.0f, 34.0f)) || ImGui::IsKeyPressed(ImGuiKey_Escape, false) ||
                ImGui::IsKeyPressed(ImGuiKey_GamepadFaceRight, false)) {
                Cancel();
                ImGui::CloseCurrentPopup();
            }
        } else if (m_state == State::Finishing) {
            ImGui::TextWrapped("The mod is installed. Joining the lobby...");
            ImGui::Spacing();
            ImGui::ProgressBar(1.0f, ImVec2(-1.0f, 8.0f), "");
            ImGui::CloseCurrentPopup();
            joinAfterPopup = true;
        } else if (m_state == State::Failed) {
            ImGui::TextColored(ImVec4(1.0f, 0.45f, 0.42f, 1.0f), "%s", m_message.c_str());
            ImGui::Spacing();
            ImGui::TextDisabled("You can also subscribe on the item's Workshop page, then press RETRY.");
            if (ImGui::Button("OPEN WORKSHOP PAGE", ImVec2(200.0f, 30.0f))) SteamWorkshop::OpenWorkshopPage(m_modId);
            NobleStyle::SectionGap();
            const bool retry = NobleStyle::PrimaryButton("RETRY", ImVec2(160.0f, 38.0f));
            if (!m_demo && (retry || ImGui::IsKeyPressed(ImGuiKey_Enter, false) ||
                            ImGui::IsKeyPressed(ImGuiKey_GamepadFaceDown, false))) {
                BeginDownload();
            }
            ImGui::SameLine();
            if (ImGui::Button("CANCEL", ImVec2(130.0f, 38.0f)) || ImGui::IsKeyPressed(ImGuiKey_Escape, false)) {
                Cancel();
                ImGui::CloseCurrentPopup();
            }
        }

        ImGui::Spacing();
        if (m_state == State::Prompt) ImGui::TextDisabled("ENTER  Download & Join        ESC  Cancel");
        else if (m_state == State::Failed) ImGui::TextDisabled("ENTER  Retry        ESC  Cancel");
        else ImGui::TextDisabled("ESC  Cancel");
        ImGui::EndPopup();
    }

    if (joinAfterPopup) {
        m_state = State::Hidden;
        m_popupOpened = false;
        std::function<void(const std::filesystem::path&)> ready = std::move(m_onReady);
        m_onReady = nullptr;
        if (ready) ready(m_installedFolder);
    }
}

} // namespace noble
