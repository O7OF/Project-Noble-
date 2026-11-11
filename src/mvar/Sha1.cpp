#include "mvar/Sha1.hpp"

#include <cstring>

namespace noble::mvar {
namespace {

inline uint32_t RotateLeft(uint32_t value, int bits) { return (value << bits) | (value >> (32 - bits)); }

void ProcessBlock(const uint8_t* block, uint32_t state[5], uint64_t& bitCount) {
    uint32_t w[80];
    for (int i = 0; i < 16; ++i) {
        w[i] = (static_cast<uint32_t>(block[i * 4]) << 24) | (static_cast<uint32_t>(block[i * 4 + 1]) << 16) |
               (static_cast<uint32_t>(block[i * 4 + 2]) << 8) | static_cast<uint32_t>(block[i * 4 + 3]);
    }
    for (int i = 16; i < 80; ++i) w[i] = RotateLeft(w[i - 3] ^ w[i - 8] ^ w[i - 14] ^ w[i - 16], 1);

    uint32_t a = state[0], b = state[1], c = state[2], d = state[3], e = state[4];
    for (int i = 0; i < 80; ++i) {
        uint32_t f = 0, k = 0;
        if (i < 20) {
            f = (b & c) | (~b & d);
            k = 0x5A827999;
        } else if (i < 40) {
            f = b ^ c ^ d;
            k = 0x6ED9EBA1;
        } else if (i < 60) {
            f = (b & c) | (b & d) | (c & d);
            k = 0x8F1BBCDC;
        } else {
            f = b ^ c ^ d;
            k = 0xCA62C1D6;
        }
        const uint32_t temp = RotateLeft(a, 5) + f + e + k + w[i];
        e = d;
        d = c;
        c = RotateLeft(b, 30);
        b = a;
        a = temp;
    }
    state[0] += a;
    state[1] += b;
    state[2] += c;
    state[3] += d;
    state[4] += e;
    bitCount += 512;
}

} // namespace

std::array<uint8_t, 20> Sha1(const uint8_t* data, size_t size) {
    uint32_t state[5] = { 0x67452301, 0xEFCDAB89, 0x98BADCFE, 0x10325476, 0xC3D2E1F0 };
    uint64_t bitCount = 0;

    size_t offset = 0;
    while (offset + 64 <= size) {
        ProcessBlock(data + offset, state, bitCount);
        offset += 64;
    }

    uint8_t tail[128] = {};
    const size_t remainder = size - offset;
    if (remainder) std::memcpy(tail, data + offset, remainder);
    tail[remainder] = 0x80;
    const size_t tailBlocks = (remainder + 1 + 8 <= 56) ? 1 : 2;
    const uint64_t messageBits = static_cast<uint64_t>(size) * 8;
    for (int i = 0; i < 8; ++i) {
        tail[tailBlocks * 64 - 1 - i] = static_cast<uint8_t>((messageBits >> (8 * i)) & 0xFF);
    }
    for (size_t block = 0; block < tailBlocks; ++block) ProcessBlock(tail + block * 64, state, bitCount);

    std::array<uint8_t, 20> digest{};
    for (int i = 0; i < 5; ++i) {
        digest[i * 4] = static_cast<uint8_t>((state[i] >> 24) & 0xFF);
        digest[i * 4 + 1] = static_cast<uint8_t>((state[i] >> 16) & 0xFF);
        digest[i * 4 + 2] = static_cast<uint8_t>((state[i] >> 8) & 0xFF);
        digest[i * 4 + 3] = static_cast<uint8_t>(state[i] & 0xFF);
    }
    return digest;
}

std::array<uint8_t, 20> Sha1(const std::vector<uint8_t>& data) { return Sha1(data.data(), data.size()); }

} // namespace noble::mvar
