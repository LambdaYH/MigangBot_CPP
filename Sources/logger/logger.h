#ifndef MIGANGBOTCPP_LOGGER_LOGGER_H_
#define MIGANGBOTCPP_LOGGER_LOGGER_H_

#include "spdlog/spdlog.h"
#include "spdlog/sinks/daily_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include <memory>
#include <unordered_map>

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
private:
    Logger() {}
    ~Logger() {}

    Logger(const Logger &) = delete;
    Logger &operator=(const Logger &) = delete;
    Logger(const Logger &&) = delete;
    Logger &operator=(const Logger &&) = delete;

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

#define LOG_INIT(log_file, level) Logger::GetInstance().Init(log_file, level)

#define LOG_DEBUG(...) Logger::GetInstance().GetLogger().debug(__VA_ARGS__)

#define LOG_INFO(...) Logger::GetInstance().GetLogger().info(__VA_ARGS__)

#define LOG_WARN(...) Logger::GetInstance().GetLogger().warn(__VA_ARGS__)

#define LOG_ERROR(...) Logger::GetInstance().GetLogger().err(__VA_ARGS__)

#define LOG_CRITICAL(...) Logger::GetInstance().GetLogger().critical(__VA_ARGS__)

} // namespace white

#endif