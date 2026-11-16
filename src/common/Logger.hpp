#pragma once

#include <cctype>
#include <cstring>
#include <memory>
#include <string>
#include <utility>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>

// Logging policy:
//   * The development build is the only client build that writes a log file. It keeps the full trace
//     (module RVAs, hook sites, map paths) because that is what makes a bug report actionable.
//   * The shipped client writes NO log at all. Its only diagnostic artefact is the crash minidump the
//     exception handler drops next to the executable. Compiling the log macros out also keeps every
//     diagnostic format string out of the release binary, so a decompiler cannot read our internals back
//     out of .rdata.
//   * The dedicated server is a server binary and keeps its log in release too.
#if defined(NOBLE_DEV_BUILD) || defined(NOBLE_DEDICATED)
#define NOBLE_LOG_ENABLED 1
#else
#define NOBLE_LOG_ENABLED 0
#endif

namespace noble {

#if NOBLE_LOG_ENABLED

// Release logs can be shared in bug reports. Absolute 64-bit addresses are redacted; module-relative
// RVAs (<= 32 bits) are kept, because they are the same in every copy of the game and are what makes a
// reported failure diagnosable without a development build.
inline std::string SanitizeLog(std::string text) {
    std::string out;
    out.reserve(text.size());
    for (size_t i = 0; i < text.size();) {
        // 0x-prefixed values: module RVAs, absolute addresses, string ids, ARGB colors.
        if (i + 2 < text.size() && text[i] == '0' && (text[i + 1] == 'x' || text[i + 1] == 'X') &&
            std::isxdigit(static_cast<unsigned char>(text[i + 2]))) {
            const size_t hexStart = i + 2;
            size_t hexEnd = hexStart;
            while (hexEnd < text.size() && std::isxdigit(static_cast<unsigned char>(text[hexEnd]))) ++hexEnd;
            const size_t digits = hexEnd - hexStart;
            if (digits <= 8) {
                // Module RVA or a small value: safe and useful to keep.
                out.append(text, i, hexEnd - i);
            } else {
                out += "0x<redacted>";
            }
            i = hexEnd;
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

#endif // NOBLE_LOG_ENABLED

class Logger {
public:
    static void Init(const std::string& logFileName = "") {
#if NOBLE_LOG_ENABLED
        try {
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
#else
        // Shipped client: no log file, no console sink, no diagnostic text. The crash dump is the only
        // artefact (see the exception handler in main.cpp).
        (void)logFileName;
        spdlog::set_level(spdlog::level::off);
#endif
    }

    static void Shutdown() {
#if NOBLE_LOG_ENABLED
        spdlog::shutdown();
#endif
    }
};

} // namespace noble

#if NOBLE_LOG_ENABLED
#define NOBLE_INFO(...)  ::noble::LogRedacted(spdlog::level::info, __VA_ARGS__)
#define NOBLE_WARN(...)  ::noble::LogRedacted(spdlog::level::warn, __VA_ARGS__)
#define NOBLE_ERROR(...) ::noble::LogRedacted(spdlog::level::err, __VA_ARGS__)
#define NOBLE_DEBUG(...) ::noble::LogRedacted(spdlog::level::debug, __VA_ARGS__)
#else
// Shipped client: the diagnostic surface compiles away completely, so neither the messages nor their
// format strings exist in the executable.
#define NOBLE_INFO(...)  ((void)0)
#define NOBLE_WARN(...)  ((void)0)
#define NOBLE_ERROR(...) ((void)0)
#define NOBLE_DEBUG(...) ((void)0)
#endif
