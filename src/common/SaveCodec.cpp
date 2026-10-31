#include "SaveCodec.hpp"
#include "common/Logger.hpp"

#include <windows.h>
#include <wincrypt.h>
#include <fstream>
#include <vector>

#pragma comment(lib, "crypt32.lib")

namespace noble {
namespace {
constexpr char kMagic[4] = { 'N', 'B', 'S', '1' };
} // namespace

bool SaveCodec::WriteJson(const std::filesystem::path& path, const nlohmann::json& value) {
    const std::string plain = value.dump(2);
    DATA_BLOB in{ static_cast<DWORD>(plain.size()), reinterpret_cast<BYTE*>(const_cast<char*>(plain.data())) };
    DATA_BLOB out{};
    if (!CryptProtectData(&in, L"Noble progression", nullptr, nullptr, nullptr, CRYPTPROTECT_UI_FORBIDDEN, &out)) {
        NOBLE_WARN("SaveCodec: CryptProtectData failed ({})", GetLastError());
        return false;
    }
    std::error_code ec;
    std::filesystem::create_directories(path.parent_path(), ec);
    std::ofstream file(path, std::ios::binary | std::ios::trunc);
    file.write(kMagic, sizeof(kMagic));
    file.write(reinterpret_cast<const char*>(out.pbData), out.cbData);
    const bool ok = file.good();
    LocalFree(out.pbData);
    return ok;
}

bool SaveCodec::IsEncryptedFile(const std::filesystem::path& path) {
    std::ifstream file(path, std::ios::binary);
    char magic[4]{};
    return file.read(magic, 4).good() && memcmp(magic, kMagic, 4) == 0;
}

bool SaveCodec::ReadJson(const std::filesystem::path& path, nlohmann::json& out) {
    std::ifstream file(path, std::ios::binary);
    if (!file) return false;
    char magic[4]{};
    file.read(magic, 4);
    if (!file || memcmp(magic, kMagic, 4) != 0) {
        file.clear();
        file.seekg(0);
        try { file >> out; return true; } catch (...) { return false; }
    }
    std::vector<unsigned char> blob((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    if (blob.empty()) return false;
    DATA_BLOB in{ static_cast<DWORD>(blob.size()), blob.data() };
    DATA_BLOB plain{};
    if (!CryptUnprotectData(&in, nullptr, nullptr, nullptr, nullptr, CRYPTPROTECT_UI_FORBIDDEN, &plain)) {
        NOBLE_WARN("SaveCodec: CryptUnprotectData failed ({})", GetLastError());
        return false;
    }
    bool ok = false;
    try {
        out = nlohmann::json::parse(std::string(reinterpret_cast<char*>(plain.pbData), plain.cbData));
        ok = true;
    } catch (...) {}
    LocalFree(plain.pbData);
    return ok;
}
} // namespace noble
