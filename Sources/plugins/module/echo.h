#ifndef PLUGINS_MODULE_ECHO_H_
#define PLUGINS_MODULE_ECHO_H_

#include "plugins/plugin_interface.h"
#include "api/onebot_11/api_impl.h"
#include "event/event.h"
#include <functional>
#include <queue>
#include <condition_variable>
#include <mutex>

namespace white
{

class Echo : public PluginInterface
{
public:
    virtual void Register(EventHandler &event_handler);
    void DoEcho(const Event &event, std::function<void(const std::string &)> &notify);
private:
    onebot11::ApiImpl api_impl;
};

inline void Echo::Register(EventHandler &event_handler)
{
    event_handler.RegisterCommand(PREFIX, "/echo", std::bind(&Echo::DoEcho, this, std::placeholders::_1, std::placeholders::_2));
}

inline void Echo::DoEcho( const Event &event, std::function<void(const std::string &)> &notify)
{
    std::string msg = event["message"].asString().substr(6);
    notify(api_impl.send_msg(event, msg));
}

} // namespace white

#endif