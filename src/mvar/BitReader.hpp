#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace noble::mvar {

// Bit-level reader for the engine's map variant chunk, mirroring ReachVariantTool's reference parser.
//
// Two kinds of read live side by side in this format:
//   * bit fields (ReadBits): packed in order, most significant bit first, any width;
//   * plain integers (ReadUInt16/32/64): byte reads on a byte boundary, little- or big-endian by field.
// Getting that distinction wrong shifts everything after it, which is how the map id first came out as 24055.
class BitReader {
public:
    BitReader(const uint8_t* data, size_t size) : m_data(data), m_size(size) {}

    void SetLittleEndian(bool on) { m_little = on; }
    bool IsLittleEndian() const { return m_little; }

    uint64_t ReadBits(int count);
    void Align();
    void Seek(size_t bitPosition) { m_bitPos = bitPosition; }
    size_t BitPosition() const { return m_bitPos; }
    bool Overran() const { return m_overrun; }

    uint8_t ReadByte();
    uint16_t ReadUInt16();
    uint32_t ReadUInt32();
    uint64_t ReadUInt64();
    uint16_t ReadUInt16Big();
    uint32_t ReadUInt32Big();
    float ReadFloatBig();
    // 32 bits read straight from the stream: the same bit order as every other packed field, so this is what a
    // big-endian float looks like inside a bit-packed structure.
    float ReadFloatBitsBig();
    std::string ReadString(int count);
    std::wstring ReadWidecharString(int chars);
    std::vector<uint8_t> ReadBytes(int count);

private:
    uint64_t AlignedInt(int bytes);
    const uint8_t* m_data = nullptr;
    size_t m_size = 0;
    size_t m_bitPos = 0;
    bool m_little = true;
    bool m_overrun = false;
};

} // namespace noble::mvar
