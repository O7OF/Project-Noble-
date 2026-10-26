#pragma once

#include "common/Types.hpp"
#include <optional>
#include <vector>

namespace noble {

class SteamLocator {
public:
    static std::optional<ReachInstallInfo> LocateAndVerifyReach();

private:
    static std::optional<fs::path> GetSteamRootFromRegistry();
    static std::vector<fs::path> GetAllSteamLibraries(const fs::path& steamRoot);
    static bool VerifyReachFiles(ReachInstallInfo& info);
};

} // namespace noble
