#pragma once

#include <memory>
#include <string>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>

namespace noble {

class Logger {
public:
    static void Init(const std::string& logFileName = "noble-client.log") {
        try {
            auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
            console_sink->set_level(spdlog::level::trace);
            console_sink->set_pattern("[%H:%M:%S.%e] [%^%l%$] %v");

            auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(logFileName, true);
            file_sink->set_level(spdlog::level::trace);
            file_sink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] %v");

            std::vector<spdlog::sink_ptr> sinks { console_sink, file_sink };
            auto logger = std::make_shared<spdlog::logger>("noble", sinks.begin(), sinks.end());
            logger->set_level(spdlog::level::trace);
            logger->flush_on(spdlog::level::info);

            spdlog::set_default_logger(logger);
            spdlog::info("=================================================");
            spdlog::info(" Halo: Reach Community Edition (Project Noble)   ");
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

#define NOBLE_INFO(...)  spdlog::info(__VA_ARGS__)
#define NOBLE_WARN(...)  spdlog::warn(__VA_ARGS__)
#define NOBLE_ERROR(...) spdlog::error(__VA_ARGS__)
#define NOBLE_DEBUG(...) spdlog::debug(__VA_ARGS__)
