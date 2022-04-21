#ifndef MIGANGBOTCPP_MODULES_PLUGIN_INTERFACE_H_
#define MIGANGBOTCPP_MODULES_PLUGIN_INTERFACE_H_

#include <nlohmann/json.hpp>
#include <string>
#include <vector>
#include <functional>
#include <initializer_list>
#include <filesystem>
#include <fstream>

#include "modules/utility.h"
#include "bot/api_bot.h"
#include "event/event_handler.h"
#include "event/event.h"
#include "logger/logger.h"
#include "message/utility.h"
#include "permission/permission.h"

namespace white
{
namespace module
{

using Config = YAML::Node;

inline void RegisterAllMessage(const plugin_func &&func)
{
    EventHandler::GetInstance().RegisterCommand(ALLMSG, "", std::move(func));
}

inline void RegisterCommand(const int type, const std::initializer_list<std::string> &commands, const plugin_func &func, int permission = permission::NORMAL, bool only_to_me = false)
{
    for(auto &command : commands)
    {
        auto func_cp = func;
        EventHandler::GetInstance().RegisterCommand(type, command, std::move(func_cp), permission, only_to_me);
    }
}

inline void RegisterNotice(const std::string_view &notice_type, const std::string_view &sub_type, const plugin_func &func)
{
    EventHandler::GetInstance().RegisterNotice(notice_type, sub_type, std::move(func));
}

inline void RegisterRequest(const std::string_view &request_type, const std::string_view &sub_type, const plugin_func &func)
{
    EventHandler::GetInstance().RegisterRequest(request_type, sub_type, std::move(func));
}

class Module
{
public:
    // 如果没有配置文件，就留空
    Module(const std::string_view &config_file, const std::string_view &config_example = "")
        : config_file_(config_file),
        config_example_(config_example)
    {}

    virtual void Register() = 0;

protected:
    YAML::Node LoadConfig()
    {
        YAML::Node ret;
        if(config_file_.empty())
        {
            LOG_WARN("未配置配置文件路径", config_file_);
            return ret;
        }
        if(!std::string_view(config_file_).ends_with("yml"))
        {
            LOG_WARN("配置文件格式错误，应当为.yml");
            return ret;
        }
        std::filesystem::path path;
        if(config_file_[0] != '/')
            path = config::kConfigDir / config_file_;
        if(!std::filesystem::exists(path))
        {
            LOG_WARN("无法加载配置文件[{}]", config_file_);
            LOG_INFO("尝试生成配置文件");
            auto parent_path = path.parent_path();
            if(!std::filesystem::create_directories(parent_path))
            {
                LOG_ERROR("创建配置文件[{}]失败", config_file_);
                return ret;
            }
            std::fstream f(path, std::ios_base::out);
            f << config_example_;
            f.close();
            LOG_INFO("已成功创建[{}]", config_file_);
        }
        ret = YAML::LoadFile(path);
        return ret;
    }

private:
    const std::string config_file_;
    const std::string config_example_;

};

}
} // namespace white
#endif