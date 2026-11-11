#pragma once

#include <filesystem>
#include <utility>

namespace noble {

// The .mvar this session was launched with. The engine parses it into its own structures and does not keep the
// container bytes, so this path is what a save falls back to when the engine writes nothing.
inline std::filesystem::path& SessionVariantStorage() {
    static std::filesystem::path path;
    return path;
}

inline void SetSessionMapVariantPath(std::filesystem::path path) { SessionVariantStorage() = std::move(path); }
inline const std::filesystem::path& SessionMapVariantPath() { return SessionVariantStorage(); }

} // namespace noble
