#include "UPnP.hpp"
#include "common/Logger.hpp"

#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <winhttp.h>
#include <algorithm>
#include <cctype>
#include <cstring>
#include <iterator>
#include <vector>

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "winhttp.lib")

namespace noble {

namespace {

std::wstring Widen(const std::string& value) {
    if (value.empty()) return {};
    const int size = MultiByteToWideChar(CP_UTF8, 0, value.data(), static_cast<int>(value.size()), nullptr, 0);
    if (size <= 0) return {};
    std::wstring result(static_cast<size_t>(size), L'\0');
    MultiByteToWideChar(CP_UTF8, 0, value.data(), static_cast<int>(value.size()), result.data(), size);
    return result;
}

std::string ElementText(const std::string& xml, const std::string& name) {
    const std::string open = "<" + name;
    const std::string close = "</" + name + ">";
    size_t start = xml.find(open);
    if (start == std::string::npos) return {};
    const size_t boundary = start + open.size();
    if (boundary >= xml.size() || (xml[boundary] != '>' && !std::isspace(static_cast<unsigned char>(xml[boundary])))) {
        start = xml.find(open, boundary);
        if (start == std::string::npos) return {};
    }
    const size_t content = xml.find('>', start + open.size());
    if (content == std::string::npos) return {};
    const size_t end = xml.find(close, content + 1);
    if (end == std::string::npos) return {};
    std::string value = xml.substr(content + 1, end - content - 1);
    const size_t first = value.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return {};
    const size_t last = value.find_last_not_of(" \t\r\n");
    return value.substr(first, last - first + 1);
}

std::string EscapeXml(const std::string& value) {
    std::string escaped;
    escaped.reserve(value.size());
    for (char c : value) {
        switch (c) {
        case '&': escaped += "&amp;"; break;
        case '<': escaped += "&lt;"; break;
        case '>': escaped += "&gt;"; break;
        case '\"': escaped += "&quot;"; break;
        case '\'': escaped += "&apos;"; break;
        default: escaped.push_back(c); break;
        }
    }
    return escaped;
}

struct HttpResponse {
    bool ok = false;
    std::string body;
};

HttpResponse HttpRequest(const std::string& url, const wchar_t* method, const std::wstring& headers,
                         const std::string& body) {
    HttpResponse result;
    const std::wstring wideUrl = Widen(url);
    if (wideUrl.empty()) return result;

    URL_COMPONENTS parts{};
    parts.dwStructSize = sizeof(parts);
    wchar_t host[256] = {};
    wchar_t path[2048] = {};
    wchar_t extra[1024] = {};
    parts.lpszHostName = host;
    parts.dwHostNameLength = static_cast<DWORD>(std::size(host));
    parts.lpszUrlPath = path;
    parts.dwUrlPathLength = static_cast<DWORD>(std::size(path));
    parts.lpszExtraInfo = extra;
    parts.dwExtraInfoLength = static_cast<DWORD>(std::size(extra));
    if (!WinHttpCrackUrl(wideUrl.c_str(), 0, 0, &parts)) return result;

    HINTERNET session = WinHttpOpen(L"Noble-UPnP/1.0", WINHTTP_ACCESS_TYPE_NO_PROXY,
                                    WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
    if (!session) return result;
    WinHttpSetTimeouts(session, 1000, 1000, 1500, 1500);

    HINTERNET connection = WinHttpConnect(session, host, parts.nPort, 0);
    std::wstring requestPath(path);
    requestPath += extra;
    HINTERNET request = connection
        ? WinHttpOpenRequest(connection, method, requestPath.c_str(), nullptr, WINHTTP_NO_REFERER,
                             WINHTTP_DEFAULT_ACCEPT_TYPES,
                             parts.nScheme == INTERNET_SCHEME_HTTPS ? WINHTTP_FLAG_SECURE : 0)
        : nullptr;
    if (request) {
        if (!headers.empty()) WinHttpAddRequestHeaders(request, headers.c_str(), static_cast<DWORD>(-1), WINHTTP_ADDREQ_FLAG_ADD);
        const BOOL sent = WinHttpSendRequest(request, WINHTTP_NO_ADDITIONAL_HEADERS, 0,
                                             body.empty() ? WINHTTP_NO_REQUEST_DATA : const_cast<char*>(body.data()),
                                             static_cast<DWORD>(body.size()), static_cast<DWORD>(body.size()), 0);
        if (sent && WinHttpReceiveResponse(request, nullptr)) {
            DWORD status = 0;
            DWORD statusSize = sizeof(status);
            if (WinHttpQueryHeaders(request, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
                                    WINHTTP_HEADER_NAME_BY_INDEX, &status, &statusSize, WINHTTP_NO_HEADER_INDEX) &&
                status >= 200 && status < 300) {
                DWORD available = 0;
                while (WinHttpQueryDataAvailable(request, &available) && available > 0 && result.body.size() < 256 * 1024) {
                    std::string chunk(available, '\0');
                    DWORD read = 0;
                    if (!WinHttpReadData(request, chunk.data(), available, &read) || read == 0) break;
                    result.body.append(chunk.data(), read);
                }
                result.ok = true;
            }
        }
    }

    if (request) WinHttpCloseHandle(request);
    if (connection) WinHttpCloseHandle(connection);
    WinHttpCloseHandle(session);
    return result;
}

} // namespace

UPnP::UPnP() = default;

UPnP::~UPnP() {
    Shutdown();
}

std::string UPnP::GetLocalAdapterIP() {
    ULONG bufferSize = 0;
    GetAdaptersAddresses(AF_INET, GAA_FLAG_INCLUDE_GATEWAYS, nullptr, nullptr, &bufferSize);
    if (bufferSize == 0) return "127.0.0.1";

    std::vector<unsigned char> buffer(bufferSize);
    auto* adapters = reinterpret_cast<PIP_ADAPTER_ADDRESSES>(buffer.data());
    if (GetAdaptersAddresses(AF_INET, GAA_FLAG_INCLUDE_GATEWAYS, nullptr, adapters, &bufferSize) != NO_ERROR) {
        return "127.0.0.1";
    }

    std::string fallback;
    for (PIP_ADAPTER_ADDRESSES adapter = adapters; adapter; adapter = adapter->Next) {
        if (adapter->OperStatus != IfOperStatusUp || adapter->IfType == IF_TYPE_SOFTWARE_LOOPBACK) continue;
        for (PIP_ADAPTER_UNICAST_ADDRESS unicast = adapter->FirstUnicastAddress; unicast; unicast = unicast->Next) {
            if (!unicast->Address.lpSockaddr || unicast->Address.lpSockaddr->sa_family != AF_INET) continue;
            const auto* address = reinterpret_cast<const sockaddr_in*>(unicast->Address.lpSockaddr);
            const uint32_t hostOrder = ntohl(address->sin_addr.s_addr);
            if (hostOrder == INADDR_ANY || (hostOrder >> 24) == 127 || (hostOrder >> 16) == 0xA9FE) continue;

            char ip[INET_ADDRSTRLEN] = {};
            if (!inet_ntop(AF_INET, &address->sin_addr, ip, sizeof(ip))) continue;
            if (adapter->FirstGatewayAddress) return ip;
            if (fallback.empty()) fallback = ip;
        }
    }
    return fallback.empty() ? "127.0.0.1" : fallback;
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
    if (m_isInitialized.exchange(false)) WSACleanup();
}

bool UPnP::SendSSDPDiscovery(std::string& outLocation) {
    SOCKET sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock == INVALID_SOCKET) return false;

