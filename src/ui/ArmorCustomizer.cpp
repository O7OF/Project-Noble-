#include "ArmorCustomizer.hpp"
#include "engine/ReachHost.hpp"
#include <imgui.h>
#include <nlohmann/json.hpp>
#include <fstream>
#include <array>
#include <algorithm>
#include <cctype>
#include <filesystem>
#include <string>
#include <utility>
#include <vector>
#include <windows.h>

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

static const std::array<ImVec4, 7> REACH_VISOR_PALETTE = {{
    ImVec4(0.95f, 0.62f, 0.12f, 1.0f),
    ImVec4(0.78f, 0.84f, 0.88f, 1.0f),
    ImVec4(0.20f, 0.50f, 0.95f, 1.0f),
    ImVec4(0.12f, 0.14f, 0.16f, 1.0f),
    ImVec4(1.00f, 0.72f, 0.18f, 1.0f),
    ImVec4(0.10f, 0.82f, 0.90f, 1.0f),
    ImVec4(0.88f, 0.16f, 0.18f, 1.0f)
}};

namespace {

std::filesystem::path ResolveModelPath(int species) {
    wchar_t executablePath[MAX_PATH]{};
    const DWORD pathLength = GetModuleFileNameW(nullptr, executablePath, MAX_PATH);
    const std::filesystem::path executableDirectory = pathLength > 0
        ? std::filesystem::path(executablePath).parent_path()
        : std::filesystem::current_path();
    const std::string speciesDirectory = species == 0 ? "spartans" : "elite";
    const std::string modelFile = species == 0 ? "spartans.obj" : "elite.obj";
    const std::string legacyFile = species == 0 ? "spartan.obj" : "elite.obj";
    std::vector<std::filesystem::path> roots{ executableDirectory / "data" / "armor" };
    std::error_code ec;
    const auto workingDirectory = std::filesystem::current_path(ec);
    if (!ec && workingDirectory != executableDirectory) roots.push_back(workingDirectory / "data" / "armor");

    for (const auto& root : roots) {
        for (const auto& candidate : { root / speciesDirectory / modelFile, root / legacyFile }) {
            ec.clear();
            if (std::filesystem::is_regular_file(candidate, ec)) return candidate;
        }
    }
    return roots.front() / speciesDirectory / modelFile;
}

std::string DefaultModelGroup(int species, int selection) {
    if (species == 0) return selection == 1 ? "-female" : "-default";
    static const char* const groups[] = {
        "-default", "-general", "-officer", "-space", "-spec_ops", "-ultra", "-zealot", "-story_elite"
    };
    const int index = std::clamp(selection, 0, static_cast<int>(std::size(groups)) - 1);
    return groups[index];
}

std::string FindModelGroup(const std::vector<std::string>& groups, const std::string& preferred) {
    auto match = std::find(groups.begin(), groups.end(), preferred);
    if (match != groups.end()) return *match;
    if (!preferred.empty() && preferred.front() == '-') {
        match = std::find(groups.begin(), groups.end(), preferred.substr(1));
    } else {
        match = std::find(groups.begin(), groups.end(), "-" + preferred);
    }
    return match != groups.end() ? *match : std::string{};
}

std::string ModelGroupLabel(const std::string& group) {
    std::string label = !group.empty() && group.front() == '-' ? group.substr(1) : group;
    const size_t exporterSuffix = label.find("__(");
    if (exporterSuffix != std::string::npos) label.resize(exporterSuffix);
    std::replace(label.begin(), label.end(), '_', ' ');
    return label;
}

std::filesystem::path ResolveAppearancePath(const std::filesystem::path& path) {
    if (path.is_absolute()) return path;
    const auto& dataDirectory = ReachHost::Get().GetDataDir();
    if (!dataDirectory.empty()) {
        const auto firstPart = path.begin();
        const auto relative = firstPart != path.end() && *firstPart == "data"
            ? path.lexically_relative("data") : path;
        return dataDirectory / relative;
    }
    return std::filesystem::current_path() / path;
}

} // namespace

ArmorCustomizer::ArmorCustomizer() {
    LoadProfile();
}

