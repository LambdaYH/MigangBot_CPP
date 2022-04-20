#ifndef MIGANGBOTCPP_GLOBAL_CONFIG_H_
#define MIGANGBOTCPP_GLOBAL_CONFIG_H_

#include <yaml-cpp/yaml.h>
#include <filesystem>

namespace white
{

extern YAML::Node global_config;

extern std::filesystem::path kConfigDir;

} // namespace white

#endif