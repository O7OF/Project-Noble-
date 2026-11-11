#pragma once

#include "mvar/MapVariant.hpp"

#include <array>
#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

namespace noble::mvar {

// Packs a decoded variant back into a BLF container.
//
// The rewrite is region-by-region: the header up to the first object slot is copied verbatim (Noble does not edit it
// here), the 651 object records are re-encoded from the model, the quota table is copied verbatim, and the result is
// zero-padded to a byte. The chunk's packed length and SHA-1 are then rewritten and the original chunk list is
// preserved around it, so an unedited variant round-trips byte for byte.
bool WriteMapVariant(const MapVariant& variant, std::vector<uint8_t>& out, bool& hashRecomputed, std::string& error);

// Writes the JSON a saved Noble map variant is: {"version","name","base_map_id","mode","data"} with the whole
// container base64-encoded in "data" - the form FrontEndMenu's UnpackJsonForgeMap reads back.
bool SaveMapVariantJson(const MapVariant& variant, const std::filesystem::path& jsonPath, const std::string& name,
                        std::string& error);

// The canonical chunk hash: SHA1(GEN3_SALT || packed_length (big-endian) || packed data).
std::array<uint8_t, 20> ComputeChunkHash(const std::vector<uint8_t>& packedData, uint32_t packedLength);

} // namespace noble::mvar
