#include "mvar/BitReader.hpp"

#include <cstring>

namespace noble::mvar {

uint64_t BitReader::ReadBits(int count) {
    if (count <= 0) return 0;
    if (count > 64) count = 64;
    uint64_t value = 0;
    for (int i = 0; i < count; ++i) {
        const size_t byteIndex = m_bitPos >> 3;
        const int bitInByte = 7 - static_cast<int>(m_bitPos & 7);
        uint32_t bit = 0;
        if (byteIndex < m_size) bit = (m_data[byteIndex] >> bitInByte) & 1u;
        else m_overrun = true;
        value = (value << 1) | bit;
        ++m_bitPos;
    }
    return value;
}

void BitReader::Align() {
    const size_t rem = m_bitPos & 7;
    if (rem) m_bitPos += 8 - rem;
}

uint64_t BitReader::AlignedInt(int bytes) {
    const size_t start = m_bitPos >> 3;   // integer fields sit on byte boundaries
    uint64_t value = 0;
    for (int i = 0; i < bytes; ++i) {
        const size_t at = start + static_cast<size_t>(i);
        const uint64_t byte = at < m_size ? m_data[at] : 0;
        if (at >= m_size) m_overrun = true;
        if (m_little) value |= byte << (8 * i);      // b0 is the least significant byte
        else value = (value << 8) | byte;            // b0 is the most significant byte
    }
    m_bitPos += static_cast<size_t>(bytes) * 8;
    return value;
}

uint8_t BitReader::ReadByte() { return static_cast<uint8_t>(AlignedInt(1)); }
uint16_t BitReader::ReadUInt16() { return static_cast<uint16_t>(AlignedInt(2)); }
uint32_t BitReader::ReadUInt32() { return static_cast<uint32_t>(AlignedInt(4)); }
uint64_t BitReader::ReadUInt64() { return AlignedInt(8); }

uint16_t BitReader::ReadUInt16Big() {
    const bool wasLittle = m_little;
    m_little = false;
    const uint16_t v = ReadUInt16();
    m_little = wasLittle;
    return v;
}

uint32_t BitReader::ReadUInt32Big() {
    const bool wasLittle = m_little;
    m_little = false;
    const uint32_t v = ReadUInt32();
    m_little = wasLittle;
    return v;
}

float BitReader::ReadFloatBig() {
    const uint32_t raw = ReadUInt32Big();
    float value = 0.0f;
    std::memcpy(&value, &raw, sizeof(value));
    return value;
}

std::string BitReader::ReadString(int count) {
    std::string out;
    out.reserve(static_cast<size_t>(count));
    for (int i = 0; i < count; ++i) out.push_back(static_cast<char>(ReadByte()));
    return out;
}

std::wstring BitReader::ReadWidecharString(int chars) {
    std::wstring out;
    for (int i = 0; i < chars; ++i) {
        const uint16_t ch = static_cast<uint16_t>(ReadByte() | (ReadByte() << 8));   // UTF-16 little-endian
        if (ch == 0) break;
        out.push_back(static_cast<wchar_t>(ch));
    }
    return out;
}

std::vector<uint8_t> BitReader::ReadBytes(int count) {
    std::vector<uint8_t> out(static_cast<size_t>(count));
    for (int i = 0; i < count; ++i) out[i] = ReadByte();
    return out;
}

} // namespace noble::mvar
