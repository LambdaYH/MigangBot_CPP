#ifndef MIGANGBOTCPP_EVENT_EVENT_H_
#define MIGANGBOTCPP_EVENT_EVENT_H_

#include <string>
#include "type.h"
#include <nlohmann/json.hpp>

namespace white
{

using plugin_func = std::function<void(const Event &, std::function<void(const std::string &)> &)>;
using Event = nlohmann::json;

} // namespace white

#endif