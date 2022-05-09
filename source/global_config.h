#ifndef MIGANGBOT_GLOBAL_CONFIG_H_
#define MIGANGBOT_GLOBAL_CONFIG_H_

#include <filesystem>
#include <unordered_set>

#include <yaml-cpp/yaml.h>

#include "type.h"

namespace white {

extern YAML::Node global_config;

namespace config {

extern std::filesystem::path kConfigDir;

extern std::string BOT_NAME;

extern std::unordered_set<QId> SUPERUSERS;

extern std::unordered_set<QId> WHITE_LIST;

extern std::filesystem::path kAssetsDir;

}  // namespace config
}  // namespace white

#endif