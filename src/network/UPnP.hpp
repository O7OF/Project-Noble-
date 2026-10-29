#pragma once

#include <string>
#include <cstdint>
#include <atomic>
#include <mutex>

namespace noble {

class UPnP {
public:
    static UPnP& Get() {
        static UPnP instance;
        return instance;
    }

    bool Initialize();
    void Shutdown();

    // Discovers local router via SSDP and forwards the requested UDP/TCP port
    bool ForwardPort(uint16_t port, const std::string& protocol = "UDP", const std::string& description = "Halo Reach CE");
    bool ReleasePort(uint16_t port, const std::string& protocol = "UDP");

    bool IsActive() const { return m_isActive; }
    const std::string& GetExternalIP() const;
    const std::string& GetInternalIP() const { return m_internalIp; }
    const std::string& GetStatus() const { return m_statusMessage; }

    std::string QueryPublicIPWithIpify();
    bool IsPublicIP(const std::string& ip) const;

private:
    UPnP();
    ~UPnP();

    bool SendSSDPDiscovery(std::string& outLocation);
    bool ParseDeviceDescription(const std::string& locationUrl);
    bool SendSOAPAction(const std::string& action, const std::string& body, std::string& outResponse);
    std::string GetLocalAdapterIP();

    std::atomic<bool> m_isInitialized{ false };
    std::atomic<bool> m_isActive{ false };
    uint16_t m_forwardedPort{ 0 };
    std::string m_protocol{ "UDP" };
    std::string m_controlUrl;
    std::string m_serviceType;
    std::string m_externalIp{ "Unknown" };
    std::string m_internalIp{ "127.0.0.1" };
    std::string m_statusMessage{ "UPnP Idle / Uninitialized" };
    std::mutex m_mutex;
};

} // namespace noble
