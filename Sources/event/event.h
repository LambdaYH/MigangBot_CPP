#ifndef MIGANGBOTCPP_EVENT_EVENT_H_
#define MIGANGBOTCPP_EVENT_EVENT_H_

#include <string>
#include <nlohmann/json.hpp>

namespace white
{

using Event = nlohmann::json;
using plugin_func = std::function<void(const Event &, std::function<void(const std::string &)> &)>;

} // namespace white

#endif