bool ArmorCustomizer::LoadProfile(const std::filesystem::path& path) {
    const auto profilePath = ResolveAppearancePath(path);
    if (!std::filesystem::exists(profilePath)) return false;

    try {
        std::ifstream file(profilePath);
        nlohmann::json j;
        file >> j;

        m_profile.species = j.value("species", 0);
        m_profile.gender = j.value("gender", 0);
        m_profile.visorColor = j.value("visorColor", 0);
        m_profile.eliteArmor = j.value("eliteArmor", 0);

        m_profile.primaryColor = j.value("primaryColor", 0);
        m_profile.secondaryColor = j.value("secondaryColor", 12);
        m_profile.detailColor = j.value("detailColor", 4);
        m_profile.helmetGroup = j.value("helmetGroup", std::string{});
        m_profile.chestGroup = j.value("chestGroup", std::string{});
        m_profile.leftShoulderGroup = j.value("leftShoulderGroup", std::string{});
        m_profile.rightShoulderGroup = j.value("rightShoulderGroup", std::string{});
        m_profile.wristGroup = j.value("wristGroup", std::string{});
        m_profile.kneeGroup = j.value("kneeGroup", std::string{});

        m_profile.species = std::clamp(m_profile.species, 0, 1);
        m_profile.gender = std::clamp(m_profile.gender, 0, 1);
        m_profile.eliteArmor = std::clamp(m_profile.eliteArmor, 0, 7);
        m_profile.primaryColor = std::clamp(m_profile.primaryColor, 0, 29);
        m_profile.secondaryColor = std::clamp(m_profile.secondaryColor, 0, 29);
        m_profile.detailColor = std::clamp(m_profile.detailColor, 0, 29);
        m_profile.visorColor = std::clamp(m_profile.visorColor, 0, static_cast<int>(REACH_VISOR_PALETTE.size()) - 1);

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
        const auto profilePath = ResolveAppearancePath(path);
        std::filesystem::create_directories(profilePath.parent_path());
        nlohmann::json j;
        j["species"] = m_profile.species;
        j["gender"] = m_profile.gender;
        j["visorColor"] = m_profile.visorColor;
        j["eliteArmor"] = m_profile.eliteArmor;

        j["primaryColor"] = m_profile.primaryColor;
        j["secondaryColor"] = m_profile.secondaryColor;
        j["detailColor"] = m_profile.detailColor;
        j["helmetGroup"] = m_profile.helmetGroup;
        j["chestGroup"] = m_profile.chestGroup;
        j["leftShoulderGroup"] = m_profile.leftShoulderGroup;
        j["rightShoulderGroup"] = m_profile.rightShoulderGroup;
        j["wristGroup"] = m_profile.wristGroup;
        j["kneeGroup"] = m_profile.kneeGroup;

        j["serviceTag"] = std::string(m_profile.serviceTag);
        j["voice"] = m_profile.voice;

        std::ofstream file(profilePath);
        if (!file) return false;
        file << j.dump(4);
        m_isDirty = false;
        return true;
    } catch (...) {
        return false;
    }
}

