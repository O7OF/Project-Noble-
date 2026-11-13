#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace noble::mvar {

// Minimal SHA-1, used for the map variant chunk's content hash. The hash is not a plain digest of the chunk: it is
// SHA1(GEN3_SALT || packed_length (big-endian) || packed_data), the same construction Bungie's security functions use
// for gen-3 saved films and variants.
std::array<uint8_t, 20> Sha1(const uint8_t* data, size_t size);
std::array<uint8_t, 20> Sha1(const std::vector<uint8_t>& data);

} // namespace noble::mvar
