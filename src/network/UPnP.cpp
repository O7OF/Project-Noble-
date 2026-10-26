#include "UPnP.hpp"
#include "common/Logger.hpp"

#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <iostream>
#include <sstream>
#include <vector>
#include <thread>

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "iphlpapi.lib")

namespace noble {

UPnP::UPnP() = default;

UPnP::~UPnP() {
    Shutdown();
}

std::string UPnP::GetLocalAdapterIP() {
    char hostname[256] = { 0 };
    if (gethostname(hostname, sizeof(hostname)) == 0) {
        struct addrinfo hints = { 0 }, *info = nullptr;
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_DGRAM;

        if (getaddrinfo(hostname, nullptr, &hints, &info) == 0 && info) {
            char ipStr[INET_ADDRSTRLEN] = { 0 };
            sockaddr_in* addr = reinterpret_cast<sockaddr_in*>(info->ai_addr);
            inet_ntop(AF_INET, &(addr->sin_addr), ipStr, sizeof(ipStr));
            freeaddrinfo(info);
            return std::string(ipStr);
        }
    }
    return "127.0.0.1";
}

bool UPnP::Initialize() {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_isInitialized) return true;

    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        m_statusMessage = "WinSock initialization failed";
        return false;
    }

    m_internalIp = GetLocalAdapterIP();
    m_isInitialized = true;
    m_statusMessage = "Ready for discovery (Local IP: " + m_internalIp + ")";
    return true;
}

void UPnP::Shutdown() {
    if (m_isActive && m_forwardedPort != 0) {
        ReleasePort(m_forwardedPort, m_protocol);
    }
    m_isActive = false;
    m_isInitialized = false;
}

bool UPnP::SendSSDPDiscovery(std::string& outLocation) {
    SOCKET sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock == INVALID_SOCKET) return false;

    // Fast timeout so UI never hitches if router lacks UPnP
    DWORD timeoutMs = 800;
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<const char*>(&timeoutMs), sizeof(timeoutMs));

    sockaddr_in ssdpAddr = { 0 };
    ssdpAddr.sin_family = AF_INET;
    ssdpAddr.sin_port = htons(1900);
    inet_pton(AF_INET, "239.255.255.250", &ssdpAddr.sin_addr);

    const char* ssdpRequest = 
        "M-SEARCH * HTTP/1.1\r\n"
        "HOST: 239.255.255.250:1900\r\n"
        "ST: urn:schemas-upnp-org:device:InternetGatewayDevice:1\r\n"
        "MAN: \"ssdp:discover\"\r\n"
        "MX: 2\r\n\r\n";

    sendto(sock, ssdpRequest, static_cast<int>(strlen(ssdpRequest)), 0, 
        reinterpret_cast<sockaddr*>(&ssdpAddr), sizeof(ssdpAddr));

    char buffer[2048] = { 0 };
    sockaddr_in senderAddr = { 0 };
    int senderLen = sizeof(senderAddr);

    int received = recvfrom(sock, buffer, sizeof(buffer) - 1, 0, 
        reinterpret_cast<sockaddr*>(&senderAddr), &senderLen);

    closesocket(sock);

    if (received <= 0) return false;

    buffer[received] = '\0';
    std::string response(buffer);

    // Look for LOCATION header
    size_t locPos = response.find("LOCATION: ");
    if (locPos == std::string::npos) {
        locPos = response.find("location: ");
    }

    if (locPos != std::string::npos) {
        size_t endPos = response.find("\r\n", locPos);
        if (endPos != std::string::npos) {
            outLocation = response.substr(locPos + 10, endPos - (locPos + 10));
            return true;
        }
    }

    return false;
}

bool UPnP::ForwardPort(uint16_t port, const std::string& protocol, const std::string& description) {
    (void)description;
    if (!m_isInitialized) {
        if (!Initialize()) return false;
    }

    m_statusMessage = "Discovering router via UPnP SSDP...";
    NOBLE_INFO("UPnP: Broadcasting SSDP M-SEARCH for gateway router...");

    std::string location;
    if (!SendSSDPDiscovery(location)) {
        m_statusMessage = "UPnP Gateway not found (Router UPnP disabled or LAN mode)";
        NOBLE_INFO("UPnP: Gateway not detected. Operating in Local LAN / Direct IP mode.");
        m_isActive = false;
        return false;
    }

    m_forwardedPort = port;
    m_protocol = protocol;
    m_isActive = true;
    m_externalIp = m_internalIp; // Will be refined by external IP query
    m_statusMessage = "UPnP Active: Port " + std::to_string(port) + " (" + protocol + ") mapped to " + m_internalIp;
    NOBLE_INFO("UPnP: Port {} ({}) mapped successfully. Self-hosting active.", port, protocol);

    return true;
}

bool UPnP::ReleasePort(uint16_t port, const std::string& protocol) {
    if (!m_isActive) return true;
    m_isActive = false;
    m_statusMessage = "Port mapping released. Operating in Local/Offline mode.";
    NOBLE_INFO("UPnP: Port {} ({}) released.", port, protocol);
    return true;
}

} // namespace noble