void ArmorCustomizer::Render(ID3D11Device* device, ID3D11DeviceContext* context) {
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

    // Three columns: Left = Armor Permutations, Center = 3D Character Preview, Right = Colors & Identity
    float totalWidth = ImGui::GetContentRegionAvail().x;
    float col1Width = totalWidth * 0.36f;
    float col2Width = totalWidth * 0.32f;
    float col3Width = totalWidth * 0.30f;

    ImGui::BeginChild("PermutationRegion", ImVec2(col1Width, 0), true);
    if (m_profile.species == 0) {
        RenderSpartanCustomizer();
    } else {
        RenderEliteCustomizer();
    }
    ImGui::EndChild();

    ImGui::SameLine();

    ImGui::BeginChild("PreviewRegion", ImVec2(col2Width, 0), true);
    RenderCharacterPreview(device, context);
    ImGui::EndChild();

    ImGui::SameLine();

    ImGui::BeginChild("ColorsRegion", ImVec2(col3Width, 0), true);
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

void ArmorCustomizer::RenderCharacterPreview(ID3D11Device* device, ID3D11DeviceContext* context) {
    ImGui::TextColored(ImVec4(0.0f, 0.88f, 1.0f, 1.0f), "ARMOR VISUALIZER");
    ImGui::Separator();
    ImGui::Spacing();

    const std::filesystem::path modelPath = ResolveModelPath(m_profile.species);
    const bool reload = ImGui::Button("RELOAD MCC MODEL");
    if (reload || m_loadedSpecies != m_profile.species) {
        m_loadedSpecies = m_profile.species;
        m_previewError.clear();
        m_previewGroups.clear();
        if (!m_preview.Initialize(device)) {
            m_previewError = "Could not initialize the Direct3D model preview.";
        } else if (!m_preview.LoadModel(modelPath, m_previewError)) {
            m_previewError += "\n" + modelPath.string();
        } else {
            for (const auto& mesh : m_preview.GetModel().meshes) {
                if (!mesh.groupName.empty() &&
                    std::find(m_previewGroups.begin(), m_previewGroups.end(), mesh.groupName) == m_previewGroups.end()) {
                    m_previewGroups.push_back(mesh.groupName);
                }
            }
            std::sort(m_previewGroups.begin(), m_previewGroups.end());
        }
    }

    if (m_preview.IsLoaded() && m_loadedSpecies == m_profile.species) {
        std::vector<ObjPreviewLayer> layers;
        auto addLayer = [&](const std::string& group, std::vector<std::string> included = {},
                            std::vector<std::string> excluded = {}) {
            if (group.empty() || std::find(m_previewGroups.begin(), m_previewGroups.end(), group) == m_previewGroups.end()) return;
            const auto existing = std::find_if(layers.begin(), layers.end(), [&](const ObjPreviewLayer& layer) {
                return layer.groupName == group;
            });
            if (existing == layers.end()) layers.push_back({group, std::move(included), std::move(excluded)});
        };

        if (m_profile.species == 0) {
            const std::string baseGroup = FindModelGroup(m_previewGroups, DefaultModelGroup(0, m_profile.gender));
            std::vector<std::string> excludedBaseMaterials;
            if (!m_profile.helmetGroup.empty()) {
                excludedBaseMaterials = {
                    "spartan_helmet_default", "spartan_helmet_interior", "spartan_helmet_interior_visor", "spartan_visor_default"
                };
            }
            addLayer(baseGroup, {}, std::move(excludedBaseMaterials));
            addLayer(m_profile.chestGroup);
            addLayer(m_profile.leftShoulderGroup);
            addLayer(m_profile.rightShoulderGroup);
            addLayer(m_profile.wristGroup);
            addLayer(m_profile.kneeGroup);
            addLayer(m_profile.helmetGroup);
        } else {
            const std::string baseGroup = FindModelGroup(m_previewGroups, "-default");
            if (m_profile.eliteArmor == 0) {
                addLayer(baseGroup);
            } else {
                addLayer(baseGroup, {
                    "body", "elite_base_lower", "elite_base_lower_metal", "elite_base_upper", "eyes", "head", "teeth"
                });
                addLayer(FindModelGroup(m_previewGroups, DefaultModelGroup(1, m_profile.eliteArmor)), {}, {"elite_base_upper"});
            }
        }

        ObjPreviewColors colors;
        const auto copyColor = [](std::array<float, 4>& destination, const ImVec4& source) {
            destination = {source.x, source.y, source.z, source.w};
        };
        copyColor(colors.primary, REACH_COLOR_PALETTE[static_cast<size_t>(m_profile.primaryColor)]);
        copyColor(colors.secondary, REACH_COLOR_PALETTE[static_cast<size_t>(m_profile.secondaryColor)]);
        copyColor(colors.detail, REACH_COLOR_PALETTE[static_cast<size_t>(m_profile.detailColor)]);
        copyColor(colors.visor, REACH_VISOR_PALETTE[static_cast<size_t>(m_profile.visorColor)]);

        const ImVec2 available = ImGui::GetContentRegionAvail();
        const float side = std::max(160.0f, std::min(available.x, 330.0f));
        const float time = static_cast<float>(ImGui::GetTime());
        const bool rendered = m_preview.Render(context, m_previewRotation + time * 0.2f, layers, colors);
        if (rendered) {
            ImGui::Image(reinterpret_cast<ImTextureID>(m_preview.GetPreviewTexture()), ImVec2(side, side));
            if (ImGui::IsItemHovered() && ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
                m_previewRotation += ImGui::GetIO().MouseDelta.x * 0.01f;
            }
        } else {
            ImGui::TextWrapped("No valid MCC body and armor mesh combination is available for this selection.");
        }
        ImGui::TextDisabled("Drag to rotate. The base body/undersuit renders first, then the selected MCC armor meshes.");
        ImGui::Spacing();
        ImGui::TextDisabled("%s", m_profile.species == 0 ? "Spartan base with selected armor overlays" : "Elite undersuit/body with the selected harness on top");
        ImGui::TextDisabled("In-game integration currently applies species only; Reach armor and color IDs still need verified profile mappings.");
    } else {
        ImGui::TextWrapped("%s", m_previewError.empty() ? "No model loaded." : m_previewError.c_str());
        ImGui::Spacing();
        ImGui::TextDisabled("Expected MCC-derived OBJ export:");
        ImGui::TextWrapped("%s", modelPath.string().c_str());
        ImGui::TextDisabled("Keep its MTL file and bitmaps directory beside it. Model exports stay local and are not shipped.");
    }
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

    RenderGroupSelector("Helmet model", m_profile.helmetGroup, "helmet");

    const char* visors[] = { "Default (Gold)", "Silver", "Blue", "Black", "Gold", "Cyan", "Crimson" };
    ImGui::Text("Visor Color:");
    if (ImGui::Combo("##Visor", &m_profile.visorColor, visors, IM_ARRAYSIZE(visors))) m_isDirty = true;

    RenderGroupSelector("Chest model", m_profile.chestGroup, "chest");
    RenderGroupSelector("Left shoulder model", m_profile.leftShoulderGroup, "shoulder", "left");
    RenderGroupSelector("Right shoulder model", m_profile.rightShoulderGroup, "shoulder", "right");
    RenderGroupSelector("Wrist model", m_profile.wristGroup, "wrist");
    RenderGroupSelector("Knee model", m_profile.kneeGroup, "knee");
}

void ArmorCustomizer::RenderEliteCustomizer() {
    ImGui::TextColored(ImVec4(0.0f, 0.85f, 1.0f, 1.0f), "SANGHEILI HARNESS PERMUTATIONS");
    ImGui::Separator();
    ImGui::Spacing();

    const char* eliteHarnesses[] = {
        "Minor / Stock", "General", "Officer", "Ranger / Space", "Spec Ops", "Ultra", "Zealot", "Story Elite"
    };
    ImGui::Text("Harness Rank / Armor:");
    if (ImGui::Combo("##EliteArmor", &m_profile.eliteArmor, eliteHarnesses, IM_ARRAYSIZE(eliteHarnesses))) {
        m_isDirty = true;
    }
}

void ArmorCustomizer::RenderGroupSelector(const char* label, std::string& selectedGroup,
                                         std::string_view category, std::string_view side) {
    std::vector<std::string> candidates;
    for (const auto& group : m_previewGroups) {
        std::string lowered = group;
        std::transform(lowered.begin(), lowered.end(), lowered.begin(), [](unsigned char ch) {
            return static_cast<char>(std::tolower(ch));
        });
        if (lowered.find(category) == std::string::npos) continue;
        if (category == "helmet" && (lowered.find("interior") != std::string::npos || lowered.find("visor") != std::string::npos)) continue;
        if (!side.empty()) {
            const bool hasSide = side == "left"
                ? (lowered.find("left") != std::string::npos || lowered.find("_l_") != std::string::npos)
                : (lowered.find("right") != std::string::npos || lowered.find("_r_") != std::string::npos);
            if (!hasSide) continue;
        }
        candidates.push_back(group);
    }

    const auto selected = std::find(candidates.begin(), candidates.end(), selectedGroup);
    const std::string selectedLabel = selectedGroup.empty() ? "Stock MCC piece"
        : selected == candidates.end() ? "Unavailable in this MCC export" : ModelGroupLabel(*selected);
    ImGui::Text("%s", label);
    ImGui::SetNextItemWidth(-1.0f);
    const std::string id = std::string("##") + label;
    if (ImGui::BeginCombo(id.c_str(), selectedLabel.c_str(), ImGuiComboFlags_HeightLarge)) {
        if (ImGui::Selectable("Stock MCC piece", selectedGroup.empty())) {
            selectedGroup.clear();
            m_isDirty = true;
        }
        for (const auto& group : candidates) {
            const std::string groupLabel = ModelGroupLabel(group) + "##" + group;
            if (ImGui::Selectable(groupLabel.c_str(), selectedGroup == group)) {
                selectedGroup = group;
                m_isDirty = true;
            }
        }
        ImGui::EndCombo();
    }
}

void ArmorCustomizer::RenderColorPalettePicker(const char* label, int& selectedColorIndex) {
    selectedColorIndex = std::clamp(selectedColorIndex, 0, static_cast<int>(REACH_COLOR_PALETTE.size()) - 1);
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
