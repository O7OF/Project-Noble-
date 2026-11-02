#pragma once

#include <cstdint>
#include <string>

namespace noble::EmbeddedBlocks {

// Retrieves a prebuilt launch block or template by filename (e.g. "launch_block_1.bin", "lb_m_c_1.bin", "campaign_178_d1.bin", "profile_1.bin").
// Returns a pointer directly into the executable's mapped .rsrc section and its byte size.
const uint8_t* GetBlock(const std::string& filename, size_t* outSize = nullptr);

// Checks if an embedded block exists.
bool HasBlock(const std::string& filename);

} // namespace noble::EmbeddedBlocks
