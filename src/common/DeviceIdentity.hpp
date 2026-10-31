#pragma once

#include <filesystem>
#include <string>

namespace noble {

// Stable, local identity. There is no account server: the device id is derived from the
// Windows MachineGuid and the system volume serial, and the display name/tag live in
// data\userdata\identity.json. Nothing is uploaded.
struct DeviceIdentity {
    std::string deviceId;      // 16 hex chars
    std::string playerName;
    std::string serviceTag;
    bool developer = false;
};

namespace DeviceIdentityStore {

const DeviceIdentity& Get();
std::filesystem::path DataDir();       // data\userdata
std::filesystem::path UserDir();       // data\userdata\<deviceId>
void Save(const DeviceIdentity& identity);
void Reload();

} // namespace DeviceIdentityStore
} // namespace noble
