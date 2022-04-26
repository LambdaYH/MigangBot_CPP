#ifndef MIGANGBOTCPP_MODULES_UTILITY_H_
#define MIGANGBOTCPP_MODULES_UTILITY_H_

#include <string_view>
#include <algorithm>
#include <nlohmann/json.hpp>
#include <yaml-cpp/yaml.h>
#include <filesystem>

#include "global_config.h"

namespace white
{

inline std::time_t GetTimeStampMs()
{
    auto tp = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now());
    return tp.time_since_epoch().count();
}

} //namespace white

#endif