    // Fast timeout so UI never hitches if router lacks UPnP
    DWORD timeoutMs = 1500;
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
        "MX: 1\r\n\r\n";

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

    std::string lower = response;
    std::transform(lower.begin(), lower.end(), lower.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    const size_t location = lower.find("location:");
    if (location == std::string::npos) return false;
    size_t valueStart = location + std::strlen("location:");
    while (valueStart < response.size() && std::isspace(static_cast<unsigned char>(response[valueStart]))) ++valueStart;
    const size_t valueEnd = response.find("\r\n", valueStart);
    if (valueEnd == std::string::npos) return false;
    outLocation = response.substr(valueStart, valueEnd - valueStart);
    return !outLocation.empty();
}

bool UPnP::ParseDeviceDescription(const std::string& locationUrl) {
    const HttpResponse response = HttpRequest(locationUrl, L"GET", {}, {});
    if (!response.ok) return false;

    size_t position = 0;
    while ((position = response.body.find("<service", position)) != std::string::npos) {
        const size_t openBoundary = position + std::strlen("<service");
        if (openBoundary < response.body.size() && response.body[openBoundary] != '>' &&
            !std::isspace(static_cast<unsigned char>(response.body[openBoundary]))) {
            position = openBoundary;
            continue;
        }
        const size_t end = response.body.find("</service>", position);
        if (end == std::string::npos) break;
        const std::string service = response.body.substr(position, end - position);
        const std::string type = ElementText(service, "serviceType");
        if (type.find("WANIPConnection") != std::string::npos || type.find("WANPPPConnection") != std::string::npos) {
            const std::string control = ElementText(service, "controlURL");
            const size_t scheme = locationUrl.find("://");
            if (control.empty() || scheme == std::string::npos) break;

            if (control.find("://") != std::string::npos) {
                m_controlUrl = control;
            } else {
                const size_t pathStart = locationUrl.find('/', scheme + 3);
                const std::string origin = pathStart == std::string::npos ? locationUrl : locationUrl.substr(0, pathStart);
                if (control.front() == '/') {
                    m_controlUrl = origin + control;
                } else {
                    const size_t directoryEnd = locationUrl.rfind('/');
                    const std::string directory = pathStart == std::string::npos || directoryEnd < pathStart
                        ? "/"
                        : locationUrl.substr(pathStart, directoryEnd - pathStart + 1);
                    m_controlUrl = origin + directory + control;
                }
            }
            m_serviceType = type;
            return true;
        }
        position = end + std::strlen("</service>");
    }
    return false;
}

bool UPnP::SendSOAPAction(const std::string& action, const std::string& body, std::string& outResponse) {
    if (m_controlUrl.empty() || m_serviceType.empty()) return false;
    const std::string envelope =
        "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
        "<s:Envelope xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\" "
        "s:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\"><s:Body><u:" + action +
        " xmlns:u=\"" + m_serviceType + "\">" + body + "</u:" + action + "></s:Body></s:Envelope>";
    const std::wstring headers = L"Content-Type: text/xml; charset=\"utf-8\"\r\nSOAPACTION: \"" +
                                 Widen(m_serviceType + "#" + action) + L"\"\r\n";
    const HttpResponse response = HttpRequest(m_controlUrl, L"POST", headers, envelope);
    outResponse = response.body;
    return response.ok;
}

bool UPnP::ForwardPort(uint16_t port, const std::string& protocol, const std::string& description) {
    if (!m_isInitialized) {
        if (!Initialize()) return false;
    }

    std::string normalizedProtocol = protocol;
    std::transform(normalizedProtocol.begin(), normalizedProtocol.end(), normalizedProtocol.begin(),
                   [](unsigned char c) { return static_cast<char>(std::toupper(c)); });
    if (normalizedProtocol != "UDP" && normalizedProtocol != "TCP") {
        m_statusMessage = "UPnP supports only TCP or UDP port mappings";
        return false;
    }
    if (m_isActive && m_forwardedPort == port && m_protocol == normalizedProtocol) return true;
    if (m_isActive) ReleasePort(m_forwardedPort, m_protocol);

    m_statusMessage = "Discovering router via UPnP SSDP...";
    NOBLE_INFO("UPnP: Broadcasting SSDP M-SEARCH for gateway router...");

    std::string location;
    if (!SendSSDPDiscovery(location)) {
        m_statusMessage = "UPnP Gateway not found (Router UPnP disabled or LAN mode)";
        NOBLE_INFO("UPnP: Gateway not detected. Operating in Local LAN / Direct IP mode.");
        m_isActive = false;
        return false;
    }

    if (!ParseDeviceDescription(location)) {
        m_statusMessage = "UPnP gateway found, but its WAN service could not be read";
        NOBLE_WARN("UPnP: Could not parse gateway device description.");
        return false;
    }

    const std::string mapping =
        "<NewRemoteHost></NewRemoteHost><NewExternalPort>" + std::to_string(port) +
        "</NewExternalPort><NewProtocol>" + normalizedProtocol + "</NewProtocol><NewInternalPort>" +
        std::to_string(port) + "</NewInternalPort><NewInternalClient>" + m_internalIp +
        "</NewInternalClient><NewEnabled>1</NewEnabled><NewPortMappingDescription>" + EscapeXml(description) +
        "</NewPortMappingDescription><NewLeaseDuration>0</NewLeaseDuration>";
    std::string response;
    if (!SendSOAPAction("AddPortMapping", mapping, response)) {
        m_statusMessage = "UPnP gateway rejected " + normalizedProtocol + " " + std::to_string(port) + "; check router settings";
        NOBLE_WARN("UPnP: AddPortMapping failed for port {} ({})", port, normalizedProtocol);
        return false;
    }

    m_forwardedPort = port;
    m_protocol = normalizedProtocol;
    m_isActive = true;
    m_externalIp = "Unknown";
    if (SendSOAPAction("GetExternalIPAddress", {}, response)) {
        const std::string externalIp = ElementText(response, "NewExternalIPAddress");
        if (!externalIp.empty()) m_externalIp = externalIp;
    }
    m_statusMessage = "UPnP active: " + m_externalIp + ":" + std::to_string(port) + " forwarded to " + m_internalIp;
    NOBLE_INFO("UPnP: Port {} ({}) mapped to {}. External IP: {}", port, normalizedProtocol, m_internalIp, m_externalIp);

    return true;
}

bool UPnP::ReleasePort(uint16_t port, const std::string& protocol) {
    if (!m_isActive) return true;
    std::string response;
    const std::string body = "<NewRemoteHost></NewRemoteHost><NewExternalPort>" + std::to_string(m_forwardedPort) +
                             "</NewExternalPort><NewProtocol>" + m_protocol + "</NewProtocol>";
    const bool removed = SendSOAPAction("DeletePortMapping", body, response);
    m_isActive = false;
    m_forwardedPort = 0;
    m_statusMessage = removed ? "Port mapping released. Operating in Local/Offline mode." : "Could not remove the UPnP port mapping";
    NOBLE_INFO("UPnP: Port {} ({}) release {}.", port, protocol, removed ? "succeeded" : "failed");
    return removed;
}

} // namespace noble
