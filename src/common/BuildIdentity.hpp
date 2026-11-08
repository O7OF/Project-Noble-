#pragma once

#include <cstdint>
#include <string_view>

namespace noble {

// ── Closed-source build identity ─────────────────────────────────────────────
// Project Noble's private line. These values intentionally differ from the
// open-source 0.9.16.x builds so the two lines cannot list, join or relay each
// other, even if an old client is pointed at the new cluster by hand.
//
// Keep kBuildTag in sync with VERSION_STRING (src/common/Version.hpp) and with
// REQUIRED_BUILD_PREFIX / CLIENT_PROTOCOL in ServerTestDeploy/server.js.

constexpr int kClientProtocol = 2;                 // closed-source wire/API revision
#ifdef NOBLE_DEV_BUILD
constexpr std::string_view kBuildTag = "0.19.22-dev";
#else
constexpr std::string_view kBuildTag = "0.19.22";  // prefix of VERSION_STRING
#endif
constexpr std::string_view kBaseBuildTag = "0.19.22";

// Sent on every master-server request. The cluster refuses requests without a
// matching build/protocol pair (HTTP 403 client_outdated).
constexpr std::string_view kClientBuildHeader = "X-Noble-Build";
constexpr std::string_view kClientProtocolHeader = "X-Noble-Protocol";

// Reach session id shared by a host and its guests. Not "HALOREAC" any more:
// an open-source client that reaches a closed-source host must also fail the
// engine's own session check, not only the master-server gate.
constexpr uint64_t kSessionMagic = 0x504E43534C4F4231ULL;   // "PNCSLOB1"

// Master-server cluster (three Render regions; they federate with each other
// through PEER_REGISTRY_URL - see ServerTestDeploy/README.md).
constexpr std::string_view kMasterUsEast = "https://projectnobleeast.onrender.com";
constexpr std::string_view kMasterUsWest = "https://projectnoblewest.onrender.com";
constexpr std::string_view kMasterUk     = "https://projectnobleuk.onrender.com";
constexpr std::string_view kMasterDefault = kMasterUsEast;

inline std::string_view ExtractBaseVersion(std::string_view v) {
    while (!v.empty() && (v.front() == ' ' || v.front() == '\t')) v.remove_prefix(1);
    const size_t end = v.find_first_of("- (");
    if (end != std::string_view::npos) {
        return v.substr(0, end);
    }
    return v;
}

inline bool AreBuildsCompatible(std::string_view a, std::string_view b) {
    if (a.empty() || b.empty()) return true;
    return ExtractBaseVersion(a) == ExtractBaseVersion(b);
}

} // namespace noble
