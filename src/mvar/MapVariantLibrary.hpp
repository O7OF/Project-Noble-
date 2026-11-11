#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace noble::mvar {

// The Noble map-variant library: one JSON per variant under data/map_variants, in the form FrontEndMenu reads.
//
// A save is a repack, not a copy: the container is parsed, its title is set to the library name (the maps menu reads
// the byte-aligned copy at +0xC0), the SHA-1 and packed length are recomputed by the writer, and the result is stored
// as {"version","name","base_map_id","mode","data"}. Saving under an existing name overwrites it, which is how an
// edited map is updated in place.

// A name that is safe as a Windows file name and as a JSON string.
std::string SanitizeVariantName(const std::string& name);

// The variant's display title, for prefilling the save box (empty when the file cannot be parsed).
std::string VariantTitle(const std::filesystem::path& sourceMvar);

// Saves `sourceMvar` into `directory` as "<name>.json", overwriting an existing file of the same name.
// Returns the written path; empty on failure with `detail` holding the reason (or "saved" on success).
// Saves a raw 0x7028-byte "mvar" chunk produced by the engine's serializer. The surrounding container (BLF header,
// chdr chunk, trailing chunks) comes from `templateMvar`; the template's map/thumbnail are kept and only the variant
// chunk and the +0xC0 title are replaced.
std::filesystem::path SaveVariantChunkToLibrary(const std::vector<uint8_t>& mvarChunk,
                                                const std::filesystem::path& templateMvar, const std::string& name,
                                                const std::filesystem::path& directory, std::string& detail);

std::filesystem::path SaveVariantToLibrary(const std::filesystem::path& sourceMvar, const std::string& name,
                                           const std::filesystem::path& directory, std::string& detail);

} // namespace noble::mvar
