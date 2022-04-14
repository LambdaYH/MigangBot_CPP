#ifndef PLUGINS_PLUGIN_INTERFACE_H_
#define PLUGINS_PLUGIN_INTERFACE_H_

#include "event/event_handler.h"
#include <nlohmann/json.hpp>
#include <string>
#include <vector>
#include <functional>
#include "plugins/utility.h"
#include "bot/api_bot.h"

namespace white
{

class PluginInterface
{
public:
    virtual void Register(EventHandler &event_handler) = 0;
};

}
#endif