#ifndef MIGANGBOTCPP_PLUGINS_UTILITY_H_
#define MIGANGBOTCPP_PLUGINS_UTILITY_H_

#include <string_view>
#include <algorithm>
#include <nlohmann/json.hpp>
#include <yaml-cpp/yaml.h>
#include <filesystem>

#include "global_config.h"

namespace white
{

inline std::string_view ExtraPlainText(const std::string &str)
{
    std::string_view view{str};
    view = view.substr(std::min(view.find_first_of(' ') + 1, view.size()));
    return view;
}

inline void ExtraPlainText(std::string_view &str)
{
    str = str.substr(std::min(str.find_first_of(' ') + 1, str.size()));
}

inline void LStrip(std::string_view &str, char ch)
{
    str.remove_prefix(std::min(str.find_first_not_of(ch), str.size()));
}

inline void RStrip(std::string_view &str, char ch)
{
    str.remove_suffix(std::max(str.find_last_not_of(ch), std::size_t(0)));
}

inline void Strip(std::string_view &str, char ch)
{
    str.remove_prefix(std::min(str.find_first_not_of(ch), str.size()));
    str.remove_suffix(std::max(str.find_last_not_of(ch), std::size_t(0)));
}

} //namespace white

#endif