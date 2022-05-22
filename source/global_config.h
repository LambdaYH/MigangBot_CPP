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

extern std::filesystem::path kServiceDir;

extern std::string BOT_NAME;

extern std::unordered_set<QId> SUPERUSERS;

extern std::unordered_set<QId> WHITE_LIST;

extern std::filesystem::path kAssetsDir;

inline std::filesystem::path AssetsPath(const std::string &path) {
  auto r_path = kAssetsDir / path;
  if(!std::filesystem::exists(r_path))
    std::filesystem::create_directory(r_path);
  return r_path;
}

}  // namespace config
}  // namespace white

#endif