#pragma once

#include <cctype>
#include <cstring>
#include <memory>
#include <string>
#include <utility>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>

namespace noble {

// Release logs can be shared in bug reports. Never print module RVAs / absolute addresses there;
// development builds keep them. Every 0x-prefixed hexadecimal token (hook RVAs, addresses, string
// ids, ARGB values) is replaced with 0x<redacted>.
inline std::string SanitizeLog(std::string text) {
    std::string out;
    out.reserve(text.size());
    for (size_t i = 0; i < text.size();) {
        // 0x-prefixed values: module RVAs, absolute addresses, string ids, ARGB colors.
        if (i + 2 < text.size() && text[i] == '0' && (text[i + 1] == 'x' || text[i + 1] == 'X') &&
            std::isxdigit(static_cast<unsigned char>(text[i + 2]))) {
            out += "0x<redacted>";
            i += 2;
            while (i < text.size() && std::isxdigit(static_cast<unsigned char>(text[i]))) ++i;
            continue;
        }
        // URLs: master/relay endpoints, download URLs.
        if (i + 7 < text.size() &&
            (_strnicmp(text.c_str() + i, "http://", 7) == 0 || _strnicmp(text.c_str() + i, "https://", 8) == 0)) {
            out += "<redacted-url>";
            while (i < text.size() && !std::isspace(static_cast<unsigned char>(text[i])) &&
                   text[i] != '"' && text[i] != '\'' && text[i] != ')' && text[i] != ']') {
                ++i;
            }
            continue;
        }
        // IPv4 addresses: server and peer endpoints.
        if (std::isdigit(static_cast<unsigned char>(text[i])) && (i == 0 || !std::isdigit(static_cast<unsigned char>(text[i - 1])))) {
            const size_t start = i;
            int octets[4] = {0, 0, 0, 0};
            size_t pos = i;
            bool ok = true;
            for (int oct = 0; oct < 4; ++oct) {
                size_t digits = 0;
                int value = 0;
                while (pos < text.size() && std::isdigit(static_cast<unsigned char>(text[pos])) && digits < 3) {
                    value = value * 10 + (text[pos] - '0');
                    ++pos;
                    ++digits;
                }
                if (digits == 0 || value > 255) { ok = false; break; }
                octets[oct] = value;
                if (oct < 3) {
                    if (pos >= text.size() || text[pos] != '.') { ok = false; break; }
                    ++pos;
                }
            }
            if (ok && octets[0] > 0 && (pos >= text.size() || (!std::isdigit(static_cast<unsigned char>(text[pos])) && text[pos] != '.'))) {
                out += "<redacted-ip>";
                i = pos;
                continue;
            }
            i = start;
        }
        out.push_back(text[i++]);
    }
    return out;
}

template <typename... Args>
inline void LogRedacted(spdlog::level::level_enum level, fmt::format_string<Args...> fmt, Args&&... args) {
    std::string message = fmt::format(fmt, std::forward<Args>(args)...);
#ifndef NOBLE_DEV_BUILD
    message = SanitizeLog(std::move(message));
#endif
    spdlog::log(level, "{}", message);
}

class Logger {
public:
    static void Init(const std::string& logFileName = "noble-client.log") {
        try {
            // Release builds only record errors (the ones a crash report needs), never the normal
            // startup/scan/hook chatter that contains RVAs and server details. Development builds
            // keep the full trace.
#ifdef NOBLE_DEV_BUILD
            constexpr spdlog::level::level_enum level = spdlog::level::trace;
#else
            constexpr spdlog::level::level_enum level = spdlog::level::info;
#endif
            auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
            console_sink->set_level(level);
            console_sink->set_pattern("[%H:%M:%S.%e] [%^%l%$] %v");

            auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(logFileName, false);
            file_sink->set_level(level);
            file_sink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] %v");

            std::vector<spdlog::sink_ptr> sinks { console_sink, file_sink };
            auto logger = std::make_shared<spdlog::logger>("noble", sinks.begin(), sinks.end());
            logger->set_level(level);
            logger->flush_on(spdlog::level::info);

            spdlog::set_default_logger(logger);
            spdlog::info("=================================================");
            spdlog::info(" Project Noble (Closed Source)   ");
            spdlog::info("=================================================");
        } catch (const spdlog::spdlog_ex&) {
            // Fallback if log file cannot be created
        }
    }

    static void Shutdown() {
        spdlog::shutdown();
    }
};

} // namespace noble

#define NOBLE_INFO(...)  ::noble::LogRedacted(spdlog::level::info, __VA_ARGS__)
#define NOBLE_WARN(...)  ::noble::LogRedacted(spdlog::level::warn, __VA_ARGS__)
#define NOBLE_ERROR(...) ::noble::LogRedacted(spdlog::level::err, __VA_ARGS__)
#define NOBLE_DEBUG(...) ::noble::LogRedacted(spdlog::level::debug, __VA_ARGS__)
