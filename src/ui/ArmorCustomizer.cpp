#include "ArmorCustomizer.hpp"
#include <imgui.h>
#include <nlohmann/json.hpp>
#include <fstream>
#include <array>

namespace noble {

// Classic Halo: Reach 30-color multiplayer palette (RGB)
static const std::array<ImVec4, 30> REACH_COLOR_PALETTE = {{
    ImVec4(0.20f, 0.22f, 0.25f, 1.0f), // 0: Steel
    ImVec4(0.55f, 0.58f, 0.62f, 1.0f), // 1: Silver
    ImVec4(0.92f, 0.92f, 0.94f, 1.0f), // 2: White
    ImVec4(0.85f, 0.55f, 0.20f, 1.0f), // 3: Gold
    ImVec4(0.60f, 0.32f, 0.15f, 1.0f), // 4: Rust
    ImVec4(0.38f, 0.22f, 0.12f, 1.0f), // 5: Brown
    ImVec4(0.35f, 0.32f, 0.18f, 1.0f), // 6: Khaki
    ImVec4(0.30f, 0.35f, 0.18f, 1.0f), // 7: Olive
    ImVec4(0.42f, 0.48f, 0.35f, 1.0f), // 8: Sage
    ImVec4(0.18f, 0.35f, 0.18f, 1.0f), // 9: Forest
    ImVec4(0.12f, 0.40f, 0.38f, 1.0f), // 10: Teal
    ImVec4(0.15f, 0.55f, 0.60f, 1.0f), // 11: Aqua
    ImVec4(0.10f, 0.70f, 0.88f, 1.0f), // 12: Cyan
    ImVec4(0.15f, 0.38f, 0.68f, 1.0f), // 13: Cobalt
    ImVec4(0.12f, 0.22f, 0.58f, 1.0f), // 14: Blue
    ImVec4(0.32f, 0.22f, 0.58f, 1.0f), // 15: Violet
    ImVec4(0.48f, 0.18f, 0.55f, 1.0f), // 16: Purple
    ImVec4(0.72f, 0.25f, 0.60f, 1.0f), // 17: Pink
    ImVec4(0.78f, 0.32f, 0.45f, 1.0f), // 18: Rose
    ImVec4(0.72f, 0.12f, 0.12f, 1.0f), // 19: Crimson
    ImVec4(0.55f, 0.10f, 0.10f, 1.0f), // 20: Maroon
    ImVec4(0.58f, 0.22f, 0.18f, 1.0f), // 21: Brick
    ImVec4(0.85f, 0.40f, 0.30f, 1.0f), // 22: Coral
    ImVec4(0.90f, 0.62f, 0.50f, 1.0f), // 23: Peach
    ImVec4(0.92f, 0.75f, 0.25f, 1.0f), // 24: Yellow
    ImVec4(0.82f, 0.68f, 0.42f, 1.0f), // 25: Sand
    ImVec4(0.45f, 0.42f, 0.38f, 1.0f), // 26: Tan
    ImVec4(0.38f, 0.42f, 0.45f, 1.0f), // 27: Smoke
    ImVec4(0.28f, 0.32f, 0.35f, 1.0f), // 28: Slate
    ImVec4(0.10f, 0.10f, 0.12f, 1.0f), // 29: Obsidian
}};

static const char* REACH_COLOR_NAMES[] = {
    "Steel", "Silver", "White", "Gold", "Rust", "Brown", "Khaki", "Olive", "Sage", "Forest",
    "Teal", "Aqua", "Cyan", "Cobalt", "Blue", "Violet", "Purple", "Pink", "Rose", "Crimson",
    "Maroon", "Brick", "Coral", "Peach", "Yellow", "Sand", "Tan", "Smoke", "Slate", "Obsidian"
};

ArmorCustomizer::ArmorCustomizer() {
    LoadProfile();
}

bool ArmorCustomizer::LoadProfile(const std::filesystem::path& path) {
    if (!std::filesystem::exists(path)) return false;

    try {
        std::ifstream file(path);
        nlohmann::json j;
        file >> j;

        m_profile.species = j.value("species", 0);
        m_profile.gender = j.value("gender", 0);
        m_profile.helmet = j.value("helmet", 0);
        m_profile.leftShoulder = j.value("leftShoulder", 0);
        m_profile.rightShoulder = j.value("rightShoulder", 0);
        m_profile.chest = j.value("chest", 0);
        m_profile.wrist = j.value("wrist", 0);
        m_profile.utility = j.value("utility", 0);
        m_profile.visorColor = j.value("visorColor", 0);
        m_profile.kneeGuards = j.value("kneeGuards", 0);
        m_profile.armorEffect = j.value("armorEffect", 0);
        m_profile.eliteArmor = j.value("eliteArmor", 0);

        m_profile.primaryColor = j.value("primaryColor", 0);
        m_profile.secondaryColor = j.value("secondaryColor", 12);
        m_profile.detailColor = j.value("detailColor", 4);

        std::string tag = j.value("serviceTag", "B312");
        strncpy_s(m_profile.serviceTag, tag.c_str(), sizeof(m_profile.serviceTag) - 1);
        m_profile.voice = j.value("voice", 0);

        m_isDirty = false;
        return true;
    } catch (...) {
        return false;
    }
}

bool ArmorCustomizer::SaveProfile(const std::filesystem::path& path) {
    try {
        nlohmann::json j;
        j["species"] = m_profile.species;
        j["gender"] = m_profile.gender;
        j["helmet"] = m_profile.helmet;
        j["leftShoulder"] = m_profile.leftShoulder;
        j["rightShoulder"] = m_profile.rightShoulder;
        j["chest"] = m_profile.chest;
        j["wrist"] = m_profile.wrist;
        j["utility"] = m_profile.utility;
        j["visorColor"] = m_profile.visorColor;
        j["kneeGuards"] = m_profile.kneeGuards;
        j["armorEffect"] = m_profile.armorEffect;
        j["eliteArmor"] = m_profile.eliteArmor;

        j["primaryColor"] = m_profile.primaryColor;
        j["secondaryColor"] = m_profile.secondaryColor;
        j["detailColor"] = m_profile.detailColor;

        j["serviceTag"] = std::string(m_profile.serviceTag);
        j["voice"] = m_profile.voice;

        std::ofstream file(path);
        file << j.dump(4);
        m_isDirty = false;
        return true;
    } catch (...) {
        return false;
    }
}

void ArmorCustomizer::Render() {
    ImGui::TextColored(ImVec4(0.0f, 0.85f, 1.0f, 1.0f), "THE ARMORY // PLAYER CUSTOMIZATION");
    ImGui::Separator();
    ImGui::Spacing();

    // Top control bar
    ImGui::Text("Species:");
    ImGui::SameLine();
    if (ImGui::RadioButton("Spartan", m_profile.species == 0)) {
        m_profile.species = 0;
        m_isDirty = true;
    }
    ImGui::SameLine();
    if (ImGui::RadioButton("Elite", m_profile.species == 1)) {
        m_profile.species = 1;
        m_isDirty = true;
    }

    ImGui::SameLine(ImGui::GetContentRegionAvail().x - 140.0f);
    if (ImGui::Button("SAVE PROFILE", ImVec2(130, 28))) {
        SaveProfile();
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // Two columns: Left = Armor Permutations, Right = Colors & Identity
    float totalWidth = ImGui::GetContentRegionAvail().x;
    ImGui::BeginChild("PermutationRegion", ImVec2(totalWidth * 0.52f, 0), true);
    if (m_profile.species == 0) {
        RenderSpartanCustomizer();
    } else {
        RenderEliteCustomizer();
    }
    ImGui::EndChild();

    ImGui::SameLine();

    ImGui::BeginChild("ColorsRegion", ImVec2(totalWidth * 0.46f, 0), true);
    ImGui::TextColored(ImVec4(0.0f, 0.85f, 1.0f, 1.0f), "SERVICE RECORD & EMBLEM");
    ImGui::Separator();
    ImGui::Spacing();

    ImGui::Text("Service Tag (4 Characters):");
    ImGui::SetNextItemWidth(100.0f);
    if (ImGui::InputText("##ServiceTag", m_profile.serviceTag, sizeof(m_profile.serviceTag), ImGuiInputTextFlags_CharsUppercase)) {
        m_isDirty = true;
    }

    ImGui::Spacing();
    ImGui::Text("Firefight Voice:");
    const char* voices[] = { "Noble Six (Default)", "Jorge-052", "Jun-A266", "Emile-A239", "Kat-B320", "Carter-A259", "Master Chief", "Cortana", "Sgt. Johnson", "Buck" };
    if (ImGui::Combo("##VoiceCombo", &m_profile.voice, voices, IM_ARRAYSIZE(voices))) {
        m_isDirty = true;
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    ImGui::TextColored(ImVec4(0.0f, 0.85f, 1.0f, 1.0f), "ARMOR COLORS");
    ImGui::Separator();
    ImGui::Spacing();

    RenderColorPalettePicker("Primary Color", m_profile.primaryColor);
    ImGui::Spacing();
    RenderColorPalettePicker("Secondary Color", m_profile.secondaryColor);
    ImGui::Spacing();
    RenderColorPalettePicker("Detail / Accent Color", m_profile.detailColor);

    ImGui::EndChild();
}

void ArmorCustomizer::RenderSpartanCustomizer() {
    ImGui::TextColored(ImVec4(0.0f, 0.85f, 1.0f, 1.0f), "SPARTAN-III PERMUTATIONS");
    ImGui::Separator();
    ImGui::Spacing();

    // Gender
    ImGui::Text("Body Type:");
    ImGui::SameLine();
    if (ImGui::RadioButton("Male", m_profile.gender == 0)) { m_profile.gender = 0; m_isDirty = true; }
    ImGui::SameLine();
    if (ImGui::RadioButton("Female", m_profile.gender == 1)) { m_profile.gender = 1; m_isDirty = true; }
    ImGui::Spacing();

    // Helmets
    const char* helmets[] = {
        "Default (Mark V [B])", "CQC", "ODST", "Hazop", "EOD", "Operator",
        "Grenadier", "Air Assault", "Scout", "EVA", "Commando", "Recon",
        "Military Police", "Mark V", "Mark VI", "GUNGNIR", "Security", "Haunted"
    };
    ImGui::Text("Helmet:");
    if (ImGui::Combo("##Helmet", &m_profile.helmet, helmets, IM_ARRAYSIZE(helmets))) m_isDirty = true;

    // Visor Colors
    const char* visors[] = { "Default (Gold)", "Silver", "Blue", "Black", "Gold", "Cyan", "Crimson" };
    ImGui::Text("Visor Color:");
    if (ImGui::Combo("##Visor", &m_profile.visorColor, visors, IM_ARRAYSIZE(visors))) m_isDirty = true;

    // Chests
    const char* chests[] = {
        "Default", "Collar/Breacher", "Collar/Up-Armor", "Tactical/LRP",
        "Tactical/Recon", "Tactical/Patrol", "Assault/Sapper", "HP/Halo"
    };
    ImGui::Text("Chest Armor:");
    if (ImGui::Combo("##Chest", &m_profile.chest, chests, IM_ARRAYSIZE(chests))) m_isDirty = true;

    // Shoulders
    const char* shoulders[] = {
        "Default", "CQC", "ODST", "Hazop", "EOD", "Operator",
        "Grenadier", "Air Assault", "Scout", "EVA", "Commando", "Security"
    };
    ImGui::Text("Left Shoulder:");
    if (ImGui::Combo("##LeftShoulder", &m_profile.leftShoulder, shoulders, IM_ARRAYSIZE(shoulders))) m_isDirty = true;

    ImGui::Text("Right Shoulder:");
    if (ImGui::Combo("##RightShoulder", &m_profile.rightShoulder, shoulders, IM_ARRAYSIZE(shoulders))) m_isDirty = true;

    // Wrist & Utility
    const char* wrists[] = { "Default", "UA/Bracer", "Tactical/Tacpad", "Tactical/UGPS" };
    ImGui::Text("Wrist Attachment:");
    if (ImGui::Combo("##Wrist", &m_profile.wrist, wrists, IM_ARRAYSIZE(wrists))) m_isDirty = true;

    const char* utilities[] = { "Default", "Tactical/Hard Case", "Tactical/Trauma Kit", "Chobham" };
    ImGui::Text("Utility Attachment:");
    if (ImGui::Combo("##Utility", &m_profile.utility, utilities, IM_ARRAYSIZE(utilities))) m_isDirty = true;

    // Knees & Effects
    const char* knees[] = { "Default", "FJ/PARA", "Gungnir", "Grenadier" };
    ImGui::Text("Knee Guards:");
    if (ImGui::Combo("##Knees", &m_profile.kneeGuards, knees, IM_ARRAYSIZE(knees))) m_isDirty = true;

    const char* effects[] = { "None", "Inclement Weather", "Pestilence", "Heart Attack", "Birthday Party", "Flaming Helmet" };
    ImGui::Text("Armor Effect:");
    if (ImGui::Combo("##ArmorEffect", &m_profile.armorEffect, effects, IM_ARRAYSIZE(effects))) m_isDirty = true;
}

void ArmorCustomizer::RenderEliteCustomizer() {
    ImGui::TextColored(ImVec4(0.0f, 0.85f, 1.0f, 1.0f), "SANGHEILI HARNESS PERMUTATIONS");
    ImGui::Separator();
    ImGui::Spacing();

    const char* eliteHarnesses[] = {
        "Minor", "Major", "Ultra", "Zealot", "Ranger", "General", "SpecOps", "Field Marshal"
    };
    ImGui::Text("Harness Rank / Armor:");
    if (ImGui::Combo("##EliteArmor", &m_profile.eliteArmor, eliteHarnesses, IM_ARRAYSIZE(eliteHarnesses))) {
        m_isDirty = true;
    }
}

void ArmorCustomizer::RenderColorPalettePicker(const char* label, int& selectedColorIndex) {
    ImGui::Text("%s: %s", label, REACH_COLOR_NAMES[selectedColorIndex]);

    float buttonSize = 22.0f;
    for (int i = 0; i < 30; ++i) {
        ImGui::PushID(label);
        ImGui::PushID(i);

        ImVec4 col = REACH_COLOR_PALETTE[i];
        if (i == selectedColorIndex) {
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
            ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 2.0f);
        }

        if (ImGui::ColorButton("##ColBtn", col, ImGuiColorEditFlags_NoTooltip, ImVec2(buttonSize, buttonSize))) {
            selectedColorIndex = i;
            m_isDirty = true;
        }

        if (i == selectedColorIndex) {
            ImGui::PopStyleVar();
            ImGui::PopStyleColor();
        }

        if ((i + 1) % 10 != 0 && i < 29) {
            ImGui::SameLine();
        }

        ImGui::PopID();
        ImGui::PopID();
    }
}

} // namespace noble
