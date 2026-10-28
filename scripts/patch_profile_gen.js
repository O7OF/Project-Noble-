const fs = require('fs');

// 1. Settings.cpp
let settings = fs.readFileSync('src/common/Settings.cpp', 'utf8');
const oldSettingsLoad = '    const auto path = FilePath();\r\n    std::ifstream in(path);\r\n    if (in) {';
const newSettingsLoad = '    const auto path = FilePath();\r\n    std::ifstream in(path);\r\n    const bool existed = in.is_open();\r\n    if (in) {';
if (settings.includes(oldSettingsLoad)) {
    settings = settings.replace(oldSettingsLoad, newSettingsLoad);
    const oldSettingsEnd = '    m_data = d;\r\n    m_loaded = true;\r\n}';
    const newSettingsEnd = '    m_data = d;\r\n    m_loaded = true;\r\n    if (!existed) {\r\n        Save();\r\n    }\r\n}';
    settings = settings.replace(oldSettingsEnd, newSettingsEnd);
    fs.writeFileSync('src/common/Settings.cpp', settings, 'utf8');
    console.log('Settings.cpp patched');
} else {
    console.log('Settings.cpp: target not found');
}

// 2. ArmorCustomizer.cpp
let ac = fs.readFileSync('src/ui/ArmorCustomizer.cpp', 'utf8');
const oldAc = '    if (!std::filesystem::exists(m_profilePath)) {\r\n        SyncLegacyFields();\r\n        m_previewSpecies = m_profile.species;\r\n        SyncToReachArmory();\r\n        return false;\r\n    }';
const newAc = '    if (!std::filesystem::exists(m_profilePath)) {\r\n        SyncLegacyFields();\r\n        m_previewSpecies = m_profile.species;\r\n        SyncToReachArmory();\r\n        SaveProfile(m_profilePath);\r\n        return true;\r\n    }';
if (ac.includes(oldAc)) {
    ac = ac.replace(oldAc, newAc);
    fs.writeFileSync('src/ui/ArmorCustomizer.cpp', ac, 'utf8');
    console.log('ArmorCustomizer.cpp patched');
} else {
    console.log('ArmorCustomizer.cpp: target not found');
}

// 3. Career.cpp
let career = fs.readFileSync('src/career/Career.cpp', 'utf8');
const oldCareer = '    std::ifstream in(UserDir() / "career.json");\r\n    if (!in) return;';
const newCareer = '    std::ifstream in(UserDir() / "career.json");\r\n    if (!in) { SaveLocked(); return; }';
if (career.includes(oldCareer)) {
    career = career.replace(oldCareer, newCareer);
    fs.writeFileSync('src/career/Career.cpp', career, 'utf8');
    console.log('Career.cpp patched');
} else {
    console.log('Career.cpp: target not found');
}

// 4. ReachHost.cpp
let rh = fs.readFileSync('src/engine/ReachHost.cpp', 'utf8');
const oldRh = '            NOBLE_INFO("ReachHost [Slot 34]: using built-in default key bindings, volumes and look settings");\r\n        }';
const newRh = '            NOBLE_INFO("ReachHost [Slot 34]: using built-in default key bindings, volumes and look settings");\r\n            std::error_code ec;\r\n            std::filesystem::path p1 = ReachHost::Get().GetDataDir() / "profile_1.bin";\r\n            if (!std::filesystem::exists(p1, ec)) {\r\n                std::ofstream out(p1, std::ios::binary | std::ios::trunc);\r\n                if (out) {\r\n                    out.write(reinterpret_cast<const char*>(s_hostProfileBuffer), 0xC00);\r\n                    NOBLE_INFO("ReachHost [Slot 34]: generated default profile_1.bin");\r\n                }\r\n            }\r\n        }';
if (rh.includes(oldRh)) {
    rh = rh.replace(oldRh, newRh);
    fs.writeFileSync('src/engine/ReachHost.cpp', rh, 'utf8');
    console.log('ReachHost.cpp patched');
} else {
    console.log('ReachHost.cpp: target not found');
}

