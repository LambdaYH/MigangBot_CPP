#ifndef MIGANGBOTCPP_GLOBAL_CONFIG_H_
#define MIGANGBOTCPP_GLOBAL_CONFIG_H_

#include <yaml-cpp/yaml.h>
#include <filesystem>
#include <unordered_set>

#include "type.h"

namespace white
{

extern YAML::Node global_config;

namespace config
{

extern std::filesystem::path kConfigDir;

extern std::string BOT_NAME;

extern std::unordered_set<QId> SUPERUSERS;

extern std::unordered_set<QId> WHITE_LIST;

} // namespace global_config
} // namespace white

#endif