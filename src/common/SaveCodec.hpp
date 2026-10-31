#pragma once

#include <filesystem>
#include <nlohmann/json.hpp>
#include <string>

namespace noble {

// Progression files are encrypted with Windows DPAPI (current user + machine).
// The bytes are no longer readable/editable in Notepad; only this Windows account
// can decrypt them. This is local obfuscation + tamper deterrence, not server-grade
// anti-cheat: a determined user can still inspect process memory.
namespace SaveCodec {
bool WriteJson(const std::filesystem::path& path, const nlohmann::json& value);
bool ReadJson(const std::filesystem::path& path, nlohmann::json& out);
bool IsEncryptedFile(const std::filesystem::path& path);
} // namespace SaveCodec
} // namespace noble
