#pragma once

#include "common/Types.hpp"

namespace noble {

class OwnershipVerifier {
public:
    // Strictly verifies that the user legitimately owns MCC and the Halo: Reach DLC on Steam.
    // If verification fails, displays a user-facing error dialog and returns false.
    static bool VerifySteamOwnership(ReachInstallInfo& info);
};

} // namespace noble
