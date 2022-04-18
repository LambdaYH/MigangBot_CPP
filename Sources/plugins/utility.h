#ifndef MIGANGBOTCPP_PLUGINS_UTILITY_H_
#define MIGANGBOTCPP_PLUGINS_UTILITY_H_

#include <string_view>
#include <algorithm>

namespace white
{

inline std::string ExtraPlainText(const std::string &str)
{
    std::string_view view{str};
    view = view.substr(std::min(view.find_first_of(' ') + 1, str.size()));
    return std::string(view);
}

} //namespace white

#endif