#pragma once

#include <cstdint>
#include <string>

namespace noble {

// A shareable lobby code: the master server's session id (16 hex chars = 8 bytes) packed into a short,
// typo-tolerant Crockford-Base32 code. The code intentionally contains no IP address or token - the joiner resolves
// the endpoint through the existing master-server lobby API, so the same code works for public AND unlisted (private)
// lobbies and is safe to post in Discord.
//
// Format: "NB1-XXXX-XXXX-XXXX-XXXX" (10 bytes: version/private flag, 8 session bytes, CRC-8).
struct InviteCodeInfo {
    std::string sessionId;            // 16 lowercase hex chars (as returned by the master server)
    bool isPrivate = false;           // informational only; joining is identical for public/private sessions
};

// Encodes an 8-byte session id. Returns an empty string if the id is not 16 hex chars.
std::string EncodeInviteCode(const InviteCodeInfo& info);

// Decodes a code pasted by the player. Accepts the "NB1-" prefix or a bare 16-char code, any case, spaces/dashes, and
// the usual Crockford confusions (O->0, I/L->1). Returns false and fills `error` for bad input.
bool DecodeInviteCode(const std::string& text, InviteCodeInfo* out, std::string* error);

// Headless round-trip/negative test (also used by the "-invite-code-test" test hook). Returns the number of failures.
int InviteCodeSelfTest();

// One-line description for logs ("public"/"private").
const char* InviteCodePrivacyName(const InviteCodeInfo& info);

}  // namespace noble
