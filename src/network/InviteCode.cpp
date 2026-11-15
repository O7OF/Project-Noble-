#include "network/InviteCode.hpp"

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <vector>

namespace noble {
namespace {

// Crockford Base32: no I, L, O, U so the code survives being retyped from a screenshot/video.
constexpr char kAlphabet[] = "0123456789ABCDEFGHJKMNPQRSTVWXYZ";
constexpr int kPayloadBytes = 10;  // version/flags + 8 session id bytes + CRC-8
constexpr int kCodeChars = 16;     // ceil(10 * 8 / 5)
constexpr const char* kPrefix = "NB1";
constexpr uint8_t kVersion = 1;

uint8_t Crc8(const uint8_t* data, size_t len) {
    uint8_t crc = 0;
    for (size_t i = 0; i < len; ++i) {
        crc ^= data[i];
        for (int b = 0; b < 8; ++b) crc = (crc & 0x80) ? static_cast<uint8_t>((crc << 1) ^ 0x07) : static_cast<uint8_t>(crc << 1);
    }
    return crc;
}

int HexValue(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}

// "37b843a50ae5d6e4" -> 8 bytes.
bool ParseSessionId(const std::string& id, uint8_t out[8]) {
    if (id.size() != 16) return false;
    for (int i = 0; i < 8; ++i) {
        const int hi = HexValue(id[static_cast<size_t>(i) * 2]);
        const int lo = HexValue(id[static_cast<size_t>(i) * 2 + 1]);
        if (hi < 0 || lo < 0) return false;
        out[i] = static_cast<uint8_t>((hi << 4) | lo);
    }
    return true;
}

std::string FormatSessionId(const uint8_t in[8]) {
    static constexpr char kHex[] = "0123456789abcdef";
    std::string id;
    id.reserve(16);
    for (int i = 0; i < 8; ++i) {
        id.push_back(kHex[in[i] >> 4]);
        id.push_back(kHex[in[i] & 0xF]);
    }
    return id;
}

// 5 bits at a time, MSB first.
std::string Base32Encode(const uint8_t* data, int len) {
    std::string out;
    out.reserve((len * 8 + 4) / 5);
    int bits = 0;
    uint32_t acc = 0;
    for (int i = 0; i < len; ++i) {
        acc = (acc << 8) | data[i];
        bits += 8;
        while (bits >= 5) {
            bits -= 5;
            out.push_back(kAlphabet[(acc >> bits) & 0x1F]);
        }
    }
    if (bits > 0) out.push_back(kAlphabet[(acc << (5 - bits)) & 0x1F]);
    return out;
}

bool Base32Decode(const std::string& text, uint8_t* out, int outLen) {
    int bits = 0;
    uint32_t acc = 0;
    int written = 0;
    for (char raw : text) {
        char c = raw;
        if (c == 'O' || c == 'o') c = '0';
        else if (c == 'I' || c == 'i' || c == 'L' || c == 'l') c = '1';
        const char* pos = std::strchr(kAlphabet, static_cast<char>(toupper(static_cast<unsigned char>(c))));
        if (!pos) return false;
        acc = (acc << 5) | static_cast<uint32_t>(pos - kAlphabet);
        bits += 5;
        if (bits >= 8) {
            bits -= 8;
            if (written >= outLen) return false;
            out[written++] = static_cast<uint8_t>((acc >> bits) & 0xFF);
        }
    }
    // Allow the encoder's final padding bits, but nothing that would mean a missing chunk of data.
    if (written != outLen) return false;
    return (acc & ((1u << bits) - 1)) == 0;
}

void FillCode(uint8_t payload[kPayloadBytes], const InviteCodeInfo& info) {
    std::memset(payload, 0, kPayloadBytes);
    payload[0] = static_cast<uint8_t>((kVersion << 4) | (info.isPrivate ? 1 : 0));
    ParseSessionId(info.sessionId, payload + 1);
    payload[9] = Crc8(payload, 9);
}

}  // namespace

std::string EncodeInviteCode(const InviteCodeInfo& info) {
    uint8_t session[8] = {};
    if (!ParseSessionId(info.sessionId, session)) return {};
    uint8_t payload[kPayloadBytes] = {};
    FillCode(payload, info);
    const std::string body = Base32Encode(payload, kPayloadBytes);
    // "NB1-" + 4 groups of 4 characters.
    std::string code = std::string(kPrefix) + "-";
    for (size_t i = 0; i < body.size(); ++i) {
        if (i > 0 && i % 4 == 0) code.push_back('-');
        code.push_back(body[i]);
    }
    return code;
}

bool DecodeInviteCode(const std::string& input, InviteCodeInfo* out, std::string* error) {
    const auto fail = [&](const char* msg) {
        if (error) *error = msg;
        return false;
    };
    std::string text;
    text.reserve(input.size());
    for (char c : input) {
        if (c == '-' || c == ' ' || c == '_') continue;
        text.push_back(static_cast<char>(toupper(static_cast<unsigned char>(c))));
    }
    if (text.size() == kCodeChars + 3 && text.compare(0, 3, kPrefix) == 0) text.erase(0, 3);
    if (text.size() != kCodeChars) return fail("Code is not complete (expected NB1-XXXX-XXXX-XXXX-XXXX).");
    uint8_t payload[kPayloadBytes] = {};
    if (!Base32Decode(text, payload, kPayloadBytes)) return fail("Code has characters or a checksum that do not match.");
    if (Crc8(payload, 9) != payload[9]) return fail("Code checksum failed - re-check it or ask for a new one.");
    const uint8_t version = static_cast<uint8_t>(payload[0] >> 4);
    if (version != kVersion) return fail("Code is from a different Noble build/format; ask the host to update.");
    if (out) {
        out->sessionId = FormatSessionId(payload + 1);
        out->isPrivate = (payload[0] & 1) != 0;
    }
    return true;
}

const char* InviteCodePrivacyName(const InviteCodeInfo& info) { return info.isPrivate ? "private" : "public"; }

int InviteCodeSelfTest() {
    int failures = 0;
    const auto check = [&](bool ok, const char* what) {
        if (!ok) {
            ++failures;
            std::printf("FAIL  InviteCode: %s\n", what);
        }
    };
    const char* ids[] = {"37b843a50ae5d6e4", "0000000000000000", "ffffffffffffffff", "09e2e5663ab22812"};
    for (const char* idText : ids) {
        InviteCodeInfo in;
        in.sessionId = idText;
        for (const bool priv : {false, true}) {
            in.isPrivate = priv;
            const std::string code = EncodeInviteCode(in);
            check(code.rfind("NB1-", 0) == 0, "code has NB1 prefix");
            check(code.size() == 23, "code length is 23");  // NB1- + 16 chars + 3 group dashes
            InviteCodeInfo out;
            std::string error;
            check(DecodeInviteCode(code, &out, &error), "round-trip decode");
            check(out.sessionId == idText, "round-trip session id");
            check(out.isPrivate == priv, "round-trip privacy flag");
            std::string lower = code;
            std::transform(lower.begin(), lower.end(), lower.begin(), [](unsigned char c) { return static_cast<char>(tolower(c)); });
            check(DecodeInviteCode(lower, &out, &error), "decode is case-insensitive");
            std::string bare;
            for (char c : code)
                if (c != '-') bare.push_back(c);
            check(DecodeInviteCode(bare, &out, &error), "decode accepts a code without dashes");
            check(!DecodeInviteCode(code.substr(0, code.size() - 1), &out, &error), "rejects truncated code");
            std::string bad = code;
            bad[bad.size() - 2] = bad[bad.size() - 2] == '0' ? '1' : '0';
            check(!DecodeInviteCode(bad, &out, &error), "rejects a changed character (checksum)");
        }
    }
    check(EncodeInviteCode(InviteCodeInfo{"not-a-session", false}).empty(), "rejects a non-hex session id");
    return failures;
}

}  // namespace noble
