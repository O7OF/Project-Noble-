#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace noble::mvar {

// Bit-level reader for the engine's map variant chunk. The format is a mixed-endian bit stream: fields are bit-packed
// in order, some are read little-endian, some big-endian (the reference implementation in ReachVariantTool reads bits
// MSB-first and byte-swaps by field). Every read advances the exact number of bits the field occupies.
class BitReader {
public:
    BitReader(const uint8_t* data, size_t size) : m_data(data), m_size(size) {}

    void SetBigEndianFloats(bool on) { m_bigFloats = on; }
    void SetLittleEndianBytes(bool on) { m_littleBytes = on; }

    // Reads p count bits, most significant first, from the current position.
    uint64_t ReadBits(int count);
    void Align();                                   // advance to the next byte boundary
    size_t BitPosition() const { return m_bitPos; }
    size_t BytePosition() const { return (m_bitPos + 7) / 8; }
    bool Ok() const { return m_ok; }
    bool Overran() const { return m_overrun; }

    uint32_t ReadUInt32(bool littleEndian);
    uint16_t ReadUInt16(bool littleEndian);
    uint64_t ReadUInt64(bool littleEndian);
    float ReadFloat();
    std::string ReadString(int count);
    std::wstring ReadWidecharString(int chars);
    std::vector<uint8_t> ReadBytes(int count);

private:
    const uint8_t* m_data = nullptr;
    size_t m_size = 0;
    size_t m_bitPos = 0;
    bool m_bigFloats = true;
    bool m_littleBytes = true;
    bool m_ok = true;
    bool m_overrun = false;
};

} // namespace noble::mvar
