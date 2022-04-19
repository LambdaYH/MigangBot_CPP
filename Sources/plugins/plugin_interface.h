#ifndef PLUGINS_PLUGIN_INTERFACE_H_
#define PLUGINS_PLUGIN_INTERFACE_H_

#include <nlohmann/json.hpp>
#include <string>
#include <vector>
#include <functional>
#include <initializer_list>

#include "plugins/utility.h"
#include "bot/api_bot.h"
#include "utility"
#include "event/event_handler.h"
#include "event/event.h"

namespace white
{

inline void RegisterCommand(const int type, const std::initializer_list<std::string> &commands, const plugin_func &func)
{
    for(auto &command : commands)
    {
        auto func_cp = func;
        EventHandler::GetInstance().RegisterCommand(type, command, std::move(func_cp));
    }
}

inline void RegisterNotice(const std::string &notice_type, const std::string &sub_type, const plugin_func &func)
{
    EventHandler::GetInstance().RegisterNotice(notice_type, sub_type, std::move(func));
}

inline void RegisterRequest(const std::string &request_type, const std::string &sub_type, const plugin_func &func)
{
    EventHandler::GetInstance().RegisterRequest(request_type, sub_type, std::move(func));
}

class PluginInterface
{
public:
    virtual void Register() = 0;
    
};

}
#endif