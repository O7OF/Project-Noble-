#include "DeviceIdentity.hpp"
#include "common/Logger.hpp"

#include <windows.h>
#include <nlohmann/json.hpp>

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <cstdio>
#include <fstream>
#include <iomanip>
#include <sstream>

namespace noble {
namespace {

std::filesystem::path ExeDataDir() {
    wchar_t exe[MAX_PATH] = {};
    if (GetModuleFileNameW(nullptr, exe, MAX_PATH) == 0) return std::filesystem::path("data") / "userdata";
    return std::filesystem::path(exe).parent_path() / "data" / "userdata";
}

std::string MachineGuid() {
    HKEY key = nullptr;
    if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Cryptography", 0, KEY_READ | KEY_WOW64_64KEY, &key) != ERROR_SUCCESS) return {};
    wchar_t value[128] = {};
    DWORD size = sizeof(value);
    DWORD type = 0;
    const LONG rc = RegQueryValueExW(key, L"MachineGuid", nullptr, &type, reinterpret_cast<LPBYTE>(value), &size);
    RegCloseKey(key);
    if (rc != ERROR_SUCCESS || type != REG_SZ) return {};
    std::wstring wide(value);
    while (!wide.empty() && wide.back() == L'\0') wide.pop_back();
    const int n = WideCharToMultiByte(CP_UTF8, 0, wide.c_str(), static_cast<int>(wide.size()), nullptr, 0, nullptr, nullptr);
    std::string out(static_cast<size_t>(n), '\0');
    WideCharToMultiByte(CP_UTF8, 0, wide.c_str(), static_cast<int>(wide.size()), out.data(), n, nullptr, nullptr);
    return out;
}

std::string VolumeSerial() {
    wchar_t root[] = L"C:\\";
    DWORD serial = 0;
    if (!GetVolumeInformationW(root, nullptr, 0, &serial, nullptr, nullptr, nullptr, 0)) return {};
    char buf[16];
    std::snprintf(buf, sizeof(buf), "%08X", serial);
    return buf;
}

std::string MakeDeviceId() {
    const std::string seed = MachineGuid() + "|" + VolumeSerial() + "|noble-device-v1";
    // 64-bit FNV-1a, written as 16 hex chars. Stable across runs and machines.
    uint64_t hash = 1469598103934665603ULL;
    for (unsigned char c : seed) { hash ^= c; hash *= 1099511628211ULL; }
    std::ostringstream out;
    out << std::hex << std::setfill('0') << std::setw(16) << hash;
    return out.str();
}

std::string DefaultName(const std::string& id) {
    return "Spartan-" + id.substr(0, 4);
}

DeviceIdentity LoadOrCreate() {
    const auto path = ExeDataDir() / "identity.json";
    DeviceIdentity id;
    std::ifstream in(path);
    if (in.is_open()) {
        try {
            const auto j = nlohmann::json::parse(in);
            id.deviceId = j.value("deviceId", "");
            id.playerName = j.value("playerName", "");
            id.serviceTag = j.value("serviceTag", "");
            id.developer = j.value("developer", false);
        } catch (...) {}
    }
    if (id.deviceId.empty()) id.deviceId = MakeDeviceId();
    if (id.playerName.empty()) id.playerName = DefaultName(id.deviceId);
    if (id.serviceTag.empty()) id.serviceTag = id.deviceId.substr(0, 4);
    for (char& ch : id.serviceTag) ch = static_cast<char>(std::toupper(static_cast<unsigned char>(ch)));
    if (id.deviceId.empty()) id.deviceId = "0000000000000000";
    std::error_code ec;
    std::filesystem::create_directories(ExeDataDir(), ec);
    nlohmann::json j = {
        { "deviceId", id.deviceId }, { "playerName", id.playerName },
        { "serviceTag", id.serviceTag }, { "developer", id.developer }
    };
    std::ofstream out(path, std::ios::trunc);
    out << j.dump(2);
    return id;
}

DeviceIdentity& Mutable() {
    static DeviceIdentity id = LoadOrCreate();
    return id;
}
} // namespace

namespace DeviceIdentityStore {
const DeviceIdentity& Get() { return Mutable(); }

std::filesystem::path DataDir() { return ExeDataDir(); }

std::filesystem::path UserDir() {
    return ExeDataDir() / Get().deviceId;
}

void Save(const DeviceIdentity& identity) {
    std::error_code ec;
    std::filesystem::create_directories(ExeDataDir(), ec);
    nlohmann::json j = {
        { "deviceId", identity.deviceId }, { "playerName", identity.playerName },
        { "serviceTag", identity.serviceTag }, { "developer", identity.developer }
    };
    std::ofstream out(ExeDataDir() / "identity.json", std::ios::trunc);
    out << j.dump(2);
    Mutable() = identity;
}

void Reload() { Mutable() = LoadOrCreate(); }
} // namespace DeviceIdentityStore
} // namespace noble
