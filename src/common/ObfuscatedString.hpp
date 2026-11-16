#pragma once
// ---------------------------------------------------------------------------------------------
// Compile-time string obfuscation
//
// Plain literals end up verbatim in .rdata, where `strings`, Ghidra, IDA and x64dbg show them in
// seconds - module names, server URLs, registry paths and window classes are the first thing a
// reverse engineer reads. NOBLE_OBF()/NOBLE_OBFW() move a literal into a compile-time encrypted
// blob and decrypt it on the stack where it is actually used, so the binary only ever contains
// ciphertext.
//
// Usage:
//     GetModuleHandleW(NOBLE_OBFW(L"haloreach.dll").c_str())
//     http.Post(NOBLE_OBF("https://projectnoblebackend.onrender.com"), ...)
//
// Rules:
//   * The result is a temporary std::string/std::wstring. Copy it if it must outlive the full
//     expression, and never store a pointer to the temporary.
//   * The seed mixes the source line and a per-translation-unit counter, so every occurrence is
//     encrypted differently and one recovered key does not unlock the others.
//   * This is obfuscation, not encryption: it removes plaintext and string cross-references and
//     raises the cost of static analysis, but it cannot stop an analyst who debugs the decryption.
//     Never put real secrets in literals.
//   * Leave strings that must stay readable for support (log messages) alone.
// ---------------------------------------------------------------------------------------------

#include <cstddef>
#include <cstdint>
#include <string>

// Development builds (and the dedicated server) keep plain literals: they need readable strings while
// debugging, and there is no reason to pay the cipher on every build. Only the shipped client encrypts.
#if defined(NOBLE_DEV_BUILD) || defined(NOBLE_DEDICATED)

#define NOBLE_OBF(literal) (std::string(literal))
#define NOBLE_OBFW(literal) (std::wstring(literal))

#else

namespace noble::obf {

// Cheap avalanche so the ciphertext has no recognisable structure.
constexpr std::uint8_t KeyByte(std::size_t index, std::uint32_t seed) noexcept {
    std::uint32_t x = seed ^ (static_cast<std::uint32_t>(index) * 0x9E3779B9u) ^ 0x85EBCA6Bu;
    x ^= x >> 15;
    x *= 0xC2B2AE35u;
    x ^= x >> 13;
    x *= 0x27D4EB2Fu;
    x ^= x >> 16;
    return static_cast<std::uint8_t>(x);
}

template <typename CharT, std::size_t N, std::uint32_t Seed>
struct Blob {
    std::uint8_t bytes[N];

    constexpr explicit Blob(const CharT (&text)[N]) noexcept : bytes{} {
        for (std::size_t i = 0; i < N; ++i) {
            bytes[i] = static_cast<std::uint8_t>(static_cast<std::uint8_t>(text[i]) ^ KeyByte(i, Seed));
        }
    }

    std::basic_string<CharT> str() const {
        std::basic_string<CharT> out(N - 1, CharT(0));
        for (std::size_t i = 0; i + 1 < N; ++i) {
            out[i] = static_cast<CharT>(bytes[i] ^ KeyByte(i, Seed));
        }
        return out;
    }
};

} // namespace noble::obf

#define NOBLE_OBF_DETAIL_SEED (static_cast<std::uint32_t>(__LINE__) * 7919u + static_cast<std::uint32_t>(__COUNTER__) * 104729u)

#define NOBLE_OBF(literal)                                                                          \
    ([]() -> std::string {                                                                          \
        constexpr ::noble::obf::Blob<char, sizeof(literal), NOBLE_OBF_DETAIL_SEED> blob{literal};    \
        return blob.str();                                                                          \
    }())

#define NOBLE_OBFW(literal)                                                                         \
    ([]() -> std::wstring {                                                                         \
        constexpr ::noble::obf::Blob<wchar_t, (sizeof(literal) / sizeof(wchar_t)),                  \
                                     NOBLE_OBF_DETAIL_SEED>                                         \
            blob{literal};                                                                          \
        return blob.str();                                                                          \
    }())

#endif // NOBLE_DEV_BUILD || NOBLE_DEDICATED
