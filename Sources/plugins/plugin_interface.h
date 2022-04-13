#ifndef PLUGINS_PLUGIN_INTERFACE_H_
#define PLUGINS_PLUGIN_INTERFACE_H_

#include "event/event_handler.h"
#include <json/json.h>
#include <string>
#include <functional>

namespace white
{

class PluginInterface
{
public:
    virtual void Register(EventHandler &event_handler) = 0;
};

}
#endif