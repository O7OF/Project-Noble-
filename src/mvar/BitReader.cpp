#include "mvar/BitReader.hpp"

#include <cstring>

namespace noble::mvar {

uint64_t BitReader::ReadBits(int count) {
    if (count <= 0) return 0;
    if (count > 64) count = 64;
    uint64_t value = 0;
    for (int i = 0; i < count; ++i) {
        const size_t byteIndex = m_bitPos >> 3;
        const int bitInByte = 7 - static_cast<int>(m_bitPos & 7);   // MSB first
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

uint32_t BitReader::ReadUInt32(bool littleEndian) { return static_cast<uint32_t>(ReadBits(32)); }
uint16_t BitReader::ReadUInt16(bool littleEndian) { return static_cast<uint16_t>(ReadBits(16)); }
uint64_t BitReader::ReadUInt64(bool littleEndian) { return ReadBits(64); }

float BitReader::ReadFloat() {
    const uint32_t raw = static_cast<uint32_t>(ReadBits(32));
    float value = 0.0f;
    std::memcpy(&value, &raw, sizeof(value));
    return value;
}

std::string BitReader::ReadString(int count) {
    std::string out;
    out.reserve(static_cast<size_t>(count));
    for (int i = 0; i < count; ++i) out.push_back(static_cast<char>(ReadBits(8) & 0xFF));
    return out;
}

std::wstring BitReader::ReadWidecharString(int chars) {
    std::wstring out;
    for (int i = 0; i < chars; ++i) {
        const uint16_t ch = static_cast<uint16_t>(ReadBits(16));
        if (ch == 0) break;
        out.push_back(static_cast<wchar_t>(ch));
    }
    return out;
}

std::vector<uint8_t> BitReader::ReadBytes(int count) {
    std::vector<uint8_t> out(static_cast<size_t>(count));
    for (int i = 0; i < count; ++i) out[i] = static_cast<uint8_t>(ReadBits(8));
    return out;
}

} // namespace noble::mvar
