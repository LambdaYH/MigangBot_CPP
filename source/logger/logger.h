#ifndef MIGANGBOT_LOGGER_LOGGER_H_
#define MIGANGBOT_LOGGER_LOGGER_H_

#include <memory>
#include <unordered_map>

#include "spdlog/spdlog.h"
#include "spdlog/sinks/daily_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"

namespace white
{

class Logger
{

public:
    static Logger &GetInstance()
    {
        static Logger logger;
        return logger;
    }
    spdlog::logger &GetLogger()
    {
        return *logger_;
    }
    void Init(const std::string &log_file, const std::string &level = "INFO");

public:
    Logger(const Logger &) = delete;
    Logger &operator=(const Logger &) = delete;
    Logger(const Logger &&) = delete;
    Logger &operator=(const Logger &&) = delete;

private:
    Logger() {}
    ~Logger() {}

private:
    std::shared_ptr<spdlog::logger> logger_;
};

inline void Logger::Init(const std::string &log_file, const std::string &level)
{
    std::vector<spdlog::sink_ptr> sinks;
    sinks.push_back(std::make_shared<spdlog::sinks::stdout_color_sink_st>());
    sinks.push_back(std::make_shared<spdlog::sinks::daily_file_sink_st>(log_file, 0, 0));
    logger_ = std::make_shared<spdlog::logger>("global_log", begin(sinks), end(sinks));
    std::unordered_map<std::string, spdlog::level::level_enum> string_to_level{
        {"DEBUG", spdlog::level::debug}, 
        {"INFO", spdlog::level::info}, 
        {"WARN", spdlog::level::warn}, 
        {"ERROR", spdlog::level::err},
        {"CRITICAL", spdlog::level::critical}};
    if(string_to_level.count(level))
        logger_->set_level(string_to_level[level]);
    else
        logger_->set_level(spdlog::level::info);
    logger_->set_pattern("[%Y-%m-%d %H:%M:%S][%l]: %v");
}

inline void LOG_INIT(const std::string &log_file, const std::string &level) noexcept
{
    Logger::GetInstance().Init(log_file, level);
}

template<typename... Args>
inline void LOG_DEBUG(spdlog::format_string_t<Args...> fmt, Args &&... args) noexcept
{
    Logger::GetInstance().GetLogger().debug(std::move(fmt), std::forward<Args>(args)...);
}

template<typename... Args>
inline void LOG_INFO(spdlog::format_string_t<Args...> fmt, Args &&... args) noexcept
{
    Logger::GetInstance().GetLogger().info(std::move(fmt), std::forward<Args>(args)...);
}

template<typename... Args>
inline void LOG_WARN(spdlog::format_string_t<Args...> fmt, Args &&... args) noexcept
{
    Logger::GetInstance().GetLogger().warn(std::move(fmt), std::forward<Args>(args)...);
}

template<typename... Args>
inline void LOG_ERROR(spdlog::format_string_t<Args...> fmt, Args &&... args) noexcept
{
    Logger::GetInstance().GetLogger().error(std::move(fmt), std::forward<Args>(args)...);
}

template<typename... Args>
inline void LOG_CRITICAL(spdlog::format_string_t<Args...> fmt, Args &&... args)
{
    Logger::GetInstance().GetLogger().critical(std::move(fmt), std::forward<Args>(args)...);
}

} // namespace white

#